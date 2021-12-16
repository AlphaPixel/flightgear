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
    PLATFORM = 1,
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
    CIS = 222,  // Commonwealth Of Indenpendent States
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
    ATTACK_HELICOPTER = 20,
    UTILITY_HELICOPTER = 21,
};

//
// US (225)
//

// US Tank
enum class SubCategory_US_Tank : std::uint8_t
{
    M1_ABRAMS = 1,
};

enum class Specific_US_M1_ABRAMS : std::uint8_t
{
    M1 = 1,
    M1A1 = 2,
    M1A2 = 3,
    M1A1_MINE_ROLLERS = 4,
    M1A1_MINE_PLOWS = 5,
    M1A1_DU_ARMOR = 6,
    M1A2_CVCC = 7,
    M1A2_SEP = 8,
    M1A1D = 9,
};

// US Attack Helicopter
enum class SubCategory_US_AttackHelicopter : std::uint8_t
{
    AH64_APACHE = 1,
};

enum class Specific_AH64_APACHE : std::uint8_t
{
    AH64A = 1,
    AH64B = 2,
    AH64C = 3,
    AH64D_LONGBOW = 4,
    PETAN = 5,
    AH64D = 6
};

// US Utility Helicopter
enum class SubCategory_US_UtilityHelicopter : std::uint8_t
{
    SIKORSKY_S70A
};

enum class Specific_SIKORSKY_S70A : std::uint8_t
{
    UH60A_BLACKHAWK = 1,
    UH60L_BLACKHAWK = 2,
    EH60C = 3,
    VH60N = 5,
    UH60P = 6,
};

//
// Commonwealth Of Independent States (CIS)
//
enum class SubCategory_CIS_Tank : std::uint8_t
{
    T80_MBT = 1,
    T72_MBT = 2,
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
