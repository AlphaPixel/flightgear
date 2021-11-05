// opendis_connection.cpp
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "opendis_connection.h"
#include <Main/fg_props.hxx>
#include <simgear/compiler.h>
#include <simgear/debug/logstream.hxx>
#include <simgear/io/raw_socket.hxx>
#include <simgear/misc/stdint.hxx>
#include <simgear/props/props.hxx>
#include <simgear/structure/commands.hxx>
#include <simgear/structure/event_mgr.hxx>
#include <simgear/structure/subsystem_mgr.hxx>
#include <simgear/timing/timestamp.hxx>

bool OpenDIS7Connection::startServer(const SGPropertyNode* arg, SGPropertyNode* root)
{
//    SwiftConnection::plug.reset(new FGSwiftBus::CPlugin{});

    serverRunning = true;
    fgSetBool("/sim/opendis/serverRunning", true);

    return true;
}

bool OpenDIS7Connection::stopServer(const SGPropertyNode* arg, SGPropertyNode* root)
{
    fgSetBool("/sim/opendis/serverRunning", false);
    serverRunning = false;

//    SwiftConnection::plug.reset();

    return true;
}

OpenDIS7Connection::OpenDIS7Connection()
{
    init();
}

OpenDIS7Connection::~OpenDIS7Connection()
{
    shutdown();

    if (serverRunning) {
//        SwiftConnection::plug.reset();
    }
}

void OpenDIS7Connection::init()
{
    if (!initialized) {
        globals->get_commands()->addCommand("opendisStart", this, &OpenDIS7Connection::startServer);
        globals->get_commands()->addCommand("opendisStop", this, &OpenDIS7Connection::stopServer);

        fgSetBool("/sim/opendis/available", true);
        initialized = true;
    }
}

void OpenDIS7Connection::update(double delta_time_sec)
{
    if (serverRunning) {
        //SwiftConnection::plug->fastLoop();
    }
}

void OpenDIS7Connection::shutdown()
{
    if (initialized) {
        fgSetBool("/sim/opendis/available", false);
        initialized = false;

        globals->get_commands()->removeCommand("opendisStart");
        globals->get_commands()->removeCommand("opendisStop");
    }
}

void OpenDIS7Connection::reinit()
{
    shutdown();
    init();
}

// Register the subsystem.
SGSubsystemMgr::Registrant<OpenDIS7Connection> registrantOpenDIS7Connection(
    SGSubsystemMgr::POST_FDM);
