/* =========================================================================
 * This file is part of six-c++ 
 * =========================================================================
 * 
 * (C) Copyright 2004 - 2009, General Dynamics - Advanced Information Systems
 *
 * six-c++ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program; If not, 
 * see <http://www.gnu.org/licenses/>.
 *
 */
#include <import/six.h>
#include <import/six/sidd.h>
#include <import/six/sicd.h>
#include <import/xml/lite.h>
#include <import/io.h>
#include "SIXUtils.h"

/*
 *  Generate a KML file for a SICD XML to help with
 *  visualization
 *
 */

std::string generateKML(six::Data* data, const sys::Path& outputDir);

/*
 *  Open the round trip product in whatever you use to view XML files
 *  (e.g., Altova XMLSpy).  This only works currently on windows
 *
 */
#if defined(WIN32) && defined(PREVIEW)
#   include <shellapi.h>
void preview(std::string outputFile)
{
    // ShellExecute might get assigned to ShellExecuteW if we arent careful
    ShellExecuteA(NULL, "open", outputFile.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}
#   else
// TODO Could open this using EDITOR or html view
void preview(std::string outputFile)
{
    std::cerr << "Preview unavailable for: " << outputFile << std::endl;
}
#endif

/*
 * Dump all files out to the local directory
 *
 */
std::vector<std::string> extractXML(std::string inputFile,
                                    const sys::Path& outputDir)
{
    std::vector < std::string > allFiles;
    std::string prefix = sys::Path::basename(inputFile, true);

    nitf::Reader reader;
    nitf::IOHandle io(inputFile);
    nitf::Record record = reader.read(io);

    nitf::Uint32 numDES = record.getNumDataExtensions();
    for (nitf::Uint32 i = 0; i < numDES; ++i)
    {
        nitf::DESegment segment = record.getDataExtensions()[i];
        nitf::DESubheader subheader = segment.getSubheader();

        nitf::SegmentReader deReader = reader.newDEReader(i);
        nitf::Off size = deReader.getSize();

        std::string typeID = subheader.getTypeID().toString();
        str::trim(typeID);
        sys::Path fileName(outputDir, FmtX("%s-%s%d.xml", prefix.c_str(),
                                           typeID.c_str(), i));

        char* xml = new char[size];
        deReader.read(xml, size);

        io::ByteStream bs;
        bs.write(xml, size);

        xml::lite::MinidomParser parser;
        parser.parse(bs);

        xml::lite::Document* doc = parser.getDocument();

        io::FileOutputStream fos(fileName.getPath());
        doc->getRootElement()->prettyPrint(fos);
        fos.close();
        delete[] xml;
        allFiles.push_back(fileName.getPath());
    }
    io.close();
    return allFiles;
}

void run(std::string inputFile, six::XMLControlRegistry* registry)
{
    try
    {
        // Create an output directory if it doesnt already exist
        sys::Path outputDir(sys::Path::basename(inputFile, true));
        if (!outputDir.exists())
            outputDir.mkdir();

        std::string xmlFile = inputFile;

        // Check if the file is a NITF - if so, extract all the parts into our outputDir
        if (nitf::Reader::getNITFVersion(inputFile) != NITF_VER_UNKNOWN)
        {
            std::vector < std::string > allFiles = extractXML(inputFile,
                                                              outputDir);
            if (!allFiles.size())
                throw except::Exception(
                                        Ctxt(
                                             std::string("Invalid input NITF: ")
                                                     + inputFile));
            xmlFile = allFiles[0];
        }

        // Now show the file (only if PREVIEW is defined)
        preview(xmlFile);

        // Read in the XML file from the disk location into a SICD/SIDD container
        io::FileInputStream xmlFileStream(xmlFile);
        xml::lite::MinidomParser treeBuilder;
        treeBuilder.parse(xmlFileStream);
        xmlFileStream.close();

        xml::lite::Document *doc = treeBuilder.getDocument();
        std::string uri = doc->getRootElement()->getUri();

        if (!str::startsWith(uri, "urn:"))
            throw except::Exception(
                                    Ctxt(
                                         FmtX(
                                              "Unable to transform XML DES: Invalid XML namespace URI: %s",
                                              uri.c_str())));

        std::string typeId = uri.substr(4); // strip off the urn:

        six::XMLControl *control = registry->newXMLControl(typeId);
        if (!control)
            throw except::Exception(Ctxt("Unable to transform XML DES"));

        six::Data *data = control->fromXML(doc);

        // Dump some core info
        std::cout << "Data Class: " << six::toString(data->getDataType())
                << std::endl;
        std::cout << "Pixel Type: " << six::toString(data->getPixelType())
                << std::endl;
        std::cout << "Num Rows  : " << six::toString(data->getNumRows())
                << std::endl;
        std::cout << "Num Cols  : " << six::toString(data->getNumCols())
                << std::endl;

        // Generate a KML in this directory
        preview(generateKML(data, outputDir));

        // Generate a Round-Trip file
        sys::Path outputFile(outputDir, "round-trip.xml");
        xml::lite::Document* outDom = control->toXML(data);
        io::FileOutputStream outXML(outputFile.getPath());
        outDom->getRootElement()->prettyPrint(outXML);
        outXML.close();
        delete outDom;

        // Now show the output file if PREVIEW
        preview(outputFile.getPath());

        delete control;
        delete data;
    }
    catch (except::Exception& ex)
    {
        std::cout << "ERROR!: " << ex.getMessage() << std::endl;
    }
}

int main(int argc, char** argv)
{

    if (argc != 2)
    {
        die_printf("Usage: %s <nitf/xml-file/nitf-dir>\n", argv[0]);
    }

    // Do this prior to reading any XML
    six::XMLControlRegistry *xmlRegistry = newXMLControlRegistry();

    // The input file (an XML or a NITF file)
    sys::Path inputFile(argv[1]);
    std::vector < sys::Path > paths;

    if (inputFile.isDirectory())
    {
        std::vector < std::string > listing = inputFile.list();
        for (unsigned int i = 0; i < listing.size(); ++i)
        {
            if (str::endsWith(listing[i], "ntf") || str::endsWith(listing[i],
                                                                  "nitf")
                    || str::endsWith(listing[i], "xml"))
            {
                paths.push_back(sys::Path(inputFile, listing[i]));
            }
        }
    }
    else
    {
        paths.push_back(inputFile);
    }
    for (unsigned int i = 0; i < paths.size(); ++i)
    {
        std::cout << "Parsing file: " << paths[i].getPath() << std::endl;
        run(paths[i].getPath(), xmlRegistry);
    }

    // cleanup
    delete xmlRegistry;

    return 0;
}

const std::string KML_URI = "http://www.opengis.net/kml/2.2";
const std::string GX_URI = "http://www.google.com/kml/ext/2.2";
const std::string FOOTPRINT_COLOR = "ffff0000";

xml::lite::Element* createPath(std::vector<six::LatLon>& coords,
                               std::string name, std::string envelopeType =
                                       "LineString", std::string envelopeURI =
                                       KML_URI)
{
    std::ostringstream oss;
    if (coords.size() < 1)
        throw except::Exception(Ctxt("Unexpected zero-length coordinates"));
    unsigned int less1 = (unsigned int) (coords.size() - 1);

    for (unsigned int i = 0; i < less1; ++i)
    {
        oss << coords[i].getLon() << "," << coords[i].getLat();
        oss << " ";
    }
    oss << coords[less1].getLon() << "," << coords[less1].getLat();

    // Repeat the first element
    if (envelopeType == "LinearRing")
    {
        oss << " ";
        oss << coords[0].getLon() << "," << coords[0].getLat();
    }

    xml::lite::Element* coordsXML = new xml::lite::Element("coordinates",
                                                           KML_URI, oss.str());
    xml::lite::Element* envelopeXML = new xml::lite::Element(envelopeType,
                                                             KML_URI);
    envelopeXML->addChild(coordsXML);

    xml::lite::Element* placemarkXML = new xml::lite::Element("Placemark",
                                                              KML_URI);
    placemarkXML->addChild(new xml::lite::Element("name", KML_URI, name));
    placemarkXML->addChild(new xml::lite::Element("styleUrl", KML_URI,
                                                  std::string("#") + name));
    placemarkXML->addChild(envelopeXML);

    return placemarkXML;
}

xml::lite::Element* createPath(std::vector<six::LatLonAlt>& coords,
                               std::string name, std::string envelopeType =
                                       "LineString", std::string envelopeURI =
                                       KML_URI)
{
    std::ostringstream oss;
    if (coords.size() < 1)
        throw except::Exception(Ctxt("Unexpected zero-length coordinates"));
    unsigned int less1 = (unsigned int) (coords.size() - 1);

    for (unsigned int i = 0; i < less1; ++i)
    {
        oss << coords[i].getLon() << "," << coords[i].getLat() << ","
                << coords[i].getAlt();
        oss << " ";
    }
    oss << coords[less1].getLon() << "," << coords[less1].getLat() << ","
            << coords[less1].getAlt();

    // Repeat the first element
    if (envelopeType == "LinearRing")
    {
        oss << " ";
        oss << coords[0].getLon() << "," << coords[0].getLat() << ","
                << coords[0].getAlt();
    }

    xml::lite::Element* coordsXML = new xml::lite::Element("coordinates",
                                                           KML_URI, oss.str());
    xml::lite::Element* envelopeXML = new xml::lite::Element(envelopeType,
                                                             KML_URI);
    envelopeXML->addChild(coordsXML);

    xml::lite::Element* placemarkXML = new xml::lite::Element("Placemark",
                                                              KML_URI);
    placemarkXML->addChild(new xml::lite::Element("name", KML_URI, name));
    placemarkXML->addChild(new xml::lite::Element("styleUrl", KML_URI,
                                                  std::string("#") + name));
    placemarkXML->addChild(envelopeXML);

    return placemarkXML;
}

xml::lite::Element* createTimeSnapshot(const six::DateTime& dt)//, int seconds)
{
    xml::lite::Element* timeSnapshot = new xml::lite::Element("TimeStamp",
                                                              KML_URI);
    timeSnapshot->addChild(
                           new xml::lite::Element(
                                                  "when",
                                                  KML_URI,
                                                  six::toString<six::DateTime>(
                                                                               dt)));
    return timeSnapshot;
}

void generateKMLForSICD(xml::lite::Element* docXML,
                        six::sicd::ComplexData* data)
{

    std::vector < six::LatLonAlt > v;
    // Lets go ahead and try to add the ARP poly

    v.push_back(data->geoData->scp.llh);
    v.push_back(scene::Utilities::ecefToLatLon(data->scpcoa->arpPos));

    docXML->addChild(createPath(v, "arpToSceneCenter", "LineString"));

    // Time associated with start
    six::DateTime dateTime = data->timeline->collectStart;

    int durationInSeconds = (int) data->timeline->collectDuration;
    six::Vector3 atTimeX;
    for (unsigned int i = 0; i < durationInSeconds; ++i)
    {
        atTimeX = data->position->arpPoly(i);
        v[1] = scene::Utilities::ecefToLatLon(atTimeX);
        xml::lite::Element* arpPolyXML = createPath(v, "arpPoly", "LineString");
        six::DateTime dt(dateTime.getTimeInMillis() + i * 1000);
        arpPolyXML->addChild(createTimeSnapshot(dt));
        docXML->addChild(arpPolyXML);
    }

}

xml::lite::Element* createLineStyle(std::string styleID, std::string color,
                                    int width)
{
    xml::lite::Element* styleXML = new xml::lite::Element("Style", KML_URI);
    styleXML->attribute("id") = styleID;

    xml::lite::Element* lineStyleXML = new xml::lite::Element("LineStyle",
                                                              KML_URI);
    lineStyleXML->addChild(new xml::lite::Element("color", KML_URI, color));
    lineStyleXML->addChild(new xml::lite::Element("width", KML_URI,
                                                  str::toString<int>(width)));

    styleXML->addChild(lineStyleXML);
    return styleXML;
}

std::string generateKML(six::Data* data, const sys::Path& outputDir)
{
    std::string kmlFile = "visualization.kml";
    sys::Path kmlPath(outputDir, kmlFile);

    io::FileOutputStream fos(kmlPath.getPath());
    xml::lite::Element* root = new xml::lite::Element("kml", KML_URI);

    xml::lite::Element* docXML = new xml::lite::Element("Document", KML_URI);
    root->addChild(docXML);

    // Add name, and open the document
    docXML->addChild(new xml::lite::Element("name", KML_URI, data->getName()));
    docXML->addChild(new xml::lite::Element("open", KML_URI, "1"));

    // Now add some styles
    docXML->addChild(createLineStyle("footprint", FOOTPRINT_COLOR, 4));
    docXML->addChild(createLineStyle("arpToSceneCenter", "ff0000ff", 2));
    docXML->addChild(createLineStyle("arpPoly", "ff00007f", 2));

    // Create footprint
    std::vector < six::LatLon > corners = data->getImageCorners();
    docXML->addChild(createPath(corners, "footprint", "LinearRing"));

    // Specifics to SICD
    if (data->getDataType() == six::DataType::COMPLEX)
        generateKMLForSICD(docXML, (six::sicd::ComplexData*) data);

    root->prettyPrint(fos);
    delete root;
    fos.close();
    return kmlPath.getPath();
}
