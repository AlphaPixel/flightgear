// CoordinateSystems.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <cmath>
#include "Frame.hxx"

inline double nearest_tenth(double a)
{
    const double a_mul = a * 10.0;
    const double a_round = std::round(a_mul);
    return a_round / 10.0;
}

inline double nearest_hundredth(double a)
{
    const double a_mul = a * 100.0;
    const double a_round = std::round(a_mul);
    return a_round / 100.0;
}

inline double nearest_thousandth(double a)
{
    const double a_mul = a * 1000.0;
    const double a_round = std::round(a_mul);
    return a_round / 1000.0;
}

inline bool is_close_tenth(double a, double b)
{
    return nearest_tenth(a) == nearest_tenth(b);
}

inline bool is_close_hundredth(double a, double b)
{
    return nearest_hundredth(a) == nearest_hundredth(b);
}

inline bool is_close_thousandth(double a, double b)
{
    return nearest_thousandth(a) == nearest_thousandth(b);
}

inline bool is_close_thousandth(const SGVec3d &a, const SGVec3d &b)
{
    return (is_close_thousandth(a.x(), b.x()) &&
            is_close_thousandth(a.y(), b.y()) &&
            is_close_thousandth(a.z(), b.z()));            
}

inline bool is_close_thousandth(const Frame &a, const Frame &b)
{
    return (is_close_thousandth(a.GetXAxis(), b.GetXAxis()) &&
            is_close_thousandth(a.GetYAxis(), b.GetYAxis()) &&
            is_close_thousandth(a.GetZAxis(), b.GetZAxis()));            
}