// EntityTypes.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <dis6/EntityType.h>
#include "Enumerations.hxx"

class SikorskyS70AHelicopter : public DIS::EntityType
{
public:
    SikorskyS70AHelicopter(Specific_SIKORSKY_S70A specific)
    {
        setCategory(static_cast<unsigned char>(Category::UTILITY_HELICOPTOR));
        setCountry(static_cast<unsigned short>(CountryCode::UNITED_STATES));
        setDomain(static_cast<unsigned char>(Domain::AIR));
        setEntityKind(static_cast<unsigned char>(EntityKind::PLATFORM));
        setExtra(0);
        setSpecific(static_cast<unsigned char>(specific));
        setSubcategory(static_cast<unsigned char>(SubCategory_US_UtilityHelicopter::SIKORSKY_S70A));
    }
};

class T72Tank : public DIS::EntityType
{
public:
    T72Tank(Specific_T72 specific)
    {
        setCategory(static_cast<unsigned char>(Category::TANK));
        setCountry(static_cast<unsigned short>(CountryCode::CIS));
        setDomain(static_cast<unsigned char>(Domain::LAND));
        setEntityKind(static_cast<unsigned char>(EntityKind::PLATFORM));
        setExtra(0);
        setSpecific(static_cast<unsigned char>(specific));
        setSubcategory(static_cast<unsigned char>(SubCategory_CIS_Tank::T72_MBT));
    }

    static bool matches(const DIS::EntityType &check)
    {
        if (check.getCategory() == static_cast<unsigned char>(Category::TANK) &&
            check.getCountry() == static_cast<unsigned char>(CountryCode::CIS) &&
            check.getDomain() == static_cast<unsigned char>(Domain::LAND) &&
            check.getEntityKind() == static_cast<unsigned char>(EntityKind::PLATFORM) &&
            check.getSubcategory() == static_cast<unsigned char>(SubCategory_CIS_Tank::T72_MBT))
        {
            return true;
        }

        return false;
    }
};
