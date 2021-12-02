// EntityManager.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <utils/IPacketProcessor.h>
#include <dis6/EntityStatePdu.h>
#include <dis6/FirePdu.h>
#include <dis6/DetonationPdu.h>

class EntityStatePDUHandler : public DIS::IPacketProcessor
{
public:
    virtual void ProcessEntityStatePDU(const DIS::EntityStatePdu &) = 0;

    // DIS::IPacketProcessor
    void Process(const DIS::Pdu& packet) override
    {
        const DIS::EntityStatePdu& entityPDU = static_cast<const DIS::EntityStatePdu&>(packet);
        ProcessEntityStatePDU(entityPDU);
    }
};

class FirePDUHandler : public DIS::IPacketProcessor
{
public:
    virtual void ProcessFirePDU(const DIS::FirePdu &) = 0;

    // DIS::IPacketProcessor
    void Process(const DIS::Pdu& packet) override
    {
        const DIS::FirePdu& firePDU = static_cast<const DIS::FirePdu&>(packet);
        ProcessFirePDU(firePDU);
    }
};

class DetonationPDUHandler : public DIS::IPacketProcessor
{
public:
    virtual void ProcessDetonationPDU(const DIS::DetonationPdu &) = 0;

    // DIS::IPacketProcessor
    void Process(const DIS::Pdu& packet) override
    {
        const DIS::DetonationPdu& detonationPDU = static_cast<const DIS::DetonationPdu&>(packet);
        ProcessDetonationPDU(detonationPDU);
    }
};
