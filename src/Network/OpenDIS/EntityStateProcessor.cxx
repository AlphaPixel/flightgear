// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "EntityStateProcessor.hxx"
#include "EntityTypes.hxx"
#include <FDM/JSBSim/math/FGLocation.h>
#include <FDM/JSBSim/math/FGColumnVector3.h>
#include <Main/fg_props.hxx>
#include <Model/modelmgr.hxx>
#include <simgear/scene/model/placement.hxx>

static const size_t modelCount_UH60 = 6;
static const size_t modelCount_T72 = 14;
static const size_t modelCount_M1 = 14;

static JSBSim::FGLocation ECEFToLocation(const DIS::Vector3Double &ecef)
{
    // Note: FGLocation expects ECEF to be specified in *FEET* but DIS 
    // reports them in *METERS*.
    const double meters_to_feet_scale_factor = 3.28084;

    return JSBSim::FGLocation(
        JSBSim::FGColumnVector3(
            ecef.getX() * meters_to_feet_scale_factor,
            ecef.getY() * meters_to_feet_scale_factor,
            ecef.getZ() * meters_to_feet_scale_factor
        )
    );
}

EntityStateProcessor::EntityStateProcessor(DIS::EntityStatePdu ownship)
    : m_ownship(ownship)
{
    // Set up model availibility arrays
    size_t globalModelIndex = 0;
    for (size_t modelIndex = 0; modelIndex < modelCount_UH60; ++modelIndex) 
    {
        m_availableModels_UH60.push_back(globalModelIndex + modelIndex);
    }
    globalModelIndex += modelCount_UH60;

    for (size_t modelIndex = 0; modelIndex < modelCount_T72; ++modelIndex) 
    {
        m_availableModels_T72.push_back(globalModelIndex + modelIndex);
    }
    globalModelIndex += modelCount_T72;

    for (size_t modelIndex = 0; modelIndex < modelCount_M1; ++modelIndex) 
    {
        m_availableModels_M1.push_back(globalModelIndex + modelIndex);
    }
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
    auto mmss = globals->get_subsystem("model-manager");
    auto mm = dynamic_cast<FGModelMgr*>(mmss);

    auto modelInstances = mm->getInstances();
    if (entity.m_modelIndex < modelInstances.size())
    {
        // Set properties on entity model
        auto modelInstance = modelInstances[entity.m_modelIndex];

        JSBSim::FGLocation location = ECEFToLocation(entityPDU.getEntityLocation());

        auto longitude = location.GetLongitudeDeg();
        auto latitude = location.GetLatitudeDeg();
        auto altitude = location.GetAltitudeASL();

        auto model = modelInstance->model;
        model->setPosition(SGGeod::fromDegFt(longitude, latitude, altitude));
        model->update();

        // auto orientation = entityPDU.getEntityOrientation();

        // fgSetDouble(entity.m_propertyName + "/phi", orientation.getPhi());
        // fgSetDouble(entity.m_propertyName + "/psi", orientation.getPsi());
        // fgSetDouble(entity.m_propertyName + "/theta", orientation.getTheta());
    }
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

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateEntity(const DIS::EntityStatePdu& entityPDU, size_t modelIndex)
{
    return std::unique_ptr<Entity>(new Entity(entityPDU, modelIndex));
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateT72(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityStateProcessor::Entity> entity;
    if (!m_availableModels_T72.empty())
    {
        entity = CreateEntity(entityPDU, m_availableModels_T72.back());
        if (entity)
        {
            m_availableModels_T72.pop_back();

            // The model has been created, set the initial properties on it.
            UpdateEntityInScene(*entity, entityPDU);
        }
    }

    return entity;
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateM1(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityStateProcessor::Entity> entity;
    if (!m_availableModels_M1.empty())
    {
        entity = CreateEntity(entityPDU, m_availableModels_M1.back());
        if (entity)
        {
            m_availableModels_M1.pop_back();

            // The model has been created, set the initial properties on it.
            UpdateEntityInScene(*entity, entityPDU);
        }
    }

    return entity;
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateAH64(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityStateProcessor::Entity> entity;
    if (!m_availableModels_AH64.empty())
    {
        entity = CreateEntity(entityPDU, m_availableModels_AH64.back());
        if (entity)
        {
            m_availableModels_AH64.pop_back();

            // The model has been created, set the initial properties on it.
            UpdateEntityInScene(*entity, entityPDU);
        }
    }

    return entity;
}

std::unique_ptr<EntityStateProcessor::Entity> EntityStateProcessor::CreateUH60(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityStateProcessor::Entity> entity;
    if (!m_availableModels_UH60.empty())
    {
        entity = CreateEntity(entityPDU, m_availableModels_UH60.back());
        if (entity)
        {
            m_availableModels_UH60.pop_back();

            // The model has been created, set the initial properties on it.
            UpdateEntityInScene(*entity, entityPDU);
        }
    }

    return entity;
}
