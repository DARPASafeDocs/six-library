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
#include "six/Types.h"
#include "six/Utilities.h"
#include <import/str.h>

#include "six/sidd/DerivedXMLControl_0_5_0.h"
#include "six/sidd/DerivedData.h"
#include "six/sidd/DerivedDataBuilder.h"

using namespace six;
using namespace six::sidd;

typedef xml::lite::Element* XMLElem;

const char DerivedXMLControl_0_5_0::SI_COMMON_URI[] = "urn:SICommon:0.1";
const char DerivedXMLControl_0_5_0::SFA_URI[] = "urn:SFA:1.2.0";

std::string DerivedXMLControl_0_5_0::getDefaultURI() const
{
    return "urn:" + six::sidd::SIDD_ID_0_5_0.toString();
}

std::string DerivedXMLControl_0_5_0::getSICommonURI() const
{
    return SI_COMMON_URI;
}

std::string DerivedXMLControl_0_5_0::getSFAURI() const
{
    return SFA_URI;
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem productCreationXML,
        ProductCreation* productCreation)
{
    XMLElem informationXML = getFirstAndOnly(productCreationXML,
                                             "ProcessorInformation");
    productCreation->processorInformation->application
            = getFirstAndOnly(informationXML, "Application")->getCharacterData();

    parseDateTime(getFirstAndOnly(informationXML, "ProcessingDateTime"),
                  productCreation->processorInformation->processingDateTime);

    productCreation->processorInformation->site
            = getFirstAndOnly(informationXML, "Site")->getCharacterData();

    XMLElem profileXML = getOptional(informationXML, "Profile");
    if (profileXML)
    {
        productCreation->processorInformation->profile
                = profileXML->getCharacterData();
    }

    //Classification
    XMLElem classificationXML = getFirstAndOnly(productCreationXML,
                                                "Classification");
    productCreation->classification.level
            = getFirstAndOnly(classificationXML, "Level")->getCharacterData();

    std::vector<XMLElem> secMarkingsXML;
    classificationXML->getElementsByTagName("SecurityMarkings", secMarkingsXML);
    for (std::vector<XMLElem>::const_iterator it = secMarkingsXML.begin(); it
            != secMarkingsXML.end(); ++it)
    {
        productCreation->classification.securityMarkings. push_back(
                                                                    (*it)->getCharacterData());
    }

    XMLElem classGuidanceXML = getOptional(classificationXML, "Guidance");
    if (classGuidanceXML)
    {
        ClassificationGuidance* classGuidance = new ClassificationGuidance();
        productCreation->classification.guidance = classGuidance;

        parseDateTime(getFirstAndOnly(classGuidanceXML, "Date"),
                      productCreation->classification.guidance->date);
        productCreation->classification.guidance->authority
                = getFirstAndOnly(classGuidanceXML, "Authority")->getCharacterData();
    }

    productCreation->productName
            = getFirstAndOnly(productCreationXML, "ProductName")->getCharacterData();
    productCreation->productClass
            = getFirstAndOnly(productCreationXML, "ProductClass")->getCharacterData();

    XMLElem productTypeXML = getOptional(productCreationXML, "ProductType");
    if (productTypeXML)
    {
        productCreation->productType = productTypeXML->getCharacterData();
    }

    try
    {
        parseParameters(productCreationXML, "ProductCreationExtension",
                        productCreation->productCreationExtensions);
    }
    catch (except::Exception&)
    {
    }
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem displayXML, Display* display)
{
    display->pixelType
            = six::toType<PixelType>(
                                     getFirstAndOnly(displayXML, "PixelType")->getCharacterData());

    //RemapInformation
    XMLElem remapInformationXML = getOptional(displayXML, "RemapInformation");
    if (remapInformationXML)
    {
        if (display->remapInformation)
            delete display->remapInformation;

        XMLElem remapXML =
                getOptional(remapInformationXML, "ColorDisplayRemap");
        if (remapXML)
        {
            XMLElem remapLUTXML = getFirstAndOnly(remapXML, "RemapLUT");

            //get size attribute
            int
                    size =
                            str::toType<int>(
                                             remapLUTXML->getAttributes().getValue(
                                                                                   "size"));
            std::string lutStr = remapLUTXML->getCharacterData();
            std::vector<std::string> lutVals = str::split(lutStr, " ");
            LUT *remapLUT = new LUT(size, 3);

            int k = 0;
            for (unsigned int i = 0; i < lutVals.size(); i++)
            {
                std::vector<std::string> rgb = str::split(lutVals[i], ",");
                for (unsigned int j = 0; j < rgb.size(); j++)
                {
                    remapLUT->table[k++] = rgb[j].at(0);
                }
            }
            display->remapInformation = new ColorDisplayRemap(remapLUT);
        }
        else
        {
            remapXML = getOptional(remapInformationXML,
                                   "MonochromeDisplayRemap");
            if (remapXML)
            {
                std::string
                        remapType =
                                getFirstAndOnly(remapXML, "RemapType")->getCharacterData();
                XMLElem remapLUTXML = getOptional(remapXML, "RemapLUT");

                LUT *remapLUT = NULL;
                if (remapLUTXML)
                {
                    //get size attribute
                    int
                            size =
                                    str::toType<int>(
                                                     remapLUTXML->getAttributes().getValue(
                                                                                           "size"));
                    std::string lutStr = remapLUTXML->getCharacterData();
                    std::vector<std::string> lutVals = str::split(lutStr, " ");
                    remapLUT = new LUT(size, 1);
                    for (unsigned int i = 0; i < lutVals.size(); i++)
                    {
                        remapLUT->table[i] = lutVals[i].at(0);
                    }
                }

                MonochromeDisplayRemap *monoRemap =
                        new MonochromeDisplayRemap(remapType, remapLUT);
                display->remapInformation = monoRemap;

                //RemapParameters
                try
                {
                    parseParameters(remapXML, "RemapParameter",
                                    monoRemap->remapParameters);
                }
                catch (except::Exception&)
                {
                }
            }
            else
            {
                throw except::Exception(Ctxt("Invalid or missing Remap choice"));
            }
        }

    }

    XMLElem magXML = getOptional(displayXML, "MagnificationMethod");
    XMLElem decXML = getOptional(displayXML, "DecimationMethod");

    //optional
    display->magnificationMethod
            = magXML ? six::toType<MagnificationMethod>(
                                                        magXML->getCharacterData())
                     : MagnificationMethod::NOT_SET;

    //optional
    display->decimationMethod
            = decXML ? six::toType<DecimationMethod>(decXML->getCharacterData())
                     : DecimationMethod::NOT_SET;

    XMLElem histogramOverridesXML = getOptional(displayXML,
                                                "DRAHistogramOverrides");
    if (histogramOverridesXML)
    {
        display->histogramOverrides = new DRAHistogramOverrides();
        parseInt(getFirstAndOnly(histogramOverridesXML, "ClipMin"),
                 display->histogramOverrides->clipMin);
        parseInt(getFirstAndOnly(histogramOverridesXML, "ClipMax"),
                 display->histogramOverrides->clipMax);
    }

    //MonitorCompensationApplied
    XMLElem monitorCompensationXML = getOptional(displayXML,
                                                 "MonitorCompensationApplied");
    if (monitorCompensationXML)
    {
        display->monitorCompensationApplied = new MonitorCompensationApplied();
        parseDouble(getFirstAndOnly(monitorCompensationXML, "Gamma"),
                    display->monitorCompensationApplied->gamma);
        parseDouble(getFirstAndOnly(monitorCompensationXML, "XMin"),
                    display->monitorCompensationApplied->xMin);
    }

    //DisplayExtension
    try
    {
        parseParameters(displayXML, "DisplayExtension",
                        display->displayExtensions);
    }
    catch (except::Exception&)
    {
    }

}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem geographicAndTargetXML,
        GeographicAndTarget* geographicAndTarget)
{
    XMLElem geographicCoverageXML = getFirstAndOnly(geographicAndTargetXML,
                                                    "GeographicCoverage");

    fromXML(geographicCoverageXML, geographicAndTarget->geographicCoverage);

    //TargetInformation
    std::vector<XMLElem> targetInfosXML;
    geographicAndTargetXML->getElementsByTagName("TargetInformation",
                                                 targetInfosXML);

    for (std::vector<XMLElem>::const_iterator it = targetInfosXML.begin(); it
            != targetInfosXML.end(); ++it)
    {
        TargetInformation* ti = new TargetInformation();
        geographicAndTarget->targetInformation.push_back(ti);

        parseParameters(*it, "Identifier", ti->identifiers);

        //Footprint
        std::vector<XMLElem> footprintsXML;
        (*it)->getElementsByTagName("Footprint", footprintsXML);
        for (std::vector<XMLElem>::const_iterator it2 = footprintsXML.begin(); it2
                != footprintsXML.end(); ++it2)
        {
            std::vector<LatLon> fp;
            parseFootprint(*it2, "Vertex", fp);
            ti->footprints.push_back(fp);
        }

        //TargetInformationExtension
        try
        {
            parseParameters(*it, "TargetInformationExtension",
                            ti->targetInformationExtensions);
        }
        catch (except::Exception&)
        {
        }
    }
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem geographicCoverageXML,
        GeographicCoverage* geographicCoverage)
{
    std::vector<XMLElem> georegionIdsXML;
    geographicCoverageXML->getElementsByTagName("GeoregionIdentifier",
                                                georegionIdsXML);
    for (std::vector<XMLElem>::const_iterator it = georegionIdsXML.begin(); it
            != georegionIdsXML.end(); ++it)
    {
        geographicCoverage->georegionIdentifiers.push_back(
                                                           (*it)->getCharacterData());
    }

    XMLElem footprintXML = getFirstAndOnly(geographicCoverageXML, "Footprint");

    //Footprint
    parseFootprint(footprintXML, "Vertex", geographicCoverage->footprint);

    //If there are subregions, recurse
    std::vector<XMLElem> subRegionsXML;
    geographicCoverageXML->getElementsByTagName("SubRegion", subRegionsXML);

    int i = 0;
    for (std::vector<XMLElem>::const_iterator it = subRegionsXML.begin(); it
            != subRegionsXML.end(); ++it)
    {
        geographicCoverage->subRegion.push_back(
                                                new GeographicCoverage(
                                                                       RegionType::SUB_REGION));
        fromXML(*it, geographicCoverage->subRegion[i++]);
    }

    //Otherwise read the GeographicInfo
    if (subRegionsXML.size() == 0)
    {
        XMLElem geographicInfoXML = getFirstAndOnly(geographicCoverageXML,
                                                    "GeographicInfo");

        geographicCoverage->geographicInformation = new GeographicInformation();

        std::vector<XMLElem> countryCodes;
        geographicInfoXML->getElementsByTagName("CountryCode", countryCodes);
        for (std::vector<XMLElem>::const_iterator it = countryCodes.begin(); it
                != countryCodes.end(); ++it)
        {
            XMLElem countryCode = *it;
            geographicCoverage->geographicInformation->countryCodes.push_back(
                                                                              countryCode->getCharacterData());
        }

        XMLElem securityInformationXML = getOptional(geographicInfoXML,
                                                     "SecurityInformation");
        if (securityInformationXML)
        {
            geographicCoverage->geographicInformation->securityInformation
                    = securityInformationXML->getCharacterData();
        }

        //GeographicInfoExtension
        try
        {
            parseParameters(
                            geographicInfoXML,
                            "GeographicInfoExtension",
                            geographicCoverage->geographicInformation-> geographicInformationExtensions);
        }
        catch (except::Exception&)
        {
        }
    }
}

// This function ASSUMES that the measurement projection has already been set!
void DerivedXMLControl_0_5_0::fromXML(const XMLElem measurementXML,
        Measurement* measurement)
{
    parseRowColInt(getFirstAndOnly(measurementXML, "PixelFootprint"),
                   measurement->pixelFootprint);

    XMLElem projXML;
    if (measurement->projection->projectionType == ProjectionType::PLANE)
    {
        projXML = getFirstAndOnly(measurementXML, "PlaneProjection");
    }
    else if (measurement->projection->projectionType
            == ProjectionType::CYLINDRICAL)
    {
        projXML = getFirstAndOnly(measurementXML, "CylindricalProjection");
    }
    else if (measurement->projection->projectionType
            == ProjectionType::GEOGRAPHIC)
    {
        projXML = getFirstAndOnly(measurementXML, "GeographicProjection");
    }

    XMLElem refXML = getFirstAndOnly(projXML, "ReferencePoint");

    parseVector3D(getFirstAndOnly(refXML, "ECEF"),
                  measurement->projection->referencePoint.ecef);

    parseRowColDouble(getFirstAndOnly(refXML, "Point"),
                      measurement->projection->referencePoint.rowCol);

    if (measurement->projection->projectionType == ProjectionType::PLANE)
    {
        PlaneProjection* planeProj = (PlaneProjection*) measurement->projection;

        parseRowColDouble(getFirstAndOnly(projXML, "SampleSpacing"),
                          planeProj->sampleSpacing);

        parsePoly2D(getFirstAndOnly(projXML, "TimeCOAPoly"),
                    planeProj->timeCOAPoly);

        XMLElem prodPlaneXML = getFirstAndOnly(projXML, "ProductPlane");

        parseVector3D(getFirstAndOnly(prodPlaneXML, "RowUnitVector"),
                      planeProj->productPlane.rowUnitVector);
        parseVector3D(getFirstAndOnly(prodPlaneXML, "ColUnitVector"),
                      planeProj->productPlane.colUnitVector);
    }
    else if (measurement->projection->projectionType
            == ProjectionType::GEOGRAPHIC)
    {
        GeographicProjection* geographicProj =
                (GeographicProjection*) measurement->projection;

        parseRowColDouble(getFirstAndOnly(projXML, "SampleSpacing"),
                          geographicProj->sampleSpacing);

        parsePoly2D(getFirstAndOnly(projXML, "TimeCOAPoly"),
                    geographicProj->timeCOAPoly);
    }
    else if (measurement->projection->projectionType
            == ProjectionType::CYLINDRICAL)
    {
        // Now, we go TimeCOA, SampleSpacing, CurvatureRadius
        CylindricalProjection* cylindricalProj =
                (CylindricalProjection*) measurement->projection;

        parseRowColDouble(getFirstAndOnly(projXML, "SampleSpacing"),
                          cylindricalProj->sampleSpacing);

        parsePoly2D(getFirstAndOnly(projXML, "TimeCOAPoly"),
                    cylindricalProj->timeCOAPoly);

        parseVector3D(getFirstAndOnly(projXML, "StripmapDirection"),
                      cylindricalProj->stripmapDirection);

        XMLElem curvRadiusXML = getOptional(projXML, "CurvatureRadius");
        if (curvRadiusXML)
        {
            parseDouble(curvRadiusXML, cylindricalProj->curvatureRadius);
        }
    }
    else if (measurement->projection->projectionType
            == ProjectionType::POLYNOMIAL)
    {
        PolynomialProjection* polyProj =
                (PolynomialProjection*) measurement->projection;

        // Get a bunch of 2D polynomials
        parsePoly2D(getFirstAndOnly(projXML, "RowColToLat"),
                    polyProj->rowColToLat);

        parsePoly2D(getFirstAndOnly(projXML, "RowColToLon"),
                    polyProj->rowColToLat);

        XMLElem optionalAltPolyXML = getOptional(projXML, "RowColToAlt");
        if (optionalAltPolyXML)
        {
            parsePoly2D(optionalAltPolyXML, polyProj->rowColToAlt);
        }

        parsePoly2D(getFirstAndOnly(projXML, "LatLonToRow"),
                    polyProj->latLonToRow);

        parsePoly2D(getFirstAndOnly(projXML, "LatLonToCol"),
                    polyProj->latLonToCol);
    }
    else
        throw except::Exception(Ctxt("Unknown projection type"));

    XMLElem tmpElem = getFirstAndOnly(measurementXML, "ARPPoly");
    parsePolyXYZ(tmpElem, measurement->arpPoly);

}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem exploitationFeaturesXML,
        ExploitationFeatures* exploitationFeatures)
{
    XMLElem tmpElem;

    std::vector<XMLElem> collectionsXML;
    exploitationFeaturesXML->getElementsByTagName("Collection", collectionsXML);

    unsigned int idx = 0;
    for (std::vector<XMLElem>::const_iterator it = collectionsXML.begin(); it
            != collectionsXML.end(); ++it)
    {
        XMLElem collectionXML = *it;
        Collection* coll;
        if (exploitationFeatures->collections.size() <= idx)
        {
            coll = new Collection();
            exploitationFeatures->collections.push_back(coll);
        }
        else
        {
            coll = exploitationFeatures->collections[idx];
        }

        // Information
        Information* info = coll->information;
        XMLElem informationXML = getFirstAndOnly(collectionXML, "Information");

        info->sensorName
                = getFirstAndOnly(informationXML, "SensorName")->getCharacterData();

        XMLElem radarModeXML = getFirstAndOnly(informationXML, "RadarMode");
        info->radarMode
                = six::toType<RadarModeType>(
                                             getFirstAndOnly(radarModeXML,
                                                             "ModeType")->getCharacterData());
        tmpElem = getOptional(radarModeXML, "ModeID");
        if (tmpElem)
        {
            info->radarModeID = tmpElem->getCharacterData();
        }

        parseDateTime(getFirstAndOnly(informationXML, "CollectionDateTime"),
                      info->collectionDateTime);

        tmpElem = getOptional(informationXML, "LocalDateTime");
        if (tmpElem)
        {
            info->localDateTime = tmpElem->getCharacterData();
        }

        parseDouble(getFirstAndOnly(informationXML, "CollectionDuration"),
                    info->collectionDuration);

        // Optional in 0.2 spec.
        XMLElem resXML = getOptional(informationXML, "Resolution");
        if (resXML)
        {
            parseRangeAzimuth(resXML, info->resolution);
        }

        XMLElem roiXML = getOptional(informationXML, "InputROI");
        if (roiXML)
        {
            info->inputROI = new InputROI();

            parseRowColDouble(getFirstAndOnly(roiXML, "Size"),
                              info->inputROI->size);
            parseRowColDouble(getFirstAndOnly(roiXML, "UpperLeft"),
                              info->inputROI->upperLeft);
        }

        // optional and unbounded
        std::vector<XMLElem> polarization;
        informationXML->getElementsByTagName("Polarization", polarization);

        for (size_t i = 0, nElems = polarization.size(); i < nElems; ++i)
        {
            XMLElem polXML = polarization[i];
            TxRcvPolarization *p = new TxRcvPolarization();
            info->polarization.push_back(p);

            tmpElem = getFirstAndOnly(polXML, "TxPolarization");
            p->txPolarization
                    = six::toType<PolarizationType>(tmpElem->getCharacterData());

            tmpElem = getFirstAndOnly(polXML, "RcvPolarization");
            p->rcvPolarization
                    = six::toType<PolarizationType>(tmpElem->getCharacterData());

            tmpElem = getOptional(polXML, "RcvPolarizationOffset");
            if (tmpElem)
                parseDouble(tmpElem, p->rcvPolarizationOffset);

            std::vector<XMLElem> processed;
            polXML->getElementsByTagName("Processed", processed);
            for (size_t j = 0, nBools = processed.size(); j < nBools; ++j)
            {
                BooleanType isProcessed;
                parseBooleanType(processed[j], isProcessed);
                p->processed.push_back(isProcessed == BooleanType::IS_TRUE);
            }
        }

        // Geometry
        XMLElem geometryXML = getOptional(collectionXML, "Geometry");

        if (geometryXML)
        {
            Geometry* geom = new Geometry();
            coll->geometry = geom;

            tmpElem = getOptional(geometryXML, "Azimuth");
            if (tmpElem)
            {
                parseDouble(tmpElem, geom->azimuth);
            }

            tmpElem = getOptional(geometryXML, "Slope");
            if (tmpElem)
            {
                parseDouble(tmpElem, geom->slope);
            }

            tmpElem = getOptional(geometryXML, "Squint");
            if (tmpElem)
            {
                parseDouble(tmpElem, geom->squint);
            }

            tmpElem = getOptional(geometryXML, "Graze");
            if (tmpElem)
            {
                parseDouble(tmpElem, geom->graze);
            }

            tmpElem = getOptional(geometryXML, "Tilt");
            if (tmpElem)
            {
                parseDouble(tmpElem, geom->tilt);
            }
        }

        // Phenomenology

        XMLElem phenomenologyXML = getOptional(collectionXML, "Phenomenology");

        if (phenomenologyXML)
        {

            Phenomenology* phenom = new Phenomenology();
            coll->phenomenology = phenom;

            tmpElem = getOptional(phenomenologyXML, "Shadow");
            if (tmpElem)
            {
                parseDouble(getFirstAndOnly(tmpElem, "Angle"),
                            phenom->shadow.angle);
                parseDouble(getFirstAndOnly(tmpElem, "Magnitude"),
                            phenom->shadow.magnitude);
            }

            tmpElem = getOptional(phenomenologyXML, "Layover");
            if (tmpElem)
            {
                parseDouble(getFirstAndOnly(tmpElem, "Angle"),
                            phenom->layover.angle);
                parseDouble(getFirstAndOnly(tmpElem, "Magnitude"),
                            phenom->layover.magnitude);
            }

            tmpElem = getOptional(phenomenologyXML, "MultiPath");
            if (tmpElem)
            {
                parseDouble(tmpElem, phenom->multiPath);
            }

            tmpElem = getOptional(phenomenologyXML, "GroundTrack");
            if (tmpElem)
            {
                parseDouble(tmpElem, phenom->groundTrack);
            }
        }
    }

    XMLElem productXML = getFirstAndOnly(exploitationFeaturesXML, "Product");
    Product prod = exploitationFeatures->product;

    parseRowColDouble(getFirstAndOnly(productXML, "Resolution"),
                      exploitationFeatures->product.resolution);

    tmpElem = getOptional(productXML, "North");
    if (tmpElem)
        parseDouble(tmpElem, prod.north);
}

Data* DerivedXMLControl_0_5_0::fromXML(const xml::lite::Document* doc)
{
    XMLElem root = doc->getRootElement();

    XMLElem productCreationXML = getFirstAndOnly(root, "ProductCreation");
    XMLElem displayXML = getFirstAndOnly(root, "Display");
    XMLElem measurementXML = getFirstAndOnly(root, "Measurement");
    XMLElem exploitationFeaturesXML = getFirstAndOnly(root,
                                                      "ExploitationFeatures");
    XMLElem geographicAndTargetXML = getFirstAndOnly(root,
                                                     "GeographicAndTarget");

    DerivedDataBuilder builder;
    DerivedData *data = builder.steal(); //steal it
    data->mVersion = six::sidd::SIDD_0_5_0;

    // see if PixelType has MONO or RGB
    PixelType
            pixelType =
                    six::toType<PixelType>(
                                           getFirstAndOnly(displayXML,
                                                           "PixelType")->getCharacterData());
    builder.addDisplay(pixelType);

    RegionType regionType = RegionType::SUB_REGION;
    XMLElem tmpElem = getFirstAndOnly(geographicAndTargetXML,
                                      "GeographicCoverage");
    // see if GeographicCoverage contains SubRegion or GeographicInfo

    if (getOptional(tmpElem, "SubRegion"))
    {
        regionType = RegionType::SUB_REGION;
    }
    else if (getOptional(tmpElem, "GeographicInfo"))
    {
        regionType = RegionType::GEOGRAPHIC_INFO;
    }
    builder.addGeographicAndTarget(regionType);

    six::ProjectionType projType = ProjectionType::NOT_SET;

    if (getOptional(measurementXML, "GeographicProjection"))
        projType = ProjectionType::GEOGRAPHIC;
    else if (getOptional(measurementXML, "CylindricalProjection"))
        projType = ProjectionType::CYLINDRICAL;
    else if (getOptional(measurementXML, "PlaneProjection"))
        projType = ProjectionType::PLANE;
    else if (getOptional(measurementXML, "PolynomialProjection"))
        projType = ProjectionType::POLYNOMIAL;

    builder.addMeasurement(projType);

    std::vector<XMLElem> elements;
    exploitationFeaturesXML->getElementsByTagName("ExploitationFeatures",
                                                  elements);
    builder.addExploitationFeatures(elements.size());

    fromXML(productCreationXML, data->productCreation);
    fromXML(displayXML, data->display);
    fromXML(geographicAndTargetXML, data->geographicAndTarget);
    fromXML(measurementXML, data->measurement);
    fromXML(exploitationFeaturesXML, data->exploitationFeatures);

    XMLElem productProcessingXML = getOptional(root, "ProductProcessing");
    if (productProcessingXML)
    {
        builder.addProductProcessing();
        fromXML(productProcessingXML, data->productProcessing);
    }

    XMLElem downstreamReprocessingXML = getOptional(root,
                                                    "DownstreamReprocessing");
    if (downstreamReprocessingXML)
    {
        builder.addDownstreamReprocessing();
        fromXML(downstreamReprocessingXML, data->downstreamReprocessing);
    }

    XMLElem errorStatisticsXML = getOptional(root, "ErrorStatistics");
    if (errorStatisticsXML)
    {
        builder.addErrorStatistics();
        XMLControl::fromXML(errorStatisticsXML, data->errorStatistics);
    }

    XMLElem radiometricXML = getOptional(root, "Radiometric");
    if (radiometricXML)
    {
        builder.addRadiometric();
        XMLControl::fromXML(radiometricXML, data->radiometric);
    }

    XMLElem annotationsXML = getOptional(root, "Annotations");
    if (annotationsXML)
    {
        std::vector<XMLElem> annChildren;
        annotationsXML->getElementsByTagName("Annotation", annChildren);
        for (unsigned int i = 0, size = annChildren.size(); i < size; ++i)
        {
            Annotation *a = new Annotation();
            fromXML(annChildren[i], a);
            data->annotations.push_back(a);
        }
    }

    return data;
}

XMLElem DerivedXMLControl_0_5_0::toXML(const ProductCreation* productCreation,
        XMLElem parent)
{
    // Make the XML node
    XMLElem productCreationXML = newElement("ProductCreation", parent);

    // Processor Info
    XMLElem procInfoXML =
            newElement("ProcessorInformation", productCreationXML);

    createString("Application",
                 productCreation->processorInformation->application,
                 procInfoXML);

    createDateTime("ProcessingDateTime",
                   productCreation->processorInformation->processingDateTime,
                   procInfoXML);

    createString("Site", productCreation->processorInformation->site,
                 procInfoXML);

    //Profile - optional
    if (productCreation->processorInformation->profile != Init::undefined<
            std::string>())
    {
        createString("Profile", productCreation->processorInformation->profile,
                     procInfoXML);
    }

    //Classification
    XMLElem classXML = newElement("Classification", productCreationXML);
    createString("Level", productCreation->classification.level, classXML);

    for (std::vector<std::string>::const_iterator it =
            productCreation->classification.securityMarkings.begin(); it
            != productCreation->classification.securityMarkings.end(); ++it)
    {
        std::string marking = *it;
        str::trim(marking);
        if (!marking.empty())
            createString("SecurityMarkings", *it, classXML);
    }

    if (productCreation->classification.guidance)
    {
        XMLElem guidanceXML = newElement("Guidance", classXML);
        createString("Authority",
                     productCreation->classification.guidance->authority,
                     guidanceXML);
        createDate("Date", productCreation->classification.guidance->date,
                   guidanceXML);
    }

    //ProductName
    createString("ProductName", productCreation->productName,
                 productCreationXML);

    //ProductClass
    createString("ProductClass", productCreation->productClass,
                 productCreationXML);

    //ProductType - optional
    if (productCreation->productType != Init::undefined<std::string>())
    {
        createString("ProductType", productCreation->productType,
                     productCreationXML);
    }

    //product extensions
    addParameters("ProductCreationExtension",
                  productCreation->productCreationExtensions,
                  productCreationXML);
    return productCreationXML;
}

XMLElem DerivedXMLControl_0_5_0::toXML(const Display* display, XMLElem parent)
{
    XMLElem displayXML = newElement("Display", parent);

    createString("PixelType", six::toString(display->pixelType), displayXML);

    //RemapInformation - optional
    if (display->remapInformation)
    {
        XMLElem remapInfoXML = newElement("RemapInformation", displayXML);

        if (display->remapInformation->displayType == DisplayType::COLOR)
        {
            XMLElem remapXML = newElement("ColorDisplayRemap", remapInfoXML);
            if (display->remapInformation->remapLUT)
                createLUT("RemapLUT", display->remapInformation->remapLUT,
                          remapXML);
        }
        else if (display->remapInformation->displayType == DisplayType::MONO)
        {
            XMLElem remapXML = newElement("MonochromeDisplayRemap",
                                          remapInfoXML);
            // a little risky, but let's assume the displayType is correct
            MonochromeDisplayRemap* mdr =
                    (MonochromeDisplayRemap*) display->remapInformation;
            createString("RemapType", mdr->remapType, remapXML);
            if (mdr->remapLUT)
                createLUT("RemapLUT", mdr->remapLUT, remapXML);
            addParameters("RemapParameter", mdr->remapParameters, remapXML);
        }
    }

    //MagnificationMethod - optional
    if (display->magnificationMethod != MagnificationMethod::NOT_SET)
    {
        createString("MagnificationMethod",
                     six::toString(display->magnificationMethod), displayXML);
    }

    //DecimationMethod - optional
    if (display->decimationMethod != DecimationMethod::NOT_SET)
    {
        createString("DecimationMethod",
                     six::toString(display->decimationMethod), displayXML);
    }

    //DRAHistogramOverrides - optional
    if (display->histogramOverrides)
    {
        XMLElem histo = newElement("DRAHistogramOverrides", displayXML);
        createInt("ClipMin", display->histogramOverrides->clipMin, histo);
        createInt("ClipMax", display->histogramOverrides->clipMax, histo);
    }

    if (display->monitorCompensationApplied)
    {
        //MonitorCompensationApplied - optional
        XMLElem monComp = newElement("MonitorCompensationApplied", displayXML);
        createDouble("Gamma", display->monitorCompensationApplied->gamma,
                     monComp);
        createDouble("XMin", display->monitorCompensationApplied->xMin, monComp);
    }

    addParameters("DisplayExtension", display->displayExtensions, displayXML);
    return displayXML;
}

XMLElem DerivedXMLControl_0_5_0::toXML(
        const GeographicAndTarget* geographicAndTarget, XMLElem parent)
{
    XMLElem geographicAndTargetXML = newElement("GeographicAndTarget", parent);

    toXML(geographicAndTarget->geographicCoverage, geographicAndTargetXML);

    //loop over TargetInformation
    for (std::vector<TargetInformation*>::const_iterator it =
            geographicAndTarget->targetInformation.begin(); it
            != geographicAndTarget->targetInformation.end(); ++it)
    {
        TargetInformation* ti = *it;
        XMLElem tiXML = newElement("TargetInformation", geographicAndTargetXML);

        addParameters("Identifier", ti->identifiers, tiXML);
        for (unsigned int i = 0; i < ti->footprints.size(); i++)
        {
            createFootprint("Footprint", "Vertex", ti->footprints[i], tiXML);
        }
        addParameters("TargetInformationExtension",
                      ti->targetInformationExtensions, tiXML);
    }
    return geographicAndTargetXML;
}

XMLElem DerivedXMLControl_0_5_0::toXML(const GeographicCoverage* geoCoverage,
        XMLElem parent)
{
    //GeographicAndTarget
    XMLElem geoCoverageXML = newElement("GeographicCoverage", parent);

    //Georegion Identifiers
    addParameters("GeoregionIdentifier", geoCoverage->georegionIdentifiers,
                  geoCoverageXML);

    //Footprint
    createFootprint("Footprint", "Vertex", geoCoverage->footprint,
                    geoCoverageXML);

    // GeographicInfo
    if (geoCoverage->geographicInformation)
    {
        XMLElem geoInfoXML = newElement("GeographicInfo", geoCoverageXML);

        size_t numCC = geoCoverage->geographicInformation->countryCodes.size();
        for (size_t i = 0; i < numCC; ++i)
        {
            createString("CountryCode",
                         geoCoverage->geographicInformation->countryCodes[i],
                         geoInfoXML);
        }
        // always write at least one - it is required - possibly remove this
        if (numCC == 0)
        {
            createString("CountryCode", "", geoInfoXML);
        }

        // optional, so check if empty
        std::string secInfo =
                geoCoverage->geographicInformation->securityInformation;
        str::trim(secInfo);
        if (!secInfo.empty())
            createString("SecurityInfo", secInfo, geoInfoXML);

        addParameters(
                      "GeographicInfoExtension",
                      geoCoverage->geographicInformation->geographicInformationExtensions,
                      geoInfoXML);
    }
    else
    {
        //loop over SubRegions
        for (std::vector<GeographicCoverage*>::const_iterator it =
                geoCoverage->subRegion.begin(); it
                != geoCoverage->subRegion.end(); ++it)
        {
            XMLElem subRegionXML = toXML(*it, geoCoverageXML);
            subRegionXML->setQName("SubRegion");
        }
    }
    return geoCoverageXML;
}

XMLElem DerivedXMLControl_0_5_0::toXML(const Measurement* measurement, XMLElem parent)
{
    XMLElem measurementXML = newElement("Measurement", parent);

    Projection* projection = measurement->projection;
    XMLElem projectionXML = newElement("", measurementXML);
    XMLElem referencePointXML = newElement("ReferencePoint", projectionXML);
    setAttribute(referencePointXML, "name", projection->referencePoint.name);

    //ECEF
    createVector3D("ECEF", projection->referencePoint.ecef, referencePointXML);
    //Point
    createRowCol("Point", projection->referencePoint.rowCol, referencePointXML);

    switch (projection->projectionType)
    {
    case ProjectionType::PLANE:
    {
        projectionXML->setLocalName("PlaneProjection");
        PlaneProjection* planeProj = (PlaneProjection*) projection;

        createRowCol("SampleSpacing", planeProj->sampleSpacing, projectionXML);

        //TimeCOAPoly
        XMLElem timeCOAPolyXML = createPoly2D("TimeCOAPoly",
                                              planeProj->timeCOAPoly,
                                              projectionXML);

        XMLElem productPlaneXML = newElement("ProductPlane", projectionXML);

        //RowBasis
        createVector3D("RowUnitVector", planeProj->productPlane.rowUnitVector,
                       productPlaneXML);

        //ColBasis
        createVector3D("ColUnitVector", planeProj->productPlane.colUnitVector,
                       productPlaneXML);

    }
        break;

    case ProjectionType::GEOGRAPHIC:
    {
        projectionXML->setLocalName("GeographicProjection");
        GeographicProjection* geographicProj =
                (GeographicProjection*) projection;

        XMLElem sampleSpacingXML = createRowCol("SampleSpacing",
                                                geographicProj->sampleSpacing,
                                                projectionXML);

        //TimeCOAPoly
        XMLElem timeCOAPolyXML = createPoly2D("TimeCOAPoly",
                                              geographicProj->timeCOAPoly,
                                              projectionXML);

    }
        break;

    case ProjectionType::CYLINDRICAL:
    {
        projectionXML->setLocalName("CylindricalProjection");

        CylindricalProjection* cylindricalProj =
                (CylindricalProjection*) projection;

        createRowCol("SampleSpacing", cylindricalProj->sampleSpacing,
                     projectionXML);

        //TimeCOAPoly
        XMLElem timeCOAPolyXML = createPoly2D("TimeCOAPoly",
                                              cylindricalProj->timeCOAPoly,
                                              projectionXML);

        createVector3D("StripmapDirection", cylindricalProj->stripmapDirection,
                       projectionXML);

        if (cylindricalProj->curvatureRadius != Init::undefined<double>())
        {
            createDouble("CurvatureRadius", cylindricalProj->curvatureRadius,
                         projectionXML);
        }

    }
        break;

    case ProjectionType::POLYNOMIAL:
    {
        projectionXML->setLocalName("PolynomialProjection");
        PolynomialProjection* polyProj = (PolynomialProjection*) projection;
        createPoly2D("RowColToLat", polyProj->rowColToLat, projectionXML);
        createPoly2D("RowColToLon", polyProj->rowColToLon, projectionXML);

        if (polyProj->rowColToAlt != Init::undefined<Poly2D>())
        {
            createPoly2D("RowColToAlt", polyProj->rowColToAlt, projectionXML);
        }
        createPoly2D("LatLonToRow", polyProj->latLonToRow, projectionXML);
        createPoly2D("LatLonToCol", polyProj->latLonToCol, projectionXML);
    }
        break;

    default:
        throw except::Exception(Ctxt("Unknown projection type!"));
    }

    //Pixel Footprint
    createRowCol("PixelFootprint", measurement->pixelFootprint, measurementXML);

    //ARPPoly
    createPolyXYZ("ARPPoly", measurement->arpPoly, measurementXML);

    return measurementXML;
}

XMLElem DerivedXMLControl_0_5_0::toXML(
        const ExploitationFeatures* exploitationFeatures, XMLElem parent)
{

    XMLElem exploitationFeaturesXML =
            newElement("ExploitationFeatures", parent);

    for (unsigned int i = 0; i < exploitationFeatures->collections.size(); ++i)
    {
        Collection* collection = exploitationFeatures->collections[i];
        XMLElem collectionXML = newElement("Collection",
                                           exploitationFeaturesXML);
        setAttribute(collectionXML, "identifier", collection->identifier);

        XMLElem informationXML = newElement("Information", collectionXML);

        createString("SensorName", collection->information->sensorName,
                     informationXML);

        XMLElem radarModeXML = newElement("RadarMode", informationXML);
        //ModeType - Incomplete?
        createString("ModeType",
                     six::toString(collection->information->radarMode),
                     radarModeXML);
        if (collection->information->radarModeID
                != Init::undefined<std::string>())
        {
            createString("ModeID", collection->information->radarModeID,
                         radarModeXML);
        }

        //CollectionDateTime
        createDateTime("CollectionDateTime",
                       collection->information->collectionDateTime,
                       informationXML);

        //LocalDateTime
        if (collection->information->localDateTime != Init::undefined<
                std::string>())
        {
            createDateTime("LocalDateTime",
                           collection->information->localDateTime,
                           informationXML);
        }

        createDouble("CollectionDuration",
                     collection->information->collectionDuration,
                     informationXML);

        //Resolution - optional
        if (!Init::isUndefined(collection->information->resolution))
        {
            createRangeAzimuth("Resolution",
                               collection->information->resolution,
                               informationXML);
        }

        //InputROI
        if (collection->information->inputROI != NULL)
        {
            XMLElem roiXML = newElement("InputROI", informationXML);
            createRowCol("Size", collection->information->inputROI->size,
                         roiXML);
            createRowCol("UpperLeft",
                         collection->information->inputROI->upperLeft, roiXML);
        }

        for (size_t n = 0, nElems =
                collection->information->polarization.size(); n < nElems; ++n)
        {
            TxRcvPolarization *p = collection->information->polarization[n];
            XMLElem polXML = newElement("Polarization", informationXML);
            createString("TxPolarization", six::toString(p->txPolarization),
                         polXML);
            createString("RcvPolarization", six::toString(p->rcvPolarization),
                         polXML);

            if (!Init::isUndefined(p->rcvPolarizationOffset))
            {
                createDouble("RcvPolarization", p->rcvPolarizationOffset,
                             polXML);
            }
            for (size_t b = 0, nBools = p->processed.size(); b < nBools; ++b)
            {
                createString("Processed", six::toString(p->processed[b]),
                             polXML);
            }
        }

        //Geometry
        Geometry* geom = collection->geometry;

        if (geom != NULL)
        {
            XMLElem geometryXML = newElement("Geometry", collectionXML);

            if (geom->azimuth != Init::undefined<double>())
            {
                createDouble("Azimuth", geom->azimuth, geometryXML);
            }

            if (geom->slope != Init::undefined<double>())
            {
                createDouble("Slope", geom->slope, geometryXML);
            }

            if (geom->squint != Init::undefined<double>())
            {
                createDouble("Squint", geom->squint, geometryXML);
            }

            if (geom->graze != Init::undefined<double>())
            {
                createDouble("Graze", geom->graze, geometryXML);
            }

            if (geom->tilt != Init::undefined<double>())
            {
                createDouble("Tilt", geom->tilt, geometryXML);
            }
        }

        //Phenomenology
        Phenomenology* phenom = collection->phenomenology;
        if (phenom != NULL)
        {
            XMLElem phenomenologyXML = newElement("Phenomenology",
                                                  collectionXML);

            if (phenom->shadow != Init::undefined<AngleMagnitude>())
            {
                XMLElem shadow = newElement("Shadow", phenomenologyXML);
                createDouble("Angle", phenom->shadow.angle, shadow);
                createDouble("Magnitude", phenom->shadow.magnitude, shadow);
            }

            if (phenom->layover != Init::undefined<AngleMagnitude>())
            {
                XMLElem layover = newElement("Layover", phenomenologyXML);
                createDouble("Angle", phenom->layover.angle, layover);
                createDouble("Magnitude", phenom->layover.magnitude, layover);
            }

            if (phenom->multiPath != Init::undefined<double>())
            {
                createDouble("MultiPath", phenom->multiPath, phenomenologyXML);
            }

            if (phenom->groundTrack != Init::undefined<double>())
            {
                createDouble("GroundTrack", phenom->groundTrack,
                             phenomenologyXML);
            }
        }
    }

    //Product
    XMLElem productXML = newElement("Product", exploitationFeaturesXML);

    //Resolution
    createRowCol("Resolution", exploitationFeatures->product.resolution,
                 productXML);

    //North - optional
    if (exploitationFeatures->product.north != Init::undefined<double>())
    {
        createDouble("North", exploitationFeatures->product.north, productXML);
    }
    return exploitationFeaturesXML;
}

xml::lite::Document* DerivedXMLControl_0_5_0::toXML(const Data* data)
{
    if (data->getDataType() != DataType::DERIVED)
    {
        throw except::Exception("Data must be derived");
    }
    xml::lite::Document* doc = new xml::lite::Document();
    XMLElem root = newElement("SIDD");
    doc->setRootElement(root);

    const DerivedData *derived = (const DerivedData*) data;

    toXML(derived->productCreation, root);
    toXML(derived->display, root);
    toXML(derived->geographicAndTarget, root);
    toXML(derived->measurement, root);
    toXML(derived->exploitationFeatures, root);

    if (derived->productProcessing)
    {
        toXML(derived->productProcessing, root);
    }
    if (derived->errorStatistics)
    {
        XMLControl::toXML(derived->errorStatistics, root);
    }
    if (derived->radiometric)
    {
        XMLControl::toXML(derived->radiometric, root);
    }

    if (!derived->annotations.empty())
    {
        XMLElem annotationsElem = newElement("Annotations", root);
        for (size_t i = 0, num = derived->annotations.size(); i < num; ++i)
        {
            toXML(derived->annotations[i], annotationsElem);
        }
    }

    //set the XMLNS
    root->setNamespacePrefix("", getDefaultURI());
    root->setNamespacePrefix("si", getSICommonURI());
    root->setNamespacePrefix("sfa", getSFAURI());

    return doc;
}

XMLElem DerivedXMLControl_0_5_0::createLUT(std::string name, const LUT *lut,
        XMLElem parent)
{
    //     unsigned char* table;
    //     unsigned int numEntries;
    //     unsigned int elementSize;

    XMLElem lutElement = newElement(name, parent);
    setAttribute(lutElement, "size", str::toString(lut->numEntries));

    std::ostringstream oss;
    for (unsigned int i = 0; i < lut->numEntries; ++i)
    {
        if (lut->elementSize == 2)
        {
            short* idx = (short*) (*lut)[i];
            oss << *idx;
        }
        else if (lut->elementSize == 3)
        {
            //must add 128 to print ASCII
            oss << (unsigned int) (*lut)[i][0] << ','
                    << (unsigned int) (*lut)[i][1] << ','
                    << (unsigned int) (*lut)[i][2];
        }
        else
        {
            throw except::Exception(Ctxt(FmtX("Invalid element size [%d]",
                                              lut->elementSize)));
        }
        if ((lut->numEntries - 1) != i)
            oss << ' ';
    }
    lutElement->setCharacterData(oss.str());
    return lutElement;
}

XMLElem DerivedXMLControl_0_5_0::toXML(const ProductProcessing* productProcessing,
        XMLElem parent)
{
    XMLElem productProcessingXML = newElement("ProductProcessing", parent);

    for (std::vector<ProcessingModule*>::const_iterator it =
            productProcessing->processingModules.begin(); it
            != productProcessing->processingModules.end(); ++it)
    {
        toXML(*it, productProcessingXML);
    }
    return productProcessingXML;
}

XMLElem DerivedXMLControl_0_5_0::toXML(const ProcessingModule* procMod,
        XMLElem parent)
{
    XMLElem procModXML = newElement("ProcessingModule", parent);
    XMLElem modNameXML = createString("ModuleName", procMod->moduleName.str(),
                                      procModXML);
    setAttribute(modNameXML, "name", procMod->moduleName.getName());

    if (!procMod->processingModules.empty())
    {
        for (std::vector<ProcessingModule*>::const_iterator it =
                procMod->processingModules.begin(); it
                != procMod->processingModules.end(); ++it)
        {
            toXML(*it, procModXML);
        }
    }
    else
    {
        addParameters("ModuleParameter", procMod->moduleParameters, procModXML);
    }
    return procModXML;

}

XMLElem DerivedXMLControl_0_5_0::toXML(
        const DownstreamReprocessing* downstreamReproc, XMLElem parent)
{
    XMLElem epXML = newElement("DownstreamReprocessing", parent);

    GeometricChip *geoChip = downstreamReproc->geometricChip;
    if (geoChip)
    {
        XMLElem geoChipXML = newElement("GeometricChip", epXML);
        createRowCol("ChipSize", geoChip->chipSize, geoChipXML);
        createRowCol("OriginalUpperLeftCoordinate",
                     geoChip->originalUpperLeftCoordinate, geoChipXML);
        createRowCol("OriginalUpperRightCoordinate",
                     geoChip->originalUpperRightCoordinate, geoChipXML);
        createRowCol("OriginalLowerLeftCoordinate",
                     geoChip->originalLowerLeftCoordinate, geoChipXML);
        createRowCol("OriginalLowerRightCoordinate",
                     geoChip->originalLowerRightCoordinate, geoChipXML);
    }
    if (!downstreamReproc->processingEvents.empty())
    {
        for (std::vector<ProcessingEvent*>::const_iterator it =
                downstreamReproc->processingEvents.begin(); it
                != downstreamReproc->processingEvents.end(); ++it)
        {
            ProcessingEvent *procEvent = *it;
            XMLElem procEventXML = newElement("ProcessingEvent", epXML);
            createString("ApplicationName", procEvent->applicationName,
                         procEventXML);
            createDateTime("AppliedDateTime", procEvent->appliedDateTime,
                           procEventXML);

            if (procEvent->interpolationMethod.empty())
            {
                createString("InterpolationMethod",
                             procEvent->interpolationMethod, procEventXML);
            }
            addParameters("Descriptor", procEvent->descriptor, procEventXML);
        }
    }
    return epXML;
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem procXML,
        ProcessingModule* procMod)
{
    XMLElem moduleName = getFirstAndOnly(procXML, "ModuleName");
    procMod->moduleName = Parameter(moduleName->getCharacterData());
    procMod->moduleName.setName(moduleName->getAttributes().getValue("name"));

    parseParameters(procXML, "ModuleParameter", procMod->moduleParameters);

    std::vector<XMLElem> procModuleXML;
    procXML->getElementsByTagName("ProcessingModule", procModuleXML);

    for (unsigned int i = 0, size = procModuleXML.size(); i < size; ++i)
    {
        ProcessingModule *pm = new ProcessingModule();
        fromXML(procModuleXML[i], pm);
        procMod->processingModules.push_back(pm);
    }
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem elem,
        ProductProcessing* productProcessing)
{
    std::vector<XMLElem> procModuleXML;
    elem->getElementsByTagName("ProcessingModule", procModuleXML);

    for (unsigned int i = 0, size = procModuleXML.size(); i < size; ++i)
    {
        ProcessingModule *procMod = new ProcessingModule();
        fromXML(procModuleXML[i], procMod);
        productProcessing->processingModules.push_back(procMod);
    }
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem elem,
        DownstreamReprocessing* downstreamReproc)
{
    XMLElem geometricChipXML = getOptional(elem, "GeometricChip");
    std::vector<XMLElem> procEventXML;
    elem->getElementsByTagName("ProcessingEvent", procEventXML);
    XMLElem processingEventXML = getOptional(elem, "ProcessingEvent");

    if (geometricChipXML)
    {
        GeometricChip *chip = downstreamReproc->geometricChip
                = new GeometricChip();

        parseRowColInt(getFirstAndOnly(geometricChipXML, "ChipSize"),
                       chip->chipSize);

        parseRowColDouble(getFirstAndOnly(geometricChipXML,
                                          "OriginalUpperLeftCoordinate"),
                          chip->originalUpperLeftCoordinate);

        parseRowColDouble(getFirstAndOnly(geometricChipXML,
                                          "OriginalUpperRightCoordinate"),
                          chip->originalUpperRightCoordinate);

        parseRowColDouble(getFirstAndOnly(geometricChipXML,
                                          "OriginalLowerLeftCoordinate"),
                          chip->originalLowerLeftCoordinate);

        parseRowColDouble(getFirstAndOnly(geometricChipXML,
                                          "OriginalLowerRightCoordinate"),
                          chip->originalLowerRightCoordinate);
    }

    for (unsigned int i = 0, size = procEventXML.size(); i < size; ++i)
    {
        ProcessingEvent *procEvent = new ProcessingEvent();
        downstreamReproc->processingEvents.push_back(procEvent);
        XMLElem peXML = procEventXML[i];

        procEvent->applicationName
                = getFirstAndOnly(peXML, "ApplicationName")->getCharacterData();
        parseDateTime(getFirstAndOnly(peXML, "AppliedDateTime"),
                      procEvent->appliedDateTime);

        XMLElem tmpElem = getOptional(peXML, "InterpolationMethod");
        if (tmpElem)
        {
            procEvent->interpolationMethod = tmpElem->getCharacterData();
        }
        parseParameters(peXML, "Descriptor", procEvent->descriptor);
    }
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem elem, Annotation *a)
{
    a->identifier = getFirstAndOnly(elem, "Identifier")->getCharacterData();
    XMLElem spatialXML = getOptional(elem, "SpatialReferenceSystem");
    if (spatialXML)
    {
        //TODO
    }

    std::vector<XMLElem> objectsXML;
    elem->getElementsByTagName("Object", objectsXML);
    for (unsigned int i = 0, size = objectsXML.size(); i < size; ++i)
    {
        XMLElem obj = objectsXML[i];
        //there should be only one child - a choice between types
        std::vector<XMLElem> &children = obj->getChildren();
        if (children.size() > 0)
        {
            //just get the first one
            XMLElem child = children[0];
            SFAGeometry *geoType = NULL;
            std::string childName = child->getLocalName();
            if (childName == "Point")
            {
                geoType = new SFAPoint;
            }
            else if (childName == "Line")
            {
                geoType = new SFALine;
            }
            else if (childName == "LinearRing")
            {
                geoType = new SFALinearRing;
            }
            else if (childName == "Polygon")
            {
                geoType = new SFAPolygon;
            }
            else if (childName == "PolyhedralSurface")
            {
                geoType = new SFAPolyhedralSurface;
            }
            else if (childName == "MultiPolygon")
            {
                geoType = new SFAMultiPolygon;
            }
            else if (childName == "MultiLineString")
            {
                geoType = new SFAMultiLineString;
            }
            else if (childName == "MultiPoint")
            {
                geoType = new SFAMultiPoint;
            }
            if (geoType)
            {
                fromXML(child, geoType);
                a->objects.push_back(geoType);
            }
        }
    }
}

XMLElem DerivedXMLControl_0_5_0::toXML(const Annotation *a, XMLElem parent)
{
    XMLElem annXML = newElement("Annotation", parent);
    createString("Identifier", a->identifier, annXML);

    //TODO spatialreferencesystem

    for (size_t i = 0, num = a->objects.size(); i < num; ++i)
    {
        XMLElem objXML = newElement("Object", annXML);
        toXML(a->objects[i], "", objXML);
    }
    return annXML;
}

void DerivedXMLControl_0_5_0::fromXML(const XMLElem elem, SFAGeometry *g)
{
    std::string geoType = g->getType();
    if (geoType == SFAPoint::TYPE_NAME)
    {
        SFAPoint *p = (SFAPoint*) g;
        parseDouble(getFirstAndOnly(elem, "X"), p->x);
        parseDouble(getFirstAndOnly(elem, "Y"), p->y);

        XMLElem tmpElem = getOptional(elem, "Z");
        if (tmpElem)
            parseDouble(tmpElem, p->z);

        tmpElem = getOptional(elem, "M");
        if (tmpElem)
            parseDouble(tmpElem, p->m);
    }
    //for now, line, linearring, and linestring are parsed the same
    else if (geoType == SFALine::TYPE_NAME || geoType
            == SFALinearRing::TYPE_NAME || geoType == SFALineString::TYPE_NAME)
    {
        //cast to the common base - LineString
        SFALineString *p = (SFALineString*) g;
        std::vector<XMLElem> vXML;
        elem->getElementsByTagName("Vertex", vXML);
        for (unsigned int i = 0, size = vXML.size(); i < size; ++i)
        {
            SFAPoint *vertex = new SFAPoint;
            fromXML(vXML[i], vertex);
            p->vertices.push_back(vertex);
        }
    }
    else if (geoType == SFAPolygon::TYPE_NAME)
    {
        SFAPolygon *p = (SFAPolygon*) g;
        std::vector<XMLElem> ringXML;
        elem->getElementsByTagName("Ring", ringXML);
        for (unsigned int i = 0, size = ringXML.size(); i < size; ++i)
        {
            SFALinearRing *ring = new SFALinearRing;
            fromXML(ringXML[i], ring);
            p->rings.push_back(ring);
        }
    }
    else if (geoType == SFAPolyhedralSurface::TYPE_NAME)
    {
        SFAPolyhedralSurface *p = (SFAPolyhedralSurface*) g;
        std::vector<XMLElem> polyXML;
        elem->getElementsByTagName("Patch", polyXML);
        for (unsigned int i = 0, size = polyXML.size(); i < size; ++i)
        {
            SFAPolygon *polygon = new SFAPolygon;
            fromXML(polyXML[i], polygon);
            p->patches.push_back(polygon);
        }
    }
    else if (geoType == SFAMultiPolygon::TYPE_NAME)
    {
        SFAMultiPolygon *p = (SFAMultiPolygon*) g;
        std::vector<XMLElem> polyXML;
        elem->getElementsByTagName("Element", polyXML);
        for (unsigned int i = 0, size = polyXML.size(); i < size; ++i)
        {
            SFAPolygon *polygon = new SFAPolygon;
            fromXML(polyXML[i], polygon);
            p->elements.push_back(polygon);
        }
    }
    else if (geoType == SFAMultiLineString::TYPE_NAME)
    {
        SFAMultiLineString *p = (SFAMultiLineString*) g;
        std::vector<XMLElem> lineXML;
        elem->getElementsByTagName("Element", lineXML);
        for (unsigned int i = 0, size = lineXML.size(); i < size; ++i)
        {
            SFALineString *ls = new SFALineString;
            fromXML(lineXML[i], ls);
            p->elements.push_back(ls);
        }
    }
    else if (geoType == SFAMultiPoint::TYPE_NAME)
    {
        SFAMultiPoint *p = (SFAMultiPoint*) g;
        std::vector<XMLElem> vXML;
        elem->getElementsByTagName("Vertex", vXML);
        for (unsigned int i = 0, size = vXML.size(); i < size; ++i)
        {
            SFAPoint *vertex = new SFAPoint;
            fromXML(vXML[i], vertex);
            p->vertices.push_back(vertex);
        }
    }
    else
    {
        mLog->warn(Ctxt(FmtX("Unable to parse unknown geometry type")));
    }
}

XMLElem DerivedXMLControl_0_5_0::toXML(const SFAGeometry *g, std::string useName,
        XMLElem parent)
{
    XMLElem geoXML = NULL;

    std::string topURI = useName.empty() ? getDefaultURI() : getSFAURI();
    std::string sfaURI = getSFAURI();

    std::string geoType = g->getType();
    if (geoType == SFAPoint::TYPE_NAME)
    {
        geoXML
                = newElement(useName.empty() ? "Point" : useName, topURI,
                             parent);
        SFAPoint *p = (SFAPoint*) g;
        createDouble("X", sfaURI, p->x, geoXML);
        createDouble("Y", sfaURI, p->y, geoXML);
        if (!Init::isUndefined(p->z))
            createDouble("Z", sfaURI, p->z, geoXML);
        if (!Init::isUndefined(p->m))
            createDouble("M", sfaURI, p->m, geoXML);
    }
    //for now, line, linearring, and linestring are parsed the same
    else if (geoType == SFALine::TYPE_NAME || geoType
            == SFALinearRing::TYPE_NAME || geoType == SFALineString::TYPE_NAME)
    {
        std::string
                eName =
                        useName.empty() ? (geoType == SFALine::TYPE_NAME ? "Line"
                                                                         : (geoType
                                                                                == SFALinearRing::TYPE_NAME ? "LinearRing"
                                                                                                            : "LineString"))
                                        : useName;
        geoXML = newElement(eName, topURI, parent);

        //cast to the common base - LineString
        SFALineString *p = (SFALineString*) g;
        for (size_t i = 0, size = p->vertices.size(); i < size; ++i)
        {
            toXML(p->vertices[i], "Vertex", geoXML);
        }
    }
    else if (geoType == SFAPolygon::TYPE_NAME)
    {
        geoXML = newElement(useName.empty() ? "Polygon" : useName, topURI,
                            parent);
        SFAPolygon *p = (SFAPolygon*) g;
        for (size_t i = 0, size = p->rings.size(); i < size; ++i)
        {
            toXML(p->rings[i], "Ring", geoXML);
        }
    }
    else if (geoType == SFAPolyhedralSurface::TYPE_NAME)
    {
        geoXML = newElement(useName.empty() ? "PolyhedralSurface" : useName,
                            topURI, parent);
        SFAPolyhedralSurface *p = (SFAPolyhedralSurface*) g;
        for (size_t i = 0, size = p->patches.size(); i < size; ++i)
        {
            toXML(p->patches[i], "Patch", geoXML);
        }
    }
    else if (geoType == SFAMultiPolygon::TYPE_NAME)
    {
        geoXML = newElement(useName.empty() ? "MultiPolygon" : useName, topURI,
                            parent);
        SFAMultiPolygon *p = (SFAMultiPolygon*) g;
        for (size_t i = 0, size = p->elements.size(); i < size; ++i)
        {
            toXML(p->elements[i], "Element", geoXML);
        }
    }
    else if (geoType == SFAMultiLineString::TYPE_NAME)
    {
        geoXML = newElement(useName.empty() ? "MultiLineString" : useName,
                            topURI, parent);
        SFAMultiLineString *p = (SFAMultiLineString*) g;
        for (size_t i = 0, size = p->elements.size(); i < size; ++i)
        {
            toXML(p->elements[i], "Element", geoXML);
        }
    }
    else if (geoType == SFAMultiPoint::TYPE_NAME)
    {
        geoXML = newElement(useName.empty() ? "MultiPoint" : useName, topURI,
                            parent);
        SFAMultiPoint *p = (SFAMultiPoint*) g;
        for (size_t i = 0, size = p->vertices.size(); i < size; ++i)
        {
            toXML(p->vertices[i], "Vertex", geoXML);
        }
    }
    else
    {
        throw except::InvalidArgumentException(
                                               Ctxt(
                                                    FmtX(
                                                         "Invalid geo type: [%s]",
                                                         geoType.c_str())));
    }

    return geoXML;
}
