// EntityManager.hxx
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include "EntityManager.hxx"
#include "EntityTypes.hxx"
#include "CoordinateSystems.hxx"
#include "Frame.hxx"

#include <Main/fg_props.hxx>
#include <Model/modelmgr.hxx>
#include <FDM/flight.hxx>
#include <FDM/fdm_shell.hxx>

static const size_t modelCount_UH60 = 6;
static const size_t modelCount_M1 = 14;
static const size_t modelCount_T72 = 11;

static double GetGroundLevelInFeet(const SGGeod& position)
{
    double groundLevel = 0.0;
    FDMShell* fdm = static_cast<FDMShell*>(globals->get_subsystem("flight"));
    FGInterface* fdmState = fdm->getInterface();
    if (fdmState) 
    {
        groundLevel = fdmState->get_groundlevel_m(position) * SG_METER_TO_FEET;
    }

    return groundLevel;
}

EntityManager::EntityManager(DIS::EntityStatePdu ownship)
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

    auto models = fgGetNode("/models");

    //
    // Ensure all the models have their property node names set and trigger the node manager
    // to load the models by creating a "load" node (then immediately removing it)
    //
    for (int modelIndex = 0; modelIndex < models->nChildren(); ++modelIndex)
    {
        const std::string propertyPath("/models/model" + (modelIndex == 0 ? "" : ("[" + std::to_string(modelIndex) + "]")));

        auto model = models->getChild("model", modelIndex, false);

        model->getNode("latitude-deg-prop", true)->setStringValue(propertyPath + "/latitude-deg");
        model->getNode("longitude-deg-prop", true)->setStringValue(propertyPath + "/longitude-deg");
        model->getNode("elevation-ft-prop", true)->setStringValue(propertyPath + "/elevation-ft");
        model->getNode("heading-deg-prop", true)->setStringValue(propertyPath + "/heading-deg");
        model->getNode("pitch-deg-prop", true)->setStringValue(propertyPath + "/pitch-deg");
        model->getNode("roll-deg-prop", true)->setStringValue(propertyPath + "/roll-deg");
        
        model->getNode("load", 1);
        model->removeChildren("load");
    }
}

EntityManager::~EntityManager()
{
}

bool EntityManager::ShouldIgnorePDU(const DIS::Pdu &packet)
{
    // TODO: Check exercise ID matches.
    return false;
}

void EntityManager::ProcessEntityStatePDU(const DIS::EntityStatePdu &packet)
{
    if (!ShouldIgnoreEntityStatePDU(packet))
    {
        HandleEntityStatePDU(packet);
    }
}

bool EntityManager::ShouldIgnoreEntityStatePDU(const DIS::EntityStatePdu& packet)
{
    bool shouldIgnore = true;
    if (!ShouldIgnorePDU(packet))
    {
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
        else 
        {
            SG_LOG(SG_IO, SG_ALERT, "Ignoring PDU with site: "
                << std::to_string(incomingEntityID.getSite())
                << ", application: "
                << std::to_string(incomingEntityID.getApplication())
            );
        }
    }

    return shouldIgnore;
}

void EntityManager::HandleEntityStatePDU(const DIS::EntityStatePdu& entityPDU)
{
    // Find the entity in the scene
    const auto i = m_entityMap.find(entityPDU.getEntityID());
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

void EntityManager::ProcessFirePDU(const DIS::FirePdu &firePDU)
{
    if (!ShouldIgnoreFirePDU(firePDU))
    {
        HandleFirePDU(firePDU);
    }
}

bool EntityManager::ShouldIgnoreFirePDU(const DIS::FirePdu &firePDU)
{
    return ShouldIgnorePDU(firePDU);
}

void EntityManager::HandleFirePDU(const DIS::FirePdu &firePDU)
{
    auto firingEntity = m_entityMap.find(firePDU.getFiringEntityID());
    if (firingEntity != m_entityMap.end())
    {
        // TODO: Add firing animation to 'firingEntity'
    }
}

void EntityManager::ProcessDetonationPDU(const DIS::DetonationPdu &detonationPDU)
{
    if (!ShouldIgnoreDetonationPDU(detonationPDU))
    {
        HandleDetonationPDU(detonationPDU);
    }
}

bool EntityManager::ShouldIgnoreDetonationPDU(const DIS::DetonationPdu &detonationPDU)
{
    return ShouldIgnoreDetonationPDU(detonationPDU);
}

void EntityManager::HandleDetonationPDU(const DIS::DetonationPdu &detonationPDU)
{
    auto targetEntity = m_entityMap.find(detonationPDU.getTargetEntityID());
    if (targetEntity != m_entityMap.end())
    {
        // TODO: Add detonation animation to 'targetEntity'
    }
}

void EntityManager::AddEntityToScene(const DIS::EntityStatePdu& entityPDU)
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
        m_entityMap.insert(std::make_pair(entityPDU.getEntityID(), *entity));
    }
}

void EntityManager::UpdateEntityInScene(Entity &entity, const DIS::EntityStatePdu& entityPDU)
{
    auto mmss = globals->get_subsystem("model-manager");
    auto mm = dynamic_cast<FGModelMgr*>(mmss);

    auto modelInstances = mm->getInstances();
    if (entity.m_modelIndex < modelInstances.size())
    {
        // Get the lat/lon/altitude from the PDU
        ECEF entityECEF(entityPDU.getEntityLocation());
        LLA entityLLA(entityECEF);

        // NOTE/HACK: If the altitude given is below the actual ground, adjust the altitude to put it on the ground.
        {
            auto position = SGGeod::fromDegFt(
                entityLLA.GetLongitude().inDegrees(), 
                entityLLA.GetLatitude().inDegrees(),
                entityLLA.GetAltitude().inFeet());
            auto groundLevelInFeet = GetGroundLevelInFeet(position);
            if (entityLLA.GetAltitude().inFeet() < groundLevelInFeet)
            {
                entityLLA.SetAltitude(Distance::fromFeet(groundLevelInFeet));
            }
        }

        // 
        // Calculate orientation (Euler angles from the NED frame) so Flight Gear can position it properly.
        //

        // Step 1 - Create a frame that represents the entity orientation in ECEF space.  It starts equal to the base ECEF frame.
        auto entityOrientation = Frame::fromECEFBase();

        // Step 2 - Rotate the entity orientation frame around the ECEF axes
        //          by the Euler angles stored in the incoming DIS PDU.  Also, record
        //          the intermediate from the rotation so it can be used later when
        //          determining the Euler angles.
        Frame intermediate = Frame::zero();
        entityOrientation.rotate(entityPDU.getEntityOrientation(), &intermediate);

        // Step 3 - Calculate the NED frame located at the entity's location.  This frame will be
        //          used as the reference frame when calculating the Euler angles relative to the
        //          entity's frame.
        const auto baseNED = Frame::fromLatLon(entityLLA.GetLatitude(), entityLLA.GetLongitude());

        // Step 4 = Calculate the Euler angles between the base NED and the rotated entity NED.
        const auto eulers = Frame::GetEulerAngles(baseNED, intermediate, entityOrientation);

        const double heading = Angle::fromRadians(eulers.getPsi()).inDegrees();
        const double pitch   = Angle::fromRadians(eulers.getTheta()).inDegrees();
      
#if 1   // TODO/HACK: Fix this 180 degree kludge.  (Due to NED pointing down instead of UP?)     
        const double roll    = 180 + Angle::fromRadians(eulers.getPhi()).inDegrees();
#endif

        // Set the values in the property system.
        const std::string propertyPath("/models/model" + (entity.m_modelIndex == 0 ? "" : ("[" + std::to_string(entity.m_modelIndex) + "]")));

        fgSetDouble(propertyPath + "/latitude-deg", entityLLA.GetLatitude().inDegrees());
        fgSetDouble(propertyPath + "/longitude-deg", entityLLA.GetLongitude().inDegrees());
        fgSetDouble(propertyPath + "/elevation-ft", entityLLA.GetAltitude().inFeet());

        fgSetDouble(propertyPath + "/heading-deg", heading);
        fgSetDouble(propertyPath + "/pitch-deg", pitch);
        fgSetDouble(propertyPath + "/roll-deg", roll);

#ifndef NDEBBUG
        SG_LOG(SG_IO, SG_BULK, "Location/Orientation: " 
            << std::to_string(entityPDU.getEntityID().getEntity())
            << ","
            << std::to_string(entityLLA.GetLatitude().inDegrees()) 
            << "," 
            << std::to_string(entityLLA.GetLongitude().inDegrees()) 
            << ",     " 
            << std::to_string(heading)
            << ","
            << std::to_string(pitch)
            << ","
            << std::to_string(roll)
        );
#endif        
    }
}

void EntityManager::RemoveExpiredEntities()
{
}

std::unique_ptr<EntityManager::Entity> EntityManager::CreateEntity(const DIS::EntityStatePdu& entityPDU, size_t modelIndex)
{
    return std::unique_ptr<Entity>(new Entity(entityPDU, modelIndex));
}

std::unique_ptr<EntityManager::Entity> EntityManager::CreateT72(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityManager::Entity> entity;
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

std::unique_ptr<EntityManager::Entity> EntityManager::CreateM1(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityManager::Entity> entity;
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

std::unique_ptr<EntityManager::Entity> EntityManager::CreateAH64(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityManager::Entity> entity;
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

std::unique_ptr<EntityManager::Entity> EntityManager::CreateUH60(const DIS::EntityStatePdu& entityPDU)
{
    std::unique_ptr<EntityManager::Entity> entity;
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

#ifndef NDEBUG
void EntityManager::PerformExtra()
{
#if 0 // TODO: Simple test logic, remove before shipping
    auto mmss = globals->get_subsystem("model-manager");
    auto mm = dynamic_cast<FGModelMgr*>(mmss);

    auto modelInstances = mm->getInstances();

    for (size_t modelIndex = 0; modelIndex < modelInstances.size(); ++modelIndex)
    {
        const std::string propertyPath("/models/model" + (modelIndex == 0 ? "" : ("[" + std::to_string(modelIndex) + "]")));
        auto specificPropertyPath = propertyPath + "/heading-deg";
        if (fgHasNode(specificPropertyPath))
        {
            auto value = fgGetDouble(specificPropertyPath);
            value += 5;
            fgSetDouble(specificPropertyPath, value);
        }
    }
#endif    
}
#endif // !NDEBUG
