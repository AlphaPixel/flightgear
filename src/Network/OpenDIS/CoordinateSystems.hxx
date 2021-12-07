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

void CalculateNED(double latitudeRadians, double longitudeRadians, SGVec3d &N, SGVec3d &E, SGVec3d &D);
void RotateFrame(const DIS::Orientation &rot, const SGVec3d &x0, const SGVec3d &y0, const SGVec3d &z0, SGVec3d &x3, SGVec3d &y3, SGVec3d &z3);
void CalculateOrientedECEFAxes(const DIS::Orientation &ecef, SGVec3d &x3, SGVec3d &y3, SGVec3d &z3);
void CalculateEulerAnglesBetweenFrames(const SGVec3d &x0, const SGVec3d &y0, const SGVec3d &z0, const SGVec3d &x3, const SGVec3d &y3, const SGVec3d &z3, double &psi, double &theta, double &phi);
void ECEFtoLLA(const DIS::Vector3Double &ecef, double &latitude, double &longitude, double &altitude_in_meters);
void ECEFtoHPR(const DIS::Orientation &ecef, double latitude, double longitude, double &heading, double &pitch, double &roll);

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
