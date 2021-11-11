// opendis_subsystem.h
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)

#pragma once

#include <simgear/structure/subsystem_mgr.hxx>

class OpenDISManager;

class OpenDISSubsystem : public SGSubsystem
{
public:
    OpenDISSubsystem();
    ~OpenDISSubsystem();

    // Subsystem API.
    void init() override;
    void reinit() override;
    void shutdown() override;
    void update(double delta_time_sec) override;

    // Subsystem identification.
    static const char* staticSubsystemClassId() { return "OpenDIS"; }

    bool startManager(const SGPropertyNode* arg, SGPropertyNode* root);
    bool stopManager(const SGPropertyNode* arg, SGPropertyNode* root);

private:
    bool managerRunning = false;
    bool initialized = false;

    std::unique_ptr<OpenDISManager> m_manager;
};
