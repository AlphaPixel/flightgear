// CoordinateSystems.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <cmath>

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
