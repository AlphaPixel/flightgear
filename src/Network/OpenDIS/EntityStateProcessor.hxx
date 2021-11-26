// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#pragma once

#include <dis6/EntityStatePdu.h>                  // for typedef
#include <utils/IPacketProcessor.h>                // for base class
#include <map>
#include <memory>

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
        size_t m_modelIndex;

        Entity(const DIS::EntityStatePdu &mostRecentPdu, size_t modelIndex)
            : m_mostRecentPdu(mostRecentPdu)
            , m_modelIndex(modelIndex)
        {
        }
    };

    void AddEntityToScene(const DIS::EntityStatePdu& entityPDU);
    void UpdateEntityInScene(Entity &entity, const DIS::EntityStatePdu& entityPDU);
    void RemoveExpiredEntities();
    std::unique_ptr<Entity> CreateEntity(const DIS::EntityStatePdu& entityPDU, size_t modelIndex);

    // Specific entities to create
    std::unique_ptr<Entity> CreateAH64(const DIS::EntityStatePdu& entityPDU);
    std::unique_ptr<Entity> CreateM1(const DIS::EntityStatePdu& entityPDU);
    std::unique_ptr<Entity> CreateT72(const DIS::EntityStatePdu& entityPDU);
    std::unique_ptr<Entity> CreateUH60(const DIS::EntityStatePdu& entityPDU);

    // Model allocation arrays
    std::vector<size_t> m_availableModels_AH64;
    std::vector<size_t> m_availableModels_M1;
    std::vector<size_t> m_availableModels_T72;
    std::vector<size_t> m_availableModels_UH60;

    std::map<unsigned short, Entity> m_entityMap;   // Keyed on entity ID.
};
