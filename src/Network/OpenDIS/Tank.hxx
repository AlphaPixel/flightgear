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
#include "UnitTypes.hxx"
#include <assert.h>

class Tank
{
public:
    enum class Type
    {
        UNKNOWN,
        T72,
        M1
    };

    Tank(Type type, osg::Transform* turret, osg::Transform* gun):
        _type(type),
        _turret(dynamic_cast<osgSim::DOFTransform*>(turret)),
        _gun(dynamic_cast<osgSim::DOFTransform*>(gun))
    {
        assert(type != Type::UNKNOWN);

        _gun->insertChild(0, new osg::MatrixTransform());
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
                _turret->setCurrentHPR(osg::Vec3(_azimuth.inRadians(), 0.0, 0.0));
                _gun->setCurrentHPR(osg::Vec3(0.0, _elevation.inRadians(), 0.0));
            }

            // M1 "heading" is Z, "pitch" is X.
            // Both _turret and _gun must be set for heading/azimuth; pitch/elevation is only
            // applied to the _gun.
            else
            {
                _turret->setCurrentHPR(osg::Vec3(0.0, 0.0, _azimuth.inRadians()));
                _gun->setCurrentHPR(osg::Vec3(-_elevation.inRadians(), 0.0, _azimuth.inRadians()));
            }
        }
    }

    void setAzimuth(Angle azimuth)
    {
        // Heading of the turret (in radians) relative to the local Z axis basis vector (points down)
        _azimuth = azimuth;
    }

    // setElevation()
    void setElevation(Angle elevation)
    {
        // Elevation of main gun (in radians) relative to the local Y axis basis vector (points right)
        _elevation = elevation;
    }

    void fireEffect()
    {
        auto pos = osg::Vec3();

        /* if (_type == Type::T72)
        {
            pos = osg::Vec3(-7.0, 1.5, 1.75) *
                osg::Matrix::rotate(_azimuth.inRadians(), 0.0, 0.0, 1.0) *
                osg::Matrix::rotate(_elevation.inRadians(), 0.0, 1.0, 0.0)
            ;
        }

        else if(_type == Type::M1)
        {
            pos = osg::Vec3(-5.0, 0.0, 1.25) *
                osg::Matrix::rotate(_azimuth.inRadians(), 0.0, 0.0, 1.0) *
                osg::Matrix::rotate(_elevation.inRadians(), -1.0, 0.0, 0.0)
            ;
        } */

        auto mt = dynamic_cast<osg::MatrixTransform*>(_gun->getChild(0));

        if (mt)
        {
            if (mt->getNumChildren()) mt->removeChildren(0, mt->getNumChildren());

            auto effect = new osgParticle::ExplosionEffect(pos, 2.0f, 3.0f);
            auto geode = new osg::Geode();

            effect->setUseLocalParticleSystem(false);
            effect->setEmitterDuration(0.1);

            geode->addDrawable(effect->getParticleSystem());

            mt->addChild(effect);
            mt->addChild(geode);
        }
    }

    Type getType() const
    {
        return _type;
    }

    osgSim::DOFTransform* getTurret()
    {
        return _turret.get();
    }

    osgSim::DOFTransform* getGun()
    {
        return _gun.get();
    }

    bool valid() const
    {
        return (_type != Type::UNKNOWN) && _turret.valid() && _gun.valid();
    }

protected:
    Type _type;

    osg::ref_ptr<osgSim::DOFTransform> _turret;
    osg::ref_ptr<osgSim::DOFTransform> _gun;

    // Temporary holding values during articulation.
    Angle _azimuth, _elevation;
};

class TankVisitor : public osg::NodeVisitor
{
public:
    TankVisitor(const std::string& turret, const std::string& gun):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _type(Tank::Type::UNKNOWN),
        _nameTurret(turret),
        _nameGun(gun)
    {
    }

    virtual void apply(osg::Transform& t) override
    {
        const auto transformName = t.getName();

        if (transformName == "T72")
        {
            _type = Tank::Type::T72;
        }

        else if (transformName == "M1")
        {
            _type = Tank::Type::M1;
        }

        else if (transformName == _nameTurret)
        {
            _turret = &t;
        }

        else if (transformName == _nameGun)
        {
            _gun = &t;
        }

        traverse(t);
    }

    std::unique_ptr<Tank> getTank()
    {
        if (_turret.get() && _gun.get())
        {
            return std::unique_ptr<Tank>(new Tank(_type, _turret.get(), _gun.get()));
        }
        else
        {
            return std::unique_ptr<Tank>();
        }
    }

protected:
    Tank::Type _type;

    std::string _nameTurret;
    std::string _nameGun;

    osg::ref_ptr<osg::Transform> _turret;
    osg::ref_ptr<osg::Transform> _gun;
};