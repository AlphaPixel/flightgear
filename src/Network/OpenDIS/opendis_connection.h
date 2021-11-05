// opendis_connection.h
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)

#pragma once

#include <simgear/structure/subsystem_mgr.hxx>

class OpenDIS7Connection : public SGSubsystem
{
public:
    OpenDIS7Connection();
    ~OpenDIS7Connection();

    // Subsystem API.
    void init() override;
    void reinit() override;
    void shutdown() override;
    void update(double delta_time_sec) override;

    // Subsystem identification.
    static const char* staticSubsystemClassId() { return "opendis7"; }

    bool startServer(const SGPropertyNode* arg, SGPropertyNode* root);
    bool stopServer(const SGPropertyNode* arg, SGPropertyNode* root);

//    std::unique_ptr<FGSwiftBus::CPlugin> plug{};

private:
    bool serverRunning = false;
    bool initialized = false;
};
