// CoordinateSystems.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once
#include <cmath>
#include <simgear/math/SGVec3.hxx>
#include <simgear/math/SGQuat.hxx>
#include <simgear/constants.h>

#include <dis6/Orientation.h>
#include <dis6/Vector3Double.h>

#include <osg/Math> // DegreesToRadians/RadiansToDegrees

#include "UnitTypes.hxx"

class LLA;

class ECEF 
{
public:
    ECEF(Distance x, Distance y, Distance z) 
        : _value(x, y, z)
    {
    }

    ECEF(const DIS::Vector3Double xyz)
        : _value(xyz.getX(), xyz.getY(), xyz.getZ())
    {
    }

    ECEF(const LLA &lla);

    typedef SGVec3<Distance> coordinate_type;

    const Distance &GetX() const
    {
        return _value.x();
    }

    const Distance &GetY() const
    {
        return _value.y();
    }

    const Distance &GetZ() const
    {
        return _value.z();
    }

private:
    coordinate_type _value;
};

class LLA
{
public:
    LLA(Angle latitude, Angle longitude, Distance altitude) 
        : _value(latitude, longitude, altitude)
    {
    }

    LLA(const ECEF &ecef);

    struct value_type
    {
        Angle _latitude;
        Angle _longitude;
        Distance _altitude;

        value_type() {}

        value_type(Angle latitude, Angle longitude, Distance altitude)
            : _latitude(latitude)
            , _longitude(longitude)
            , _altitude(altitude)
        {
        }
    };

    typedef value_type coordinate_type;

    Angle GetLatitude() const
    {
        return _value._latitude;
    }

    Angle GetLongitude() const
    {
        return _value._longitude;
    }

    Distance GetAltitude() const
    {
        return _value._altitude;
    }

    void SetAltitude(Distance altitude)
    {
        _value._altitude = altitude;
    }

private:
    value_type _value;
};
