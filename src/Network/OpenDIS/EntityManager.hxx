// EntityManager.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <dis6/EntityStatePdu.h>                  // for typedef
#include <utils/IPacketProcessor.h>                // for base class
#include <map>
#include <memory>

#include "PDUHandlers.hxx"
#include "Main/fg_props.hxx"

// EntityIDCompare - custom comparison functor for std::map<>
struct EntityIDCompare 
{
    bool operator()(const DIS::EntityID &a, const DIS::EntityID &b) const 
    {
        return a.getEntity() < b.getEntity();
    }
};

// EntityManager - class that handles Entity management (create, update, delete)
class EntityManager : 
    virtual public EntityStatePDUHandler, 
    virtual public FirePDUHandler, 
    virtual public DetonationPDUHandler
{
public:
    EntityManager(DIS::EntityStatePdu ownship);
    virtual ~EntityManager();

    typedef DIS::EntityStatePdu PduType;

// TODO: Remove before shipping
#ifndef NDEBUG
    void PerformExtra();
#endif    

private:
    // EntityStatePDUHandler
    virtual void ProcessEntityStatePDU(const DIS::EntityStatePdu& entityPDU) override;

    // FirePDUHandler
    virtual void ProcessFirePDU(const DIS::FirePdu &firePDU) override;

    // DetonationPDUHandler
    virtual void ProcessDetonationPDU(const DIS::DetonationPdu &detonationPDU) override;

private:
    bool ShouldIgnorePDU(const DIS::Pdu &pdu);

    // EntityState PDU Processing
    bool ShouldIgnoreEntityStatePDU(const DIS::EntityStatePdu& entityPDU);
    void HandleEntityStatePDU(const DIS::EntityStatePdu& entityPDU);

    // Fire PDU Processing
    bool ShouldIgnoreFirePDU(const DIS::FirePdu &firePDU);
    void HandleFirePDU(const DIS::FirePdu &firePDU);

    // Detonation PDU Processing
    bool ShouldIgnoreDetonationPDU(const DIS::DetonationPdu &detonationPDU);
    void HandleDetonationPDU(const DIS::DetonationPdu &detonationPDU);

    DIS::EntityStatePdu m_ownship;

    // Structure used to track DIS entities in the scenegraph
    struct Entity
    {
        DIS::EntityStatePdu m_mostRecentPdu;
        std::string m_modelPath;

        Entity(const DIS::EntityStatePdu &mostRecentPdu, const std::string modelPath)
            : m_mostRecentPdu(mostRecentPdu)
            , m_modelPath(modelPath)
        {
        }
    };

    void AddEntityToScene(const DIS::EntityStatePdu& entityPDU);
    void UpdateEntityInScene(Entity &entity, const DIS::EntityStatePdu& entityPDU);
    void RemoveExpiredEntities();
    std::unique_ptr<Entity> CreateEntity(const DIS::EntityStatePdu& entityPDU, const std::string& modelPath);

    // Specific entities to create
    std::unique_ptr<Entity> CreateAH64(const DIS::EntityStatePdu& entityPDU);
    std::unique_ptr<Entity> CreateM1(const DIS::EntityStatePdu& entityPDU);
    std::unique_ptr<Entity> CreateT72(const DIS::EntityStatePdu& entityPDU);
    std::unique_ptr<Entity> CreateUH60(const DIS::EntityStatePdu& entityPDU);

    // Model allocation arrays
    std::vector<std::string> m_availableModels_AH64;
    std::vector<std::string> m_availableModels_M1;
    std::vector<std::string> m_availableModels_T72;
    std::vector<std::string> m_availableModels_UH60;

    // EntityID -> Entity map
    std::map<DIS::EntityID, Entity, EntityIDCompare> m_entityMap;
};
