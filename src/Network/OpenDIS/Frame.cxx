// Frame.cxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "Frame.hxx"
#include "Math.hxx"
#include <simgear/math/SGQuat.hxx>

#ifndef NDEBUG
#include <assert.h>

namespace
{
    class UnitTest
    {
    public:
        // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf)
        const double adelaide_latitude_radians = osg::DegreesToRadians(-34.9);
        const double adelaide_longitude_radians = osg::DegreesToRadians(138.5);

        UnitTest()
        {
            testNED();
            testDISOrientation();
            testFrameDelta();
        }

        void testNED()
        {
            Frame adelaide = Frame::fromLatLon(Angle::fromRadians(adelaide_latitude_radians), Angle::fromRadians(adelaide_longitude_radians));

            // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf - figure 4.8)
            assert(is_close_thousandth(adelaide.GetXAxis().x(), -0.429));
            assert(is_close_thousandth(adelaide.GetXAxis().y(),  0.379));
            assert(is_close_thousandth(adelaide.GetXAxis().z(),  0.820));

            assert(is_close_thousandth(adelaide.GetYAxis().x(), -0.663));
            assert(is_close_thousandth(adelaide.GetYAxis().y(), -0.749));
            assert(is_close_thousandth(adelaide.GetYAxis().z(),  0.000));

            assert(is_close_thousandth(adelaide.GetZAxis().x(),  0.614));
            assert(is_close_thousandth(adelaide.GetZAxis().y(), -0.543));
            assert(is_close_thousandth(adelaide.GetZAxis().z(),  0.572));
        }

        void testDISOrientation()
        {
            // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf - figure 5 for help understanding the below orientation values)

            // Start with a known frame at the prime meridian and equator
            const Frame base = Frame::fromLatLon(Angle::fromRadians(0), Angle::fromRadians(0));

            // Create a simple orientations and verify their values.
            DIS::Orientation o;

            {
                o.setPsi(0);
                o.setTheta(0);
                o.setPhi(0);

                auto check = base;
                check.rotate(o);

                // Check N
                assert(is_close_thousandth(check.GetXAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().z(),  1.0));

                // Check E
                assert(is_close_thousandth(check.GetYAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetYAxis().y(),  1.0));
                assert(is_close_thousandth(check.GetYAxis().z(),  0.0));

                // Check D
                assert(is_close_thousandth(check.GetZAxis().x(),  -1.0));
                assert(is_close_thousandth(check.GetZAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().z(),  0.0));
            }


            {
                o.setPsi(Angle::fromDegrees(90.0).inRadians());
                o.setTheta(0);
                o.setPhi(0);

                auto check = base;
                check.rotate(o);

                // Check N
                assert(is_close_thousandth(check.GetXAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().z(),  1.0));

                // Check E
                assert(is_close_thousandth(check.GetYAxis().x(),  -1.0));
                assert(is_close_thousandth(check.GetYAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetYAxis().z(),  0.0));

                // Check D
                assert(is_close_thousandth(check.GetZAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().y(),  -1.0));
                assert(is_close_thousandth(check.GetZAxis().z(),  0.0));
            }

            {
                o.setPsi(0);
                o.setTheta(Angle::fromDegrees(90.0).inRadians());
                o.setPhi(0);

                auto check = base;
                check.rotate(o);

                // Check N
                assert(is_close_thousandth(check.GetXAxis().x(),  1.0));
                assert(is_close_thousandth(check.GetXAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().z(),  0.0));

                // Check E
                assert(is_close_thousandth(check.GetYAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetYAxis().y(),  1.0));
                assert(is_close_thousandth(check.GetYAxis().z(),  0.0));

                // Check D
                assert(is_close_thousandth(check.GetZAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().z(),  1.0));
            }

            {
                o.setPsi(0);
                o.setTheta(0);
                o.setPhi(Angle::fromDegrees(90.0).inRadians());

                auto check = base;
                check.rotate(o);

                // Check N
                assert(is_close_thousandth(check.GetXAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().y(), -1.0));
                assert(is_close_thousandth(check.GetXAxis().z(),  0.0));

                // Check E
                assert(is_close_thousandth(check.GetYAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetYAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetYAxis().z(),  1.0));

                // Check D
                assert(is_close_thousandth(check.GetZAxis().x(), -1.0));
                assert(is_close_thousandth(check.GetZAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().z(),  0.0));
            }

            {
                o.setPsi(Angle::fromDegrees(90.0).inRadians());
                o.setTheta(Angle::fromDegrees(90.0).inRadians());
                o.setPhi(0);

                // Note: the rotation for Theta above rotates around the rotated (Psi)
                //       Y axis which now points down the old negative X axis.

                auto check = base;
                check.rotate(o);

                // Check N
                assert(is_close_thousandth(check.GetXAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().y(),  1.0));
                assert(is_close_thousandth(check.GetXAxis().z(),  0.0));

                // Check E
                assert(is_close_thousandth(check.GetYAxis().x(), -1.0));
                assert(is_close_thousandth(check.GetYAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetYAxis().z(),  0.0));

                // Check D
                assert(is_close_thousandth(check.GetZAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().z(),  1.0));
            }

            {
                o.setPsi(Angle::fromDegrees(90.0).inRadians());
                o.setTheta(Angle::fromDegrees(90.0).inRadians());
                o.setPhi(Angle::fromDegrees(90.0).inRadians());

                auto check = base;
                check.rotate(o);

                // Check N
                assert(is_close_thousandth(check.GetXAxis().x(),  1.0));
                assert(is_close_thousandth(check.GetXAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetXAxis().z(),  0.0));

                // Check E
                assert(is_close_thousandth(check.GetYAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetYAxis().y(),  1.0));
                assert(is_close_thousandth(check.GetYAxis().z(),  0.0));

                // Check D
                assert(is_close_thousandth(check.GetZAxis().x(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().y(),  0.0));
                assert(is_close_thousandth(check.GetZAxis().z(),  1.0));
            }
        }

        void testFrameDelta()
        {
            // Start with a known frame at the prime meridian and equator
            const Frame base = Frame::fromLatLon(Angle::fromRadians(0), Angle::fromRadians(0));

            // Create a simple oriention with a single rotation in Psi() - around the north pole in ECEF
            DIS::Orientation o;
            
            {
                o.setPsi(0);
                o.setTheta(0);
                o.setPhi(0);

                auto check = base;
                check.rotate(o);

                auto eulerAngles = check - base;
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPsi()).inDegrees(), 0.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getTheta()).inDegrees(), 0.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPhi()).inDegrees(), 0.0));
            }
#if 0
            {
                // A change in Psi() in ECEF space at the base frame is a bank change (in NED space)
                // of 90 degrees.
                o.setPsi(Angle::fromDegrees(90.0).inRadians());
                o.setTheta(0);
                o.setPhi(0);

                auto check = base;
                check.rotate(o);

                auto eulerAngles = check - base;
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPsi()).inDegrees(), 0.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getTheta()).inDegrees(), 0.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPhi()).inDegrees(), 90.0));
            }
#endif
            {
                // A change in Theta() in ECEF space at the base frame is a pitch change (in NED space)
                // of 90 degrees.
                o.setPsi(0);
                o.setTheta(Angle::fromDegrees(90.0).inRadians());
                o.setPhi(0);

                auto check = base;
                check.rotate(o);

                auto eulerAngles = check - base;
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPsi()).inDegrees(), 0.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getTheta()).inDegrees(), 90.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPhi()).inDegrees(), 0.0));
            }

            {
                // A change in Phi() in ECEF space at the base frame is a heading change (in NED space)
                // of -90 degrees.
                o.setPsi(0);
                o.setTheta(0);
                o.setPhi(Angle::fromDegrees(90.0).inRadians());

                auto check = base;
                check.rotate(o);

                auto eulerAngles = check - base;
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPsi()).inDegrees(), -90.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getTheta()).inDegrees(), 0.0));
                assert(is_close_thousandth(Angle::fromRadians(eulerAngles.getPhi()).inDegrees(), 0.0));
            }
        }
    };

    static UnitTest unitTest;
}
#endif

Frame Frame::fromLatLon(Angle latitude, Angle longitude)
{
    auto frame = Frame::fromNEDBase();

    frame.rotate(longitude, -frame.GetXAxis());
    frame.rotate(latitude, frame.GetYAxis());

    return frame;
}

void Frame::rotate(const Angle &angle, const SGVec3d &axis)
{
    const auto c = SGQuatd::fromAngleAxis(angle.inRadians(), axis);

    _x = c.transform(_x);
    _y = c.transform(_y);
    _z = c.transform(_z);
}

void Frame::rotate(const DIS::Orientation &orientation)
{
    auto ecefFrame = Frame::fromECEFBase();

    rotate(Angle::fromRadians(orientation.getPsi()), -ecefFrame.GetZAxis());
    ecefFrame.rotate(Angle::fromRadians(orientation.getPsi()), -ecefFrame.GetZAxis());

    rotate(Angle::fromRadians(orientation.getTheta()), -ecefFrame.GetYAxis());
    ecefFrame.rotate(Angle::fromRadians(orientation.getTheta()), -ecefFrame.GetYAxis());

    rotate(Angle::fromRadians(orientation.getPhi()), -ecefFrame.GetXAxis());
}

DIS::Orientation operator - (const Frame &a, const Frame &b)
{
    // Returns Euler angles (in the form of a DIS::Orientation) that
    // goes from frame 'b' to frame 'a' (angles = a - b)

    // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf - figure 4.19)
    const Frame &_0 = b;
    const Frame &_3 = a;    
    const SGVec3d x0 = _0.GetXAxis();
    const SGVec3d y0 = _0.GetYAxis();
    const SGVec3d z0 = _0.GetZAxis();

    const SGVec3d x3 = _3.GetXAxis();
    const SGVec3d y3 = _3.GetYAxis();
    const SGVec3d z3 = _3.GetZAxis();

    auto result = DIS::Orientation();

    // psi = atan2( dot(x3,y0), dot(x3,x0) );
    result.setPsi(std::atan2(dot(x3,y0), dot(x3,x0)));

    // theta = atan2( -dot(x3,z0), sqrt( (dot(x3,x0))^2 + (dot(x3,y0))^2) );    
    const double x3_dot_x0 = dot(x3, x0);
    const double x3_dot_x0_squared = x3_dot_x0 * x3_dot_x0;

    const double x3_dot_y0 = dot(x3, y0);
    const double x3_dot_y0_squared = x3_dot_y0 * x3_dot_y0;
    result.setTheta(std::atan2(-dot(x3,z0), std::sqrt(x3_dot_x0_squared + x3_dot_y0_squared)));

    // phi = atan2(dot(y3,z2), dot(y3,y2));
    // const SGQuatd second = SGQuatd::fromEulerRad(ecef.getPsi(), ecef.getTheta(), 0.0);
    // const auto y2 = second.transform(y0);
    // const auto z2 = second.transform(z0);
    //phi = std::atan2(dot(y3, z2), dot(y3, y2));
    result.setPhi(0);

    return result; 
}
