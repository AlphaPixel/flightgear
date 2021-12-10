// CoordinateSystems.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include "UnitTypes.hxx"
#include <simgear/math/SGVec3.hxx>
#include <dis6/Orientation.h>

class Frame
{
public:
    static Frame fromECEFBase()
    {
        return Frame(SGVec3d(1, 0, 0), SGVec3d(0, 1, 0), SGVec3d(0, 0, 1));
    }

    static Frame fromNEDBase()
    {
        return Frame(SGVec3d(0, 0, 1), SGVec3d(0, 1, 0), SGVec3d(-1, 0, 0));
    }

    // Create a local NED frame at the given latitude/longitude
    static Frame fromLatLon(Angle latitude, Angle longitude);

    const SGVec3d &GetXAxis() const
    {
        return _x;
    }

    const SGVec3d &GetYAxis() const
    {
        return _y;
    }

    const SGVec3d &GetZAxis() const
    {
        return _z;
    }

    void rotate(const Angle &a, const SGVec3d &axis);
    void rotate(const DIS::Orientation &orientation);
    void rotate(const SGQuatd &q);

    static SGQuatd GetRotateTo(const Frame &from, const Frame &to);
    static DIS::Orientation GetEulerAngles(const Frame &from, const Frame &to);
    
private:
    Frame(SGVec3d x, SGVec3d y, SGVec3d z)
        : _x(x)
        , _y(y)
        , _z(z)
    {
    }

    SGVec3d _x, _y, _z;
};
