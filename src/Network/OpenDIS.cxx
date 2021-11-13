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

#include <Main/fg_props.hxx>
#include <Main/globals.hxx>

#include "OpenDIS.hxx"
#include "OpenDIS/EntityStateProcessor.hxx"

// OpenDIS headers
#include <dis6/EntityStatePdu.h>

FGOpenDIS::FGOpenDIS()
	: m_incomingMessage(new DIS::IncomingMessage)
	, m_entityStateProcessor(new EntityStateProcessor)
{
	m_ioBuffer.reserve(FG_MAX_MSG_SIZE);

	const unsigned char es_pdu_type = 1;	// REVIEW: What's this used for?
	m_incomingMessage->AddProcessor(es_pdu_type, m_entityStateProcessor.get());
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

    return true;
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
