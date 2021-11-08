// opendis_subsystem.cpp
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "opendis_subsystem.h"
#include "opendis_manager.h"
#include <Main/fg_props.hxx>
#include <simgear/structure/commands.hxx>

// #include <simgear/compiler.h>
// #include <simgear/debug/logstream.hxx>
// #include <simgear/io/raw_socket.hxx>
// #include <simgear/misc/stdint.hxx>
// #include <simgear/props/props.hxx>
// #include <simgear/structure/event_mgr.hxx>
// #include <simgear/timing/timestamp.hxx>

bool OpenDISSubsystem::startServer(const SGPropertyNode* arg, SGPropertyNode* root)
{
    m_manager = OpenDISManager::create();

    serverRunning = true;
    fgSetBool("/sim/opendis/serverRunning", true);

    return true;
}

bool OpenDISSubsystem::stopServer(const SGPropertyNode* arg, SGPropertyNode* root)
{
    fgSetBool("/sim/opendis/serverRunning", false);
    serverRunning = false;

    m_manager.reset();

    return true;
}

OpenDISSubsystem::OpenDISSubsystem()
{
    init();
}

OpenDISSubsystem::~OpenDISSubsystem()
{
    shutdown();

    if (serverRunning) 
    {
    }
}

void OpenDISSubsystem::init()
{
    if (!initialized) 
    {
        globals->get_commands()->addCommand("opendisStart", this, &OpenDISSubsystem::startServer);
        globals->get_commands()->addCommand("opendisStop", this, &OpenDISSubsystem::stopServer);

        fgSetBool("/sim/opendis/available", true);
        initialized = true;
    }
}

void OpenDISSubsystem::update(double delta_time_sec)
{
    if (serverRunning) 
    {
        m_manager->update(delta_time_sec);
    }
}

void OpenDISSubsystem::shutdown()
{
    if (initialized) 
    {
        fgSetBool("/sim/opendis/available", false);
        initialized = false;

        globals->get_commands()->removeCommand("opendisStart");
        globals->get_commands()->removeCommand("opendisStop");
    }
}

void OpenDISSubsystem::reinit()
{
    shutdown();
    init();
}

// Register the subsystem.
SGSubsystemMgr::Registrant<OpenDISSubsystem> registrantOpenDISSubsystem(SGSubsystemMgr::POST_FDM);
