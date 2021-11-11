// opendis_manager.cpp
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "opendis_manager.h"

std::unique_ptr<OpenDISManager> OpenDISManager::create()
{
    return std::unique_ptr<OpenDISManager>(new OpenDISManager());
}

OpenDISManager::OpenDISManager()
    : m_flightProperties(new FlightProperties)
{
}

OpenDISManager::~OpenDISManager()
{
}

void OpenDISManager::update(double time_delta_sec)
{
    // Push any non-ownship properties that have changed since the last update into the property tree.

    // Push any ownship properties from the simulation out to any DIS listeners.
}
