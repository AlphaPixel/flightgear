// CoordinateSystem.cxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "CoordinateSystems.hxx"
#include "Frame.hxx"
#include "UnitTypes.hxx"
#include "Math.hxx"

#include <simgear/math/sg_geodesy.hxx>

#ifndef NDEBUG
#include <assert.h>
namespace
{
    class UnitTest
    {
    public:
        UnitTest()
        {
            const double adelaide_latitude = -34.9;
            const double adelaide_longitude = 138.5;

            LLA adelaide_lla(Angle::fromDegrees(adelaide_latitude), Angle::fromDegrees(adelaide_longitude), Distance::fromMeters(10000));
            ECEF adelaide_ecef(adelaide_lla);

            // From http://www.sysense.com/products/ecef_lla_converter/index.html
            assert(is_close_hundredth(adelaide_ecef.GetX().inMeters(), -3928260.52));
            assert(is_close_hundredth(adelaide_ecef.GetY().inMeters(),  3475431.33));
            assert(is_close_hundredth(adelaide_ecef.GetZ().inMeters(), -3634495.17));

            LLA adelaide_lla_check(adelaide_ecef);
            assert(is_close_tenth(adelaide_lla_check.GetLatitude().inDegrees(), -34.9));
            assert(is_close_tenth(adelaide_lla_check.GetLongitude().inDegrees(), 138.5));
            assert(is_close_tenth(adelaide_lla_check.GetAltitude().inMeters(), 10000.0));
        }
    };

    static UnitTest unitTest;
}
#endif

ECEF::ECEF(const LLA &lla)
{
    double xyz[3];
    sgGeodToCart(lla.GetLatitude().inRadians(), lla.GetLongitude().inRadians(), lla.GetAltitude().inMeters(), xyz);
    _value = ECEF::coordinate_type(Distance::fromMeters(xyz[0]), Distance::fromMeters(xyz[1]), Distance::fromMeters(xyz[2]));
}

LLA::LLA(const ECEF &ecef)
{
    const double xyz[3] = {ecef.GetX().inMeters(), ecef.GetY().inMeters(), ecef.GetZ().inMeters()};
    double latitude_in_radians, longitude_in_radians, altitude_in_meters;
    sgCartToGeod(xyz, &latitude_in_radians, &longitude_in_radians, &altitude_in_meters);

    _value = LLA::value_type(Angle::fromRadians(latitude_in_radians), Angle::fromRadians(longitude_in_radians), Distance::fromMeters(altitude_in_meters));
}

void CalculateNED(double latitude, double longitude, SGVec3d &N, SGVec3d &E, SGVec3d &D)
{
    // Start with NED axes on the equator and prime meridian in the ECEF frame.
    // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf - figure 4.1)
    static const SGVec3d n0(0, 0, 1);
    static const SGVec3d e0(0, 1, 0);
    static const SGVec3d d0(-1, 0, 0);

//    const auto a = SGQuatd::fromAngleAxis(longitude, n0);
    const auto a = SGQuatd::fromAngleAxis(longitude, -n0);
    const auto n1 = a.transform(n0);
    const auto e1 = a.transform(e0);
    const auto d1 = a.transform(d0);

//    const auto b = SGQuatd::fromAngleAxis(latitude, -e1);
    const auto b = SGQuatd::fromAngleAxis(latitude, e1);
    N = b.transform(n1);
    E = b.transform(e1);
    D = b.transform(d1);
}

void RotateFrame(const DIS::Orientation &rot, 
                        const SGVec3d &x0, 
                        const SGVec3d &y0, 
                        const SGVec3d &z0,
                        SGVec3d &x3,
                        SGVec3d &y3,
                        SGVec3d &z3)
{
#if 1
    SGVec3d X(1, 0, 0);
    SGVec3d Y(0, 1, 0);
    SGVec3d Z(0, 0, 1);

    const auto c = SGQuatd::fromAngleAxis(-rot.getPsi(), Z);

    // X = c.transform(X);
    // Y = c.transform(Y);
    // Z = c.transform(Z);

    const auto _x1 = c.transform(x0);
    const auto _y1 = c.transform(y0);
    const auto _z1 = c.transform(z0);    

    const auto d = SGQuatd::fromAngleAxis(rot.getTheta(), Y);

    // X = d.transform(X);
    // Y = d.transform(Y);
    // Z = d.transform(Z);

    const auto _x2 = d.transform(_x1);
    const auto _y2 = d.transform(_y1);
    const auto _z2 = d.transform(_z1);    

    const auto e = SGQuatd::fromAngleAxis(-rot.getPhi(), X);

    // X = d.transform(X);
    // Y = d.transform(Y);
    // Z = d.transform(Z);

    x3 = e.transform(_x2);
    y3 = e.transform(_y2);
    z3 = e.transform(_z2);    
#else
    const auto c = SGQuatd::fromEulerRad(rot.getPsi(), rot.getTheta(), rot.getPhi());
    x3 = c.transform(x0);
    y3 = c.transform(y0);
    z3 = c.transform(z0);    
#endif    
}                        

void CalculateOrientedECEFAxes(const DIS::Orientation &ecef, SGVec3d &x3, SGVec3d &y3, SGVec3d &z3)
{
    // Convert a ECEF euler angle representation to local heading/pitch/roll.
    static const SGVec3d x0(1, 0, 0);  // ECEF X axis
    static const SGVec3d y0(0, 1, 0);  // ECEF Y axis
    static const SGVec3d z0(0, 0, 1);  // ECEF Z axis

    RotateFrame(ecef, x0, y0, z0, x3, y3, z3);
}

void CalculateEulerAnglesBetweenFrames(
    const SGVec3d &x0, 
    const SGVec3d &y0, 
    const SGVec3d &z0,
    const SGVec3d &x3,
    const SGVec3d &y3,
    const SGVec3d &z3,
    double &psi,
    double &theta,
    double &phi)
{
    // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf - figure 4.19)

    // psi = atan2( dot(x3,y0), dot(x3,x0) );
    psi = std::atan2(dot(x3,y0), dot(x3,x0));

    // theta = atan2( -dot(x3,z0), sqrt( (dot(x3,x0))^2 + (dot(x3,y0))^2) );    
    const double x3_dot_x0 = dot(x3, x0);
    const double x3_dot_x0_squared = x3_dot_x0 * x3_dot_x0;

    const double x3_dot_y0 = dot(x3, y0);
    const double x3_dot_y0_squared = x3_dot_y0 * x3_dot_y0;
    theta = std::atan2(-dot(x3,z0), std::sqrt(x3_dot_x0_squared + x3_dot_y0_squared));

    // phi = atan2(dot(y3,z2), dot(y3,y2));
    // const SGQuatd second = SGQuatd::fromEulerRad(ecef.getPsi(), ecef.getTheta(), 0.0);
    // const auto y2 = second.transform(y0);
    // const auto z2 = second.transform(z0);
    //phi = std::atan2(dot(y3, z2), dot(y3, y2));
    phi = 0;
}

void ECEFtoLLA(const DIS::Vector3Double &ecef, 
                      double &latitude, 
                      double &longitude, 
                      double &altitude_in_meters)
{
    const double xyz[3] = {ecef.getX(), ecef.getY(), ecef.getZ()};
    sgCartToGeod(xyz, &latitude, &longitude, &altitude_in_meters);
}

void ECEFtoHPR(const DIS::Orientation &ecef, 
                      double latitude, 
                      double longitude, 
                      double &heading, 
                      double &pitch, 
                      double &roll)
{
    // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf - figure 4.3)


    // Calculate NED (in ECEF space) using latitude and longitude
    SGVec3d N, E, D;
    CalculateNED(latitude, longitude, N, E, D);

    // Rotate the NED axes by the desired euler angles
    SGVec3d Nr, Er, Dr;
    RotateFrame(ecef, N, E, D, Nr, Er, Dr);

    // Calculate ECEF axes oriented by the DIS orientation
//    SGVec3d x, y, z;
//    CalculateOrientedECEFAxes(ecef, x, y, z); // BUSTED

    double psi, theta, phi;
    CalculateEulerAnglesBetweenFrames(E, D, N, Er, Dr, Nr, psi, theta, phi);

    heading = psi;
    pitch = 0;
    roll = 0;   //phi;
}
