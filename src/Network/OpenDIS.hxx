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
#ifndef _FG_OPENDIS_HXX
#define _FG_OPENDIS_HXX

#include "protocol.hxx"

#include <dis6/EntityStatePdu.h>
#include <utils/IncomingMessage.h>                 // for library usage
#include "OpenDIS/EntityTypes.hxx"

class EntityStateProcessor;
class FlightProperties;
class SGSocket;

class FGOpenDIS : public FGProtocol 
{
public:
    FGOpenDIS();
    ~FGOpenDIS();

    bool gen_message();
    bool parse_message();
 
    bool open() override;

    bool process() override;

    bool close() override;

private:
    void init_ownship();
    bool process_outgoing();

    std::vector<char> m_ioBuffer;
    std::unique_ptr<DIS::IncomingMessage> m_incomingMessage;
    std::unique_ptr<EntityStateProcessor> m_entityStateProcessor;
    std::unique_ptr<FlightProperties> m_flightProperties;

    std::unique_ptr<SGSocket> m_outgoingSocket;
    DIS::DataStream m_outgoingBuffer;
    DIS::EntityStatePdu m_ownship;
    DIS::EntityID m_ownshipID;
    SikorskyS70AHelicopter m_ownshipType;
};

#endif // _FG_OPENDIS_HXX
