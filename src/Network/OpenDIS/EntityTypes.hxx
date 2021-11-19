// EntityTypes.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <dis6/EntityType.h>
#include "Enumerations.hxx"

class AH64ApacheHelicopter : public DIS::EntityType
{
public:
    AH64ApacheHelicopter(Specific_AH64_APACHE specific)
    {
        setCategory(static_cast<unsigned char>(Category::ATTACK_HELICOPTER));
        setCountry(static_cast<unsigned short>(CountryCode::UNITED_STATES));
        setDomain(static_cast<unsigned char>(Domain::AIR));
        setEntityKind(static_cast<unsigned char>(EntityKind::PLATFORM));
        setExtra(0);
        setSpecific(static_cast<unsigned char>(specific));
        setSubcategory(static_cast<unsigned char>(SubCategory_US_AttackHelicopter::AH64_APACHE));
    }

    static bool matches(const DIS::EntityType &check)
    {
        if (check.getCategory() == static_cast<unsigned char>(Category::ATTACK_HELICOPTER) &&
            check.getCountry() == static_cast<unsigned char>(CountryCode::UNITED_STATES) &&
            check.getDomain() == static_cast<unsigned char>(Domain::AIR) &&
            check.getEntityKind() == static_cast<unsigned char>(EntityKind::PLATFORM) &&
            check.getSubcategory() == static_cast<unsigned char>(SubCategory_US_AttackHelicopter::AH64_APACHE))
        {
            return true;
        }

        return false;
    }
};

class SikorskyS70AHelicopter : public DIS::EntityType
{
public:
    SikorskyS70AHelicopter(Specific_SIKORSKY_S70A specific)
    {
        setCategory(static_cast<unsigned char>(Category::UTILITY_HELICOPTER));
        setCountry(static_cast<unsigned short>(CountryCode::UNITED_STATES));
        setDomain(static_cast<unsigned char>(Domain::AIR));
        setEntityKind(static_cast<unsigned char>(EntityKind::PLATFORM));
        setExtra(0);
        setSpecific(static_cast<unsigned char>(specific));
        setSubcategory(static_cast<unsigned char>(SubCategory_US_UtilityHelicopter::SIKORSKY_S70A));
    }

    static bool matches(const DIS::EntityType &check)
    {
        if (check.getCategory() == static_cast<unsigned char>(Category::UTILITY_HELICOPTER) &&
            check.getCountry() == static_cast<unsigned char>(CountryCode::UNITED_STATES) &&
            check.getDomain() == static_cast<unsigned char>(Domain::AIR) &&
            check.getEntityKind() == static_cast<unsigned char>(EntityKind::PLATFORM) &&
            check.getSubcategory() == static_cast<unsigned char>(SubCategory_US_UtilityHelicopter::SIKORSKY_S70A))
        {
            return true;
        }

        return false;
    }
};

class M1AbramsTank : public DIS::EntityType
{
public:
    M1AbramsTank(Specific_US_M1_ABRAMS specific)
    {
        setCategory(static_cast<unsigned char>(Category::TANK));
        setCountry(static_cast<unsigned short>(CountryCode::UNITED_STATES));
        setDomain(static_cast<unsigned char>(Domain::LAND));
        setEntityKind(static_cast<unsigned char>(EntityKind::PLATFORM));
        setExtra(0);
        setSpecific(static_cast<unsigned char>(specific));
        setSubcategory(static_cast<unsigned char>(SubCategory_US_Tank::M1_ABRAMS));
    }

    static bool matches(const DIS::EntityType &check)
    {
        if (check.getCategory() == static_cast<unsigned char>(Category::TANK) &&
            check.getCountry() == static_cast<unsigned char>(CountryCode::UNITED_STATES) &&
            check.getDomain() == static_cast<unsigned char>(Domain::LAND) &&
            check.getEntityKind() == static_cast<unsigned char>(EntityKind::PLATFORM) &&
            check.getSubcategory() == static_cast<unsigned char>(SubCategory_US_Tank::M1_ABRAMS))
        {
            return true;
        }

        return false;
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

