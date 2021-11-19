// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "EntityStateProcessor.hxx"

EntityStateProcessor::EntityStateProcessor(DIS::EntityStatePdu ownship)
    : m_ownship(ownship)
{
}

EntityStateProcessor::~EntityStateProcessor()
{
}

// DIS::IPacketProcessor
void EntityStateProcessor::Process(const DIS::Pdu& packet)
{
}
