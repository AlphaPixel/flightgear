// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <dis6/EntityStatePdu.h>                  // for typedef
#include <utils/IPacketProcessor.h>                // for base class

class EntityStateProcessor : public DIS::IPacketProcessor
{
public:
    EntityStateProcessor();
    virtual ~EntityStateProcessor();

    typedef DIS::EntityStatePdu PduType;

    // DIS::IPacketProcessor
    void Process(const DIS::Pdu& packet) override;
};
