// EntityStateProcessor.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "EntityStateProcessor.hxx"
#include "EntityTypes.hxx"
#include <Main/fg_props.hxx>
#include <Model/modelmgr.hxx>
#include <FDM/flight.hxx>
#include <FDM/fdm_shell.hxx>
#include <simgear/scene/model/placement.hxx>
#include <osg/Math>  // RadiansToDegrees()
#include <simgear/math/sg_geodesy.hxx>

static const size_t modelCount_UH60 = 6;
static const size_t modelCount_M1 = 14;
static const size_t modelCount_T72 = 11;

static const double meters_to_feet_scale_factor = 3.28084;

static void ECEFtoLLA(const DIS::Vector3Double &ecef, double &latitude, double &longitude, double &altitude_in_meters)
{
    const double xyz[3] = {ecef.getX(), ecef.getY(), ecef.getZ() };
    sgCartToGeod(xyz, &latitude, &longitude, &altitude_in_meters);
}

static double GetGroundLevelInFeet(const SGGeod& position)
{
    double groundLevel = 0.0;
    FDMShell* fdm = static_cast<FDMShell*>(globals->get_subsystem("flight"));
    FGInterface* fdmState = fdm->getInterface();
    if (fdmState) 
    {
        groundLevel = fdmState->get_groundlevel_m(position) * meters_to_feet_scale_factor;
    }

    return groundLevel;
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

    for (size_t modelIndex = 0; modelIndex < modelCount_M1; ++modelIndex) 
    {
        m_availableModels_M1.push_back(globalModelIndex + modelIndex);
    }
    globalModelIndex += modelCount_M1;

    for (size_t modelIndex = 0; modelIndex < modelCount_T72; ++modelIndex) 
    {
        m_availableModels_T72.push_back(globalModelIndex + modelIndex);
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

        // Get the lat/lon/altitude from the PDU
        double latitudeInRadians, longitudeInRadians, altitudeInMeters;
        ECEFtoLLA(entityPDU.getEntityLocation(), latitudeInRadians, longitudeInRadians, altitudeInMeters);

        // Change from radians to degrees and from meters to feet.
        auto latitudeInDegrees = osg::RadiansToDegrees(latitudeInRadians);
        auto longitudeInDegrees = osg::RadiansToDegrees(longitudeInRadians);
        auto altitudeInFeet = altitudeInMeters * meters_to_feet_scale_factor;

        // NOTE/HACK: If the altitude given is below the actual ground, adjust the altitude to put it on the ground.
        auto position = SGGeod::fromDegFt(longitudeInDegrees, latitudeInDegrees, altitudeInFeet);
        auto groundLevelInFeet = GetGroundLevelInFeet(position);
        if (altitudeInFeet < groundLevelInFeet)
        {
            altitudeInFeet = groundLevelInFeet;
            position.setElevationFt(groundLevelInFeet);
        }

        // Get the orientation of the model from the PDU
        auto orientation = entityPDU.getEntityOrientation();
        auto psi = orientation.getPsi();
        auto theta = orientation.getTheta();
        auto phi = orientation.getPhi();

        // NOTE/HACK: we write to both the model and the property system.  This must be done because sometimes (UFO mode), based on the
        // FDM in use, the property system updates won't make it down into the model and other times (non-UFO mode) they will
        // and will overwrite what's written in the model.
        auto model = modelInstance->model;
        model->setPosition(position);

        // NOTES:
        // Euler angle rotation sequence (per DIS spec)
        // Z Axis Rotation (first) - Psi
        // Y Axis Rotation (second) - Theta
        // X Axis Rotation (third) - Phi
        //
        // DIS axis orientation, 
        // X - points forward
        // Y - points to the right
        // Z - points down

        // NOTE: DIS and FG don't always agree on orientations.  These are the various corrections.
        auto headingCorrection = 3.14159;   // Spin the model 180 degrees in heading.
        // auto pitchCorrection = 0.0;
        // auto rollCorrection = 0.0;

        auto heading = phi + headingCorrection;
        //auto pitch = psi;
        //auto roll = psi;

        auto q = SGQuatd::fromEulerRad(psi, theta, phi);
        model->setOrientation(q);
        model->update();

        // Set the values in the property system.
        const std::string propertyPath("/models/model" + (entity.m_modelIndex == 0 ? "" : ("[" + std::to_string(entity.m_modelIndex) + "]")));

        fgSetDouble(propertyPath + "/latitude-deg", latitudeInDegrees);
        fgSetDouble(propertyPath + "/longitude-deg", longitudeInDegrees);
        fgSetDouble(propertyPath + "/elevation-ft", altitudeInFeet);

        fgSetDouble(propertyPath + "/heading-deg", osg::RadiansToDegrees(heading));
//        fgSetDouble(propertyPath + "/pitch-deg", osg::RadiansToDegrees(pitch));
//        fgSetDouble(propertyPath + "/roll-deg", osg::RadiansToDegrees(roll));
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
