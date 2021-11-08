// opendis_manager.cpp
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "opendis_manager.h"

std::unique_ptr<OpenDISManager> OpenDISManager::create()
{
    return std::unique_ptr<OpenDISManager>(new OpenDISManager());
}

OpenDISManager::OpenDISManager()
{
}

OpenDISManager::~OpenDISManager()
{
}

void OpenDISManager::update(double time_delta_sec)
{
}
