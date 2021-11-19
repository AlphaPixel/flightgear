// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <dis6/EntityStatePdu.h>                  // for typedef
#include <utils/IPacketProcessor.h>                // for base class
#include <map>

class EntityStateProcessor : public DIS::IPacketProcessor
{
public:
    EntityStateProcessor(DIS::EntityStatePdu ownship);
    virtual ~EntityStateProcessor();

    typedef DIS::EntityStatePdu PduType;

    // DIS::IPacketProcessor
    void Process(const DIS::Pdu& packet) override;

private:
    bool ShouldIgnoreEntityPDU(const DIS::EntityStatePdu& entityPDU);
    void ProcessEntityPDU(const DIS::EntityStatePdu& entityPDU);

    DIS::EntityStatePdu m_ownship;

    // Structure used to track DIS entities in the scenegraph
    struct Entity
    {
        DIS::EntityStatePdu m_mostRecentPdu;
        std::string m_propertyName;             // Name of root in property tree
    };

    void AddEntityToScene(const DIS::EntityStatePdu& entityPDU);
    void UpdateEntityInScene(Entity &entity, const DIS::EntityStatePdu& entityPDU);
    void RemoveExpiredEntities();

    std::map<unsigned short, Entity> m_entityMap;   // Keyed on entity ID.
};
