// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "EntityStateProcessor.hxx"
#include "EntityTypes.hxx"
#include <FDM/JSBSim/math/FGLocation.h>
#include <FDM/JSBSim/math/FGColumnVector3.h>

EntityStateProcessor::EntityStateProcessor(DIS::EntityStatePdu ownship)
    : m_ownship(ownship)
{
}

EntityStateProcessor::~EntityStateProcessor()
{
}

bool EntityStateProcessor::ShouldIgnoreEntityPDU(const DIS::EntityStatePdu& packet)
{
    bool shouldIgnore = true;

    auto incomingEntityID = packet.getEntityID();
    auto ownshipEntityID = m_ownship.getEntityID();

    // Only pay attention to PDUs with site and application IDs the same as ownship.
    if (incomingEntityID.getSite() == ownshipEntityID.getSite() &&
        incomingEntityID.getApplication() == ownshipEntityID.getApplication())
    {
        // Ignore PDUs that are from our ownship (since we'll receive PDUs we send ourselves)
        if (incomingEntityID.getEntity() != ownshipEntityID.getEntity())
        {
            shouldIgnore = false;
        }
    }

    return shouldIgnore;
}

void EntityStateProcessor::ProcessEntityPDU(const DIS::EntityStatePdu& entityPDU)
{
    // Find the entity in the scene
    const auto i = m_entityMap.find(entityPDU.getEntityID().getEntity());
    if (i == m_entityMap.end())
    {
        AddEntityToScene(entityPDU);
    }
    else
    {
        UpdateEntityInScene((*i).second, entityPDU);
    }

    RemoveExpiredEntities();
}

void EntityStateProcessor::AddEntityToScene(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<Entity> entity;
    // Is the PDU for a T72?
    if (T72Tank::matches(entityPDU.getEntityType()))
    {
        entity = CreateT72(entityPDU);
    }
    else if (M1AbramsTank::matches(entityPDU.getEntityType()))
    {
        entity = CreateM1(entityPDU);
    }
    else if (AH64ApacheHelicopter::matches(entityPDU.getEntityType()))
    {
        entity = CreateAH64(entityPDU);
    }
    else if (SikorskyS70AHelicopter::matches(entityPDU.getEntityType()))
    {
        entity = CreateUH60(entityPDU);
    }

    // If an entity was created above, add it to the map
    if (entity)
    {
        m_entityMap.insert(std::make_pair(entityPDU.getEntityID().getEntity(), *entity));
    }
}

void EntityStateProcessor::UpdateEntityInScene(Entity &entity, const DIS::EntityStatePdu& entityPDU)
{
    // // Set properties on entity model
    // auto location = entityPDU.getEntityLocation();

    // FGColumnVector3 ecefCoordinates;

    // JSBSim::FGLocation location;
	// location.SetPositionGeodetic(longitude, latitude, altitude_in_feet);


}

void EntityStateProcessor::RemoveExpiredEntities()
{
}

// DIS::IPacketProcessor
void EntityStateProcessor::Process(const DIS::Pdu& packet)
{
    const DIS::EntityStatePdu& entityPDU = static_cast<const DIS::EntityStatePdu&>(packet);
    if (!ShouldIgnoreEntityPDU(entityPDU))
    {
        ProcessEntityPDU(entityPDU);
    }
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateEntity(const DIS::EntityStatePdu& entityPDU)
{
    std::string propertyName = "/models/" + std::to_string(entityPDU.getEntityID().getEntity());
    return std::unique_ptr<Entity>(new Entity(entityPDU, propertyName));
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateT72(const DIS::EntityStatePdu& entityPDU)
{
    auto entity = CreateEntity(entityPDU);
    if (entity)
    {
        // TODO: Jeremy - add a T72 at /models/<id> where <id> = entity->m_propertyName

        // END TODO

        // The model has been created, set the initial properties on it.
        UpdateEntityInScene(*entity, entityPDU);
    }

    return entity;
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateM1(const DIS::EntityStatePdu& entityPDU)
{
    auto entity = CreateEntity(entityPDU);
    if (entity)
    {
        // TODO: Jeremy - add a M1 at /models/<id> where <id> = entity->m_propertyName

        // END TODO

        // The model has been created, set the initial properties on it.
        UpdateEntityInScene(*entity, entityPDU);
    }

    return entity;
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateAH64(const DIS::EntityStatePdu& entityPDU)
{
    auto entity = CreateEntity(entityPDU);
    if (entity)
    {
        // TODO: Jeremy - add a AH64 at /models/<id> where <id> = entity->m_propertyName

        // END TODO

        // The model has been created, set the initial properties on it.
        UpdateEntityInScene(*entity, entityPDU);
    }

    return entity;
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateUH60(const DIS::EntityStatePdu& entityPDU)
{
    auto entity = CreateEntity(entityPDU);
    if (entity)
    {
        // TODO: Jeremy - add a UH60 at /models/<id> where <id> = entity->m_propertyName

        // END TODO

        // The model has been created, set the initial properties on it.
        UpdateEntityInScene(*entity, entityPDU);
    }

    return entity;
}
