// Tank.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <osg/io_utils>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgSim/DOFTransform>
#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>
#include <osgViewer/Viewer>

class Tank
{
public:
    enum class Type
    {
        T72,
        M1
    };

    Tank(Type type, osg::Transform* turret, osg::Transform* gun):
        _type(type),
        _turret(dynamic_cast<osgSim::DOFTransform*>(turret)),
        _gun(dynamic_cast<osgSim::DOFTransform*>(gun))
    {
    }

    void beginArticulation()
    {
        _azimuth = 0;
        _elevation = 0;
    }

    void endArticulation()
    {
        if (_turret && _gun)
        {
            // Put azimuth and elevation into HPR
            // DIS spec says order is: azimuth -> elevation -> rotation (which we're not handling)
            // Those are standard Euler angle ordering.

            // T72 "heading" is X, "pitch" is Y.
            if (_type == Type::T72)
            {
                _turret->setCurrentHPR(osg::Vec3(_azimuth, 0.0, 0.0));
                _gun->setCurrentHPR(osg::Vec3(0.0, _elevation, 0.0));
            }

            // M1 "heading" is Z, "pitch" is X.
            // Both _turret and _gun must be set for heading/azimuth; pitch/elevation is only
            // applied to the _gun.
            else
            {
                _turret->setCurrentHPR(osg::Vec3(0.0, 0.0, _azimuth));
                _gun->setCurrentHPR(osg::Vec3(_elevation, 0.0, _azimuth));
            }
        }
    }

    void setAzimuth(double azimuth)
    {
        // Heading of the turret (in radians) relative to the local Z axis basis vector (points down)
        _azimuth = azimuth;
    }

    void setElevation(double elevation)
    {
        // Elevation of main gun (in radians) relative to the local Y axis basis vector (points right)
        _elevation = elevation;
    }

    const osgSim::DOFTransform* getTurret() const
    {
        return _turret.get();
    }

    const osgSim::DOFTransform* getGun() const
    {
        return _gun.get();
    }

protected:
    Type _type;

    osg::ref_ptr<osgSim::DOFTransform> _turret;
    osg::ref_ptr<osgSim::DOFTransform> _gun;

    // Temporary holding values during articulation.
    double _azimuth, _elevation;
};

class TankVisitor : public osg::NodeVisitor
{
public:
    TankVisitor(const std::string& turret, const std::string& gun):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _nameTurret(turret),
        _nameGun(gun)
    {
    }

    virtual void apply(osg::Transform& t) override
    {
        const auto transformName = t.getName();
        if (transformName == _nameTurret)
        {
            _turret = &t;
        }

        else if (transformName == _nameGun)
        {
            _gun = &t;
        }

        traverse(t);
    }

    std::unique_ptr<Tank> getTank(Tank::Type type)
    {
        if (_turret.get() && _gun.get())
        {
            return std::unique_ptr<Tank>(new Tank(type, _turret.get(), _gun.get()));
        }
        else
        {
            return std::unique_ptr<Tank>();
        }
    }

protected:
    std::string _nameTurret;
    std::string _nameGun;

    osg::ref_ptr<osg::Transform> _turret;
    osg::ref_ptr<osg::Transform> _gun;
};
