// Enumerations.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

// Taken from SISO-STD-010.xml - see https://github.com/open-dis/dis-enumerations
enum class PDUType : std::uint8_t
{
    ENTITY_STATE = 1,
    FIRE = 2,
    DETONATION = 3,
    COLLISION = 4,
};

enum class EntityKind : std::uint8_t
{
    OTHER = 0,
    PLATFORM= 1,
    MUNITION = 2,
    LIFE_FORM = 3,
    ENVIRONMENTAL = 4,
    CULTURAL_FEATURE = 5,
    SUPPLY = 6,
    RADIO = 7,
    EXPENDABLE = 8,
    SENSOR_EMITTER = 9,
};

enum class CountryCode : std::uint16_t 
{
    PEOPLES_REPUBLIC_OF_CHINA = 45,
    UNITED_STATES = 225,
    RUSSIA = 260,
};

enum class Domain : std::uint8_t
{
    OTHER = 0,
    LAND = 1,
    AIR = 2,
    SURFACE = 3,
    SUBSURFACE = 4,
    SPACE = 5,
};

enum class Category : std::uint8_t
{
    TANK = 1,
    UTILITY_HELICOPTOR = 21,
};

enum class SubCategory_Tank : std::uint8_t
{
    T80_MBT = 1,
    T72_MBT = 2,
};

enum class SubCategory_UtilityHelicopter : std::uint8_t
{
    SIKORSKY_S70A
};

enum class Specific_T72 : std::uint8_t
{
    T72 = 1,
    T72M = 2,
    T72K = 3,
    T72A = 4,
    T72AK = 5,
    T72B = 6,
    T72BK = 7,
    T72B1 = 8,
    T72B1K = 9,
};

enum class Specific_SIKORSKY_S70A : std::uint8_t
{
    UH_60A_BLACKHAWK = 1,
    UH_60L_BLACKHAWK = 2,
    EH_60C = 3,
    VH_60N = 5,
    UH_60P = 6,
};
