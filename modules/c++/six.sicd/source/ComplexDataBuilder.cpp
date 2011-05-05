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
#include "six/sicd/ComplexDataBuilder.h"

using namespace six;
using namespace six::sicd;

ComplexDataBuilder::ComplexDataBuilder(six::sicd::Version version) :
    mData(new ComplexData()), mAdopt(true)
{
    mData->setVersion(version);
}

ComplexDataBuilder::ComplexDataBuilder(ComplexData *data) :
    mData(data), mAdopt(false)
{
}

ComplexDataBuilder::~ComplexDataBuilder()
{
    if (mData && mAdopt)
        delete mData;
}

ComplexDataBuilder& ComplexDataBuilder::addImageCreation()
{
    if (mData->imageCreation)
        delete mData->imageCreation;
    mData->imageCreation = new ImageCreation();
    return *this;
}

ComplexDataBuilder& ComplexDataBuilder::addRadiometric()
{
    if (mData->radiometric)
        delete mData->radiometric;
    mData->radiometric = new Radiometric();
    return *this;
}

ComplexDataBuilder& ComplexDataBuilder::addAntenna()
{
    if (mData->antenna)
        delete mData->antenna;
    mData->antenna = new Antenna();
    return *this;
}

ComplexDataBuilder& ComplexDataBuilder::addErrorStatistics()
{
    if (mData->errorStatistics)
        delete mData->errorStatistics;
    mData->errorStatistics = new ErrorStatistics();
    return *this;
}

ComplexDataBuilder& ComplexDataBuilder::addMatchInformation()
{
    if (mData->matchInformation)
        delete mData->matchInformation;
    mData->matchInformation = new MatchInformation();
    return *this;
}

ComplexData* ComplexDataBuilder::get()
{
    return mData;
}

ComplexData* ComplexDataBuilder::steal()
{
    mAdopt = false;
    return get();
}
