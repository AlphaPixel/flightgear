// OpenDIX.hxx -- OpenDIS protocal class
//
// Written by AlphaPixel
//
// Copyright (C) 2021  AlphaPixel (john@alphapixel.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <FDM/flightProperties.hxx>
//#include <simgear/debug/logstream.hxx>
//#include <simgear/math/sg_geodesy.hxx>
#include <simgear/io/iochannel.hxx>
#include <simgear/timing/sg_time.hxx>
#include <simgear/io/sg_socket.hxx>
#include <Main/fg_props.hxx>
#include <Main/globals.hxx>
#include <FDM/JSBSim/math/FGLocation.h>

#include "OpenDIS.hxx"
#include "OpenDIS/EntityStateProcessor.hxx"

// OpenDIS headers
#include <dis6/EntityStatePdu.h>

#include "OpenDIS/EntityTypes.hxx"

FGOpenDIS::FGOpenDIS()
	: m_incomingMessage(new DIS::IncomingMessage)
	, m_entityStateProcessor(new EntityStateProcessor)
	, m_ownshipType(Specific_SIKORSKY_S70A::UH_60A_BLACKHAWK, CountryCode::UNITED_STATES)
{
	m_ioBuffer.reserve(FG_MAX_MSG_SIZE);

	m_incomingMessage->AddProcessor(static_cast<unsigned char>(PDUType::ENTITY_STATE), m_entityStateProcessor.get());
}

FGOpenDIS::~FGOpenDIS()
{
}

bool FGOpenDIS::gen_message() 
{
    return true;
}

bool FGOpenDIS::parse_message() 
{
	m_incomingMessage->Process(&m_ioBuffer[0], m_ioBuffer.size(), DIS::BIG);
    return true;
}

bool FGOpenDIS::open() 
{
	bool openedSuccessfully = false;
    if(is_enabled()) 
	{
		SG_LOG(SG_IO, SG_ALERT, "FGOpenDIS: Attempted to open an already enabled protocol - ignoring.");
    }
	else
	{
		SGIOChannel *io = get_io_channel();
		if(!io->open(get_direction()))
		{
			SG_LOG(SG_IO, SG_ALERT, "FGOpenDIS: Error opening channel communication layer.");
		}
		else 
		{
			openedSuccessfully = true;
		    set_enabled(true);
		}
	}		

	m_outgoingSocket = std::unique_ptr<SGSocket>(new SGSocket("255.255.255.255", "3000", "broadcast"));

	init_ownship();

    return openedSuccessfully;
}

bool FGOpenDIS::process() 
{
    SGIOChannel *io = get_io_channel();

    if(get_direction() == SG_IO_OUT) 
	{
		gen_message();
		if(!io->write(&m_ioBuffer[0], m_ioBuffer.size())) 
		{
	    	SG_LOG( SG_IO, SG_WARN, "Error writing data.");
	    	return false;
		}			
	}
    else if(get_direction() == SG_IO_IN) 
	{
		m_ioBuffer.resize(FG_MAX_MSG_SIZE);
		int length = io->read(&m_ioBuffer[0], FG_MAX_MSG_SIZE);
		if (length > 0)
		{
			m_ioBuffer.resize(length);
			if(!parse_message()) 
			{
				SG_LOG(SG_IO, SG_ALERT, "Error parsing data.");
			}
		} 
		else if(errno == 0 || errno == ENOENT || errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)
		{
			// Socket didn't have any data...
			m_ioBuffer.resize(0);
		}
		else
		{
			SG_LOG(SG_IO, SG_ALERT, "Error reading data.");
			return false;
		}
    }

	return process_outgoing();
}

bool FGOpenDIS::close() 
{
    SGIOChannel *io = get_io_channel();

    set_enabled(false);

    if(!io->close())
	{
		return false;
    }

    return true;
}

void FGOpenDIS::init_ownship()
{
	m_ownshipID.setSite(0);
    m_ownshipID.setApplication(1);
    m_ownshipID.setEntity(1);

	m_ownship.setProtocolVersion(6);
	m_ownship.setExerciseID(0);
	m_ownship.setEntityID(m_ownshipID);
	m_ownship.setEntityType(m_ownshipType);
}

bool FGOpenDIS::process_outgoing()
{
#if 0
	const auto latitude = m_flightProperties->get_Latitude();
	const auto longitude = m_flightProperties->get_Longitude();
	const auto altitude_in_feet = m_flightProperties->get_Altitude();

	// Use JSBSim::FGLocation to determine the ECEF coordinates of the lat, lon, alt.
	JSBSim::FGLocation location;
	location.SetPositionGeodetic(longitude, latitude, altitude_in_feet);

	// Determine the velocity vector
	DIS::Vector3Float velocity;


	// Update ownship from flight dynamics
	DIS::Vector3Double position;
	position.setX(location(1));	// NOTE: FGLocation indices start at 1.
	position.setY(location(2));
	position.setZ(location(3));

	m_ownship.setEntityLocation(position);
	m_ownship.setEntityOrientation(dynamics.orientation);
	m_ownship.setEntityLinearVelocity(dynamics.velocity);
	m_ownship.setTimestamp(frame_stamp);
#endif

	return true;
}
