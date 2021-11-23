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
        std::string m_propertyName;             // Name of root in property tree

        Entity(const DIS::EntityStatePdu &mostRecentPdu, const std::string propertyName)
            : m_mostRecentPdu(mostRecentPdu)
            , m_propertyName(propertyName)
        {
        }
    };

    void AddEntityToScene(const DIS::EntityStatePdu& entityPDU);
    void UpdateEntityInScene(Entity &entity, const DIS::EntityStatePdu& entityPDU);
    void RemoveExpiredEntities();
    std::unique_ptr<Entity> CreateEntity(const DIS::EntityStatePdu& entityPDU, const std::string &propertyTreePath);

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

    std::map<unsigned short, Entity> m_entityMap;   // Keyed on entity ID.
};
