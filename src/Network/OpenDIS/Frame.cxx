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
            testECEFtoNEDRotations();
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

        void testECEFtoNEDRotations()
        {
            {
                // A null rotation in ECEF space should be a null rotation in NED space.
                DIS::Orientation ecef;
                ecef.setPsi(Angle::fromDegrees(0.0).inRadians());
                ecef.setTheta(Angle::fromDegrees(0).inRadians());
                ecef.setPhi(Angle::fromDegrees(0).inRadians());

                DIS::Orientation ned;
                ned.setPsi(Angle::fromDegrees(0).inRadians());
                ned.setTheta(Angle::fromDegrees(0).inRadians());
                ned.setPhi(Angle::fromDegrees(0).inRadians());

                verifyFrameRotation(ecef, ned);
            }

            {
                // A change in Psi() in ECEF space at the base frame is a bank change (in NED space)
                // of 90 degrees.
                DIS::Orientation ecef;
                ecef.setPsi(Angle::fromDegrees(90).inRadians());
                ecef.setTheta(Angle::fromDegrees(0).inRadians());
                ecef.setPhi(Angle::fromDegrees(0).inRadians());

                DIS::Orientation ned;
                ned.setPsi(Angle::fromDegrees(0).inRadians());
                ned.setTheta(Angle::fromDegrees(0).inRadians());
                ned.setPhi(Angle::fromDegrees(90).inRadians());

                verifyFrameRotation(ecef, ned);
            }

            {
                // A change in Theta() in ECEF space at the base frame is a pitch change (in NED space)
                // of 90 degrees.
                DIS::Orientation ecef;
                ecef.setPsi(Angle::fromDegrees(0).inRadians());
                ecef.setTheta(Angle::fromDegrees(90).inRadians());
                ecef.setPhi(Angle::fromDegrees(0).inRadians());

                DIS::Orientation ned;
                ned.setPsi(Angle::fromDegrees(0).inRadians());
                ned.setTheta(Angle::fromDegrees(90).inRadians());
                ned.setPhi(Angle::fromDegrees(0).inRadians());

                verifyFrameRotation(ecef, ned);
            }

            {
                // A change in Phi() in ECEF space at the base frame is a heading change (in NED space)
                // of -90 degrees.
                DIS::Orientation ecef;
                ecef.setPsi(Angle::fromDegrees(0).inRadians());
                ecef.setTheta(Angle::fromDegrees(0).inRadians());
                ecef.setPhi(Angle::fromDegrees(90).inRadians());

                DIS::Orientation ned;
                ned.setPsi(Angle::fromDegrees(-90).inRadians());
                ned.setTheta(Angle::fromDegrees(0).inRadians());
                ned.setPhi(Angle::fromDegrees(0).inRadians());

                verifyFrameRotation(ecef, ned);
            }
        }

        // Verifies that an ECEF orientation matches the corresponding NED orientation.
        void verifyFrameRotation(const DIS::Orientation &ecef, const DIS::Orientation &ned)
        {
            // Start with a known frame at the prime meridian and equator
            const Frame base = Frame::fromLatLon(Angle::fromRadians(0), Angle::fromRadians(0));

            auto check = base;
            check.rotate(ecef);    // ECEF space rotation

            auto q = Frame::GetRotateTo(base, check);

            // Verify the rotation actually rotates 'base' to 'check' (by rerotating a new frame named c2)
            auto c2 = base;
            c2.rotate(q);       // ECEF space rotation
            assert(is_close_thousandth(check, c2));

            // Create a quarternion (in NED space) that matches the expected resulting Euler angles from 'base' to 'check'.  Rotate
            // a new copy of 'base' by that much and compare the resulting frame with 'check'.
            auto c3 = base;
            c3.rotate(SGQuatd::fromAngleAxis(Angle::fromRadians(ned.getPsi()).inRadians(), -c3.GetZAxis()));
            c3.rotate(SGQuatd::fromAngleAxis(Angle::fromRadians(ned.getTheta()).inRadians(), -c3.GetYAxis()));
            c3.rotate(SGQuatd::fromAngleAxis(Angle::fromRadians(ned.getPhi()).inRadians(), -c3.GetXAxis()));

            assert(is_close_thousandth(check, c3));
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
    rotate(c);
}

void Frame::rotate(Frame rotationFrame, const DIS::Orientation &orientation, Frame *intermediate)
{
    rotate(Angle::fromRadians(orientation.getPsi()), -rotationFrame.GetZAxis());
    rotationFrame.rotate(Angle::fromRadians(orientation.getPsi()), -rotationFrame.GetZAxis());

    rotate(Angle::fromRadians(orientation.getTheta()), -rotationFrame.GetYAxis());
    rotationFrame.rotate(Angle::fromRadians(orientation.getTheta()), -rotationFrame.GetYAxis());

    if (intermediate)
    {
        *intermediate = rotationFrame;
    }

    rotate(Angle::fromRadians(orientation.getPhi()), -rotationFrame.GetXAxis());
}

void Frame::rotate(const DIS::Orientation &orientation, Frame *intermediate)
{
    rotate(Frame::fromECEFBase(), orientation, intermediate);
}

void Frame::rotate(const SGQuatd &q)
{
    _x = q.transform(_x);
    _y = q.transform(_y);
    _z = q.transform(_z);
}

SGQuatd Frame::GetRotateTo(const Frame &from, const Frame &to)
{
    static const auto unit = SGQuatd::unit();
    auto r = SGQuatd::fromRotateTo(from.GetXAxis(), to.GetXAxis());
    // If a unit quaternion was returned, the X axes of both frames was the same.  We need
    // to check another axis.   If *that* axis is also coincident, then the frames are the same
    // and we can keep going.
    if (r == unit)
    {
        r = SGQuatd::fromRotateTo(from.GetYAxis(), to.GetYAxis());
    }

    return r;
}

DIS::Orientation Frame::GetEulerAngles(const Frame &from, const Frame &intermediate, const Frame &to)
{
    // (https://apps.dtic.mil/sti/pdfs/ADA484864.pdf - figure 4.20, 4.21, 4.22, 4.23)
    DIS::Orientation o;
    o.setPsi(
        std::atan2(
            dot(to.GetXAxis(), from.GetYAxis()), 
            dot(to.GetXAxis(), from.GetXAxis())
        )
    ); 

    const auto dot_x3_z0 = dot(to.GetXAxis(), from.GetZAxis());
    const auto dot_x3_x0 = dot(to.GetXAxis(), from.GetXAxis());
    const auto dot_x3_y0 = dot(to.GetXAxis(), from.GetYAxis());
    o.setTheta(
        std::atan2(
            -dot_x3_z0, 
            std::sqrt(
                (dot_x3_x0 * dot_x3_x0) + (dot_x3_y0 * dot_x3_y0)
            )
        )
    );

    const auto dot_y3_z2 = dot(to.GetYAxis(), intermediate.GetZAxis());
    const auto dot_y3_y2 = dot(to.GetYAxis(), intermediate.GetYAxis());
    o.setPhi(std::atan2(dot_y3_z2, dot_y3_y2));
    return o;
}
