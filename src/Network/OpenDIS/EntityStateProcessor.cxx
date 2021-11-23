// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "EntityStateProcessor.hxx"
#include "EntityTypes.hxx"
#include <FDM/JSBSim/math/FGLocation.h>
#include <FDM/JSBSim/math/FGColumnVector3.h>
#include <Main/fg_props.hxx>

static JSBSim::FGLocation ECEFToLocation(const DIS::Vector3Double &ecef)
{
    // Note: FGLocation expects ECEF to be specified in *FEET* but DIS 
    // reports them in *METERS*.
    const double meters_to_feet_scale_factor = 3.28084;

    return JSBSim::FGLocation(
        ecef.getX() * meters_to_feet_scale_factor,
        ecef.getY() * meters_to_feet_scale_factor,
        ecef.getZ() * meters_to_feet_scale_factor
    );
}

static std::map<unsigned short, std::string> FIXME_mapping;

EntityStateProcessor::EntityStateProcessor(DIS::EntityStatePdu ownship)
    : m_ownship(ownship)
{
	FIXME_mapping[0x0123] = "model";
	FIXME_mapping[0x0456] = "model[1]";
	FIXME_mapping[0x0789] = "model[2]";
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
    // Set properties on entity model
    JSBSim::FGLocation location = ECEFToLocation(entityPDU.getEntityLocation());

    auto longitude = location.GetLongitude();
    auto latitude = location.GetLatitude();
    auto altitude = location.GetAltitudeASL();

    fgSetDouble(entity.m_propertyName + "/latitude-deg", latitude);
    fgSetDouble(entity.m_propertyName + "/longitude-deg", longitude);
    fgSetDouble(entity.m_propertyName + "/elevation-ft", altitude);
    
    // auto orientation = entityPDU.getEntityOrientation();

    // fgSetDouble(entity.m_propertyName + "/phi", orientation.getPhi());
    // fgSetDouble(entity.m_propertyName + "/psi", orientation.getPsi());
    // fgSetDouble(entity.m_propertyName + "/theta", orientation.getTheta());
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
    // std::string propertyName = "/models/" + std::to_string(entityPDU.getEntityID().getEntity());
	std::string propertyName = "/models/" + FIXME_mapping[entityPDU.getEntityID().getEntity()];
    return std::unique_ptr<Entity>(new Entity(entityPDU, propertyName));
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateT72(const DIS::EntityStatePdu& entityPDU)
{
    auto entity = CreateEntity(entityPDU);
    if (entity)
    {
        // TODO: Jeremy - add a T72 at <id> where <id> = entity->m_propertyName

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
        // TODO: Jeremy - add a M1 at <id> where <id> = entity->m_propertyName

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
        // TODO: Jeremy - add a AH64 at <id> where <id> = entity->m_propertyName

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
        // TODO: Jeremy - add a UH60 at <id> where <id> = entity->m_propertyName

        // END TODO

        // The model has been created, set the initial properties on it.
        UpdateEntityInScene(*entity, entityPDU);
    }

    return entity;
}
