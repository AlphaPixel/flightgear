// CoordinateSystems.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <simgear/constants.h>
#include <osg/Math>

class Angle
{
public:
    static Angle fromRadians(double radians)
    {
        return Angle(radians);
    }

    static Angle fromDegrees(double degrees)
    {
        return Angle(osg::DegreesToRadians(degrees));
    }

    Angle(double radians = 0.0) :
        _radians(radians)
    {
    }

    double inRadians() const
    {
        return _radians;
    }

    double inDegrees() const
    {
        return osg::RadiansToDegrees(_radians);
    }

private:    
    double _radians;
};

class Distance
{
public:
    static Distance fromMeters(double meters)
    {
        return Distance(meters);
    }

    static Distance fromFeet(double feet)
    {
        return Distance(feet / SG_METER_TO_FEET);
    }

    Distance(double meters = 0.0) :
        _meters(meters)
    {
    }

    double inMeters() const
    {
        return _meters;
    }

    double inFeet() const
    {
        return _meters * SG_METER_TO_FEET;
    }

private:
    double _meters;
};
