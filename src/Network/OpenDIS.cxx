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
#include <simgear/io/iochannel.hxx>
#include <simgear/timing/sg_time.hxx>
#include <simgear/io/sg_socket.hxx>
#include <Main/fg_props.hxx>
#include <Main/globals.hxx>
#include <simgear/math/sg_geodesy.hxx>

#include "OpenDIS.hxx"
#include "OpenDIS/EntityManager.hxx"
#include "OpenDIS/EntityTypes.hxx"
#include "OpenDIS/Frame.hxx"
#include "OpenDIS/CoordinateSystems.hxx"

// OpenDIS headers
#include <dis6/EntityStatePdu.h>

FGOpenDIS::FGOpenDIS()
	: m_incomingMessage(new DIS::IncomingMessage)
	, m_flightProperties(new FlightProperties)
	, m_ownshipType(Specific_SIKORSKY_S70A::UH60A_BLACKHAWK)
	, m_outgoingBuffer(DIS::BIG)
{
	m_ioBuffer.reserve(FG_MAX_MSG_SIZE);
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

	init_ownship();

	m_entityManager.reset(new EntityManager(m_ownship));

	m_incomingMessage->AddProcessor(
		static_cast<unsigned char>(PDUType::ENTITY_STATE), 
		static_cast<EntityStatePDUHandler *>(m_entityManager.get())
	);

	m_incomingMessage->AddProcessor(
		static_cast<unsigned char>(PDUType::FIRE), 
		static_cast<FirePDUHandler *>(m_entityManager.get())
	);

	m_incomingMessage->AddProcessor(
		static_cast<unsigned char>(PDUType::DETONATION), 
		static_cast<DetonationPDUHandler *>(m_entityManager.get())
	);

	m_outgoingSocket = std::unique_ptr<SGSocket>(new SGSocket("255.255.255.255", "3000", "broadcast"));
	m_outgoingSocket->open(SGProtocolDir::SG_IO_OUT);

    return openedSuccessfully;
}

bool FGOpenDIS::simulation_ready() const
{
	// return true;

	bool simulationReady = false;
	const bool fdmInitialized = fgGetBool("/sim/fdm-initialized", false);
	const time_t simulationSettleTimeInSeconds = 3;

	if (fdmInitialized)
	{
		static bool init = false;
		static time_t initTime = 0;
		if (!init)
		{
			init = true;
			initTime = time(NULL);
		}

		const auto currentTime = time(NULL);
		const auto elapsedSeconds = difftime(currentTime, initTime);

		if (elapsedSeconds > simulationSettleTimeInSeconds)
		{
			simulationReady = true;
		}
	}

	return simulationReady;
}

bool FGOpenDIS::process() 
{
	const bool paused = fgGetBool("/sim/freeze/master",true) | fgGetBool("/sim/freeze/clock",true);

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
		int length;
		do
		{
			m_ioBuffer.resize(FG_MAX_MSG_SIZE);
			length = io->read(&m_ioBuffer[0], FG_MAX_MSG_SIZE);
			if (length > 0)
			{
				m_ioBuffer.resize(length);
				if(simulation_ready() && !paused && !parse_message()) 
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
		while (length > 0);
    }

#ifndef NDEBUG
	// TODO: Remove before shipping
	if (simulation_ready())
	{
		m_entityManager->PerformExtra();
	}
#endif	

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
	m_ownshipID.setSite(fgGetInt("/sim/dis/site", 1767));
    m_ownshipID.setApplication(fgGetInt("/sim/dis/application", 3534));
    m_ownshipID.setEntity(fgGetInt("/sim/dis/ownship_id", 8888));

	m_ownship.setProtocolVersion(6);
	m_ownship.setExerciseID(0);
	m_ownship.setEntityID(m_ownshipID);
	m_ownship.setEntityType(m_ownshipType);
}

bool FGOpenDIS::process_outgoing()
{
	const auto latitude = Angle::fromRadians(m_flightProperties->get_Latitude());
	const auto longitude = Angle::fromRadians(m_flightProperties->get_Longitude());
	const auto altitude = Distance::fromFeet(m_flightProperties->get_Altitude());

	//
	// Update ownship from flight dynamics
	//

	// Position
	{
		// Determine ECEF coordinates from Lat/Lon/Alt
		ECEF location(LLA(latitude, longitude, altitude));
		DIS::Vector3Double locationVector;

		locationVector.setX(location.GetX().inMeters());
		locationVector.setY(location.GetY().inMeters());
		locationVector.setZ(location.GetZ().inMeters());

		m_ownship.setEntityLocation(locationVector);
	}

	// Orientation
	{
		const auto baseNED = Frame::fromLatLon(latitude, longitude);
		Frame entityNED = baseNED;
		auto intermediate = Frame::zero();

		{
			const auto psi = m_flightProperties->get_Psi();
			const auto theta = m_flightProperties->get_Theta();
			const auto phi = m_flightProperties->get_Phi();

			DIS::Orientation orientation;
			orientation.setPsi(psi);
			orientation.setTheta(theta);

#if 1		// TODO: Fix.  NED to ECEF hack
			orientation.setPhi(phi + Angle::fromDegrees(180).inRadians());
#endif			

			entityNED.rotate(baseNED, orientation, &intermediate);
		}

		const auto orientation = Frame::GetEulerAngles(Frame::fromECEFBase(), intermediate, entityNED);

		m_ownship.setEntityOrientation(orientation);
	}

	m_ownship.marshal(m_outgoingBuffer);
	m_outgoingSocket->write(&m_outgoingBuffer[0], m_outgoingBuffer.size());
	m_outgoingBuffer.clear();

	return true;
}
