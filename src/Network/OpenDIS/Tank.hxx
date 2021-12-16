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
    Tank(osg::Transform* turret, osg::Transform* gun):
        _turret(dynamic_cast<osgSim::DOFTransform*>(turret)),
        _gun(dynamic_cast<osgSim::DOFTransform*>(gun)) 
    {

    }

	osg::Vec3 getHPR() const 
    {
		return _turret->getCurrentHPR();
	}

	void setHPR(const osg::Vec3& hpr) 
    {
		_turret->setCurrentHPR(hpr);
		_gun->setCurrentHPR(hpr);
	}

    void beginArticulation()
    {
        _azimuth = 0;
        _elevation = 0;
    }

    void endArticulation()
    {
        // Put azimuth and elevation into HPR
        // DIS spec says order is: azimuth -> elevation -> rotation (which we're not handling)
        // Those are standard Euler angle ordering.
        
        // TODO: Jeremy
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

	osgSim::DOFTransform* getTurret() 
    { 
        return _turret.get(); 
    }

	osgSim::DOFTransform* getGun() 
    { 
        return _gun.get(); 
    }

protected:
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
		if(t.getName() == _nameTurret) 
        {
            _turret = &t;
        }
		else if(t.getName() == _nameGun)
        {
            _gun = &t;
        }

		traverse(t);
	}

	std::unique_ptr<Tank> getTank() 
    {
		return std::unique_ptr<Tank>(new Tank(_turret.get(), _gun.get()));
	}

protected:
	std::string _nameTurret;
	std::string _nameGun;

	osg::ref_ptr<osg::Transform> _turret;
	osg::ref_ptr<osg::Transform> _gun;
};
