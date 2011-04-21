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
#ifndef __SIX_DERIVED_DATA_H__
#define __SIX_DERIVED_DATA_H__

#include "six/Data.h"
#include "six/ErrorStatistics.h"
#include "six/sidd/ProductCreation.h"
#include "six/sidd/Display.h"
#include "six/sidd/GeographicAndTarget.h"
#include "six/sidd/Measurement.h"
#include "six/sidd/ExploitationFeatures.h"
#include "six/sidd/ProductProcessing.h"
#include "six/sidd/DownstreamReprocessing.h"
#include "six/Radiometric.h"
#include "six/sidd/Annotations.h"
#include "six/sidd/Enums.h"

namespace six
{

namespace sidd
{
/*!
 *  \class DerivedData
 *  \brief The implementation of Data for derived products
 *
 *  Sub-class of Data for handling Derived Products (SIDD)
 *  Contains the structs that are the model for SIDD products
 *
 */
struct DerivedData: public Data
{

    /*!
     *  Information related to processor, classification,
     *  and product type
     */
    ProductCreation* productCreation;

    /*!
     *  Contains information on the parameters needed to display
     *  the product in an exploitation tool
     */
    Display* display;

    /*!
     *  Contains generic and extensible targeting and geographic
     *  region information.
     */
    GeographicAndTarget* geographicAndTarget;

    /*!
     *  Contains the meta-data necessary for performing
     *  measurements
     */
    Measurement* measurement;

    /*!
     *  Computed metadata for collections
     */
    ExploitationFeatures* exploitationFeatures;

    /*!
     *  (Optional) Contains meta-data related to algorithms used
     *  during product generation
     */
    ProductProcessing* productProcessing;

    /*!
     *  (Optional) Contains meta-data related to downstream
     *  processing of the product
     */
    DownstreamReprocessing* downstreamReprocessing;

    /*!
     *  (Optional) Contains error statistics structures
     */
    ErrorStatistics* errorStatistics;

    /*!
     *  (Optional) Contains radiometric calibration params
     */
    Radiometric* radiometric;

    /*!
     * (Optional) Contains SFA annotations
     */
    Annotations annotations;

    /*!
     *  Constructor.  Creates only the product creation.  All other
     *  elements are set to NULL.  A builder should be used where possible
     *  rather than invoking this object directly.
     */
    DerivedData();
    
    //!  Destructor.  Deletes all initialized child elements
    ~DerivedData();

    /*!
     *  We are dealing with derived data
     */
    DataType getDataType() const
    {
        return DataType::DERIVED;
    }

    /*!
     *  Make a deep copy of all of the objects here.
     */
    virtual Data* clone() const;

    /*!
     *  Maps to:
     *  /SIDD/Measurement/PixelFootprint/Row
     */
    virtual unsigned long getNumRows() const
    {
        return measurement->pixelFootprint.row;
    }

    /*!
     *  Maps to:
     *  /SIDD/Measurement/PixelFootprint/Row
     */
    virtual void setNumRows(unsigned long numRows)
    {
        measurement->pixelFootprint.row = numRows;
    }

    /*!
     *  Maps to:
     *  /SIDD/Measurement/PixelFootprint/Col
     */
    virtual unsigned long getNumCols() const
    {
        return measurement->pixelFootprint.col;
    }

    /*!
     *  Maps to:
     *  /SIDD/Measurement/PixelFootprint/Col
     */
    virtual void setNumCols(unsigned long numCols)
    {
        measurement->pixelFootprint.col = numCols;
    }

    /*!
     *  Maps to:
     *  /SIDD/Display/PixelType
     */
    virtual PixelType getPixelType() const
    {
        return display->pixelType;
    }

    /*!
     *  Maps to:
     *  /SIDD/Display/PixelType
     */
    virtual void setPixelType(PixelType pixelType)
    {
        display->pixelType = pixelType;
    }

    /*!
     *  Maps to:
     *  /SIDD/GeographicAndTarget/GeographicCoverage/Footprint
     */
    virtual std::vector<LatLon> getImageCorners() const
    {
        return geographicAndTarget->geographicCoverage->footprint;
    }

    /*!
     *  Maps to:
     *  /SIDD/GeographicAndTarget/GeographicCoverage/Footprint
     */
    virtual void setImageCorners(const std::vector<LatLon>& imageCorners)
    {
        geographicAndTarget->geographicCoverage->footprint = imageCorners;
    }

    /*!
     *  Maps to:
     *  /SIDD/ProductCreation/ProductName
     */
    virtual std::string getName() const
    {
        return productCreation->productName;
    }

    /*!
     *  Maps to:
     *  /SIDD/ProductCreation/ProductName
     */
    virtual void setName(std::string name)
    {
        productCreation->productName = name;
    }

    /*!
     *  Maps to:
     *  /SIDD/AdvancedExploitation/Collection/Information/SensorName
     */
    virtual std::string getSource() const
    {
        // TODO throw exception instead of returning empty string?
        return (exploitationFeatures
                && !exploitationFeatures->collections.empty() ? exploitationFeatures->collections[0]->information->sensorName
                                                              : std::string(""));
    }

    /*!
     *  Maps to:
     *  /SIDD/AdvancedExploitation/Collection/Information/SensorName
     */
    virtual void setSource(std::string name)
    {
        // TODO throw exception if cannot set?
        if (exploitationFeatures && !exploitationFeatures->collections.empty())
        {
            exploitationFeatures->collections[0]->information->sensorName
                    = name;
        }
    }

    /*!
     *  Maps to:
     *  /SIDD/ProductCreation/ProcessorInformation/ProcessingDateTime  
     */
    virtual DateTime getCreationTime() const
    {
        return productCreation->processorInformation->processingDateTime;
    }

    /*!
     *  Maps to:
     *  /SIDD/ProductCreation/ProcessorInformation/ProcessingDateTime  
     */
    virtual void setCreationTime(DateTime creationTime)
    {
        productCreation->processorInformation->processingDateTime
                = creationTime;
    }
    
    /*!
     *  Maps to:
     *  /SIDD/ProductCreation/Classification
     */
    virtual Classification& getClassification()
    {
        return productCreation->classification;
    }

    /*!
     *  Maps to:
     *  /SIDD/ProductCreation/Classification
     */
    virtual Classification getClassification() const
    {
        return productCreation->classification;
    }

    /*!
     *  Maps to:
     *  /SIDD/ProductCreation/Classification
     */
    virtual void setClassification(Classification classification)
    {
        productCreation->classification = classification;
    }

    virtual LUT* getDisplayLUT()
    {
        return display->remapInformation->remapLUT;
    }

    virtual std::string getVendorID() const
    {
        return std::string(VENDOR_ID);
    }

    virtual std::string getVersion() const
    {
        return mVersion.toString();
    }

protected:
    friend class DerivedXMLControl;
    friend class DerivedDataBuilder;
    Version mVersion;

private:
    static const char VENDOR_ID[];
};
}
}
#endif
