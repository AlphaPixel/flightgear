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
#include <simgear/scene/model/placement.hxx>
#include <osg/Math>  // RadiansToDegrees()
#include <simgear/math/sg_geodesy.hxx>

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
}

EntityManager::~EntityManager()
{
}

bool EntityManager::ShouldIgnorePDU(const DIS::Pdu &packet)
{
    // TODO: Check exercise ID matches.
    return false;
}

void EntityManager::valueChanged(SGPropertyNode *node)
{
    // TODO: This is an O(n^2) operation when a single value is changed (it updates
    //       all models).   This is terribly inefficient.   This might warrant
    //       a change in model-manager to only update a single model.
    auto mmss = globals->get_subsystem("model-manager");
    auto mm = dynamic_cast<FGModelMgr*>(mmss);

    const double unused = 0.0;
    mm->update(unused); //Causes all models up update from the property tree.
}

void EntityManager::childAdded(SGPropertyNode *parent, SGPropertyNode *child)
{
}

void EntityManager::childRemoved(SGPropertyNode *parent, SGPropertyNode *child)
{
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

        const std::string propertyPath("/models/model" + (entity->m_modelIndex == 0 ? "" : ("[" + std::to_string(entity->m_modelIndex) + "]")));
        fgAddChangeListener(this, propertyPath);
    }
}

void EntityManager::UpdateEntityInScene(Entity &entity, const DIS::EntityStatePdu& entityPDU)
{
    auto mmss = globals->get_subsystem("model-manager");
    auto mm = dynamic_cast<FGModelMgr*>(mmss);

    auto modelInstances = mm->getInstances();
    if (entity.m_modelIndex < modelInstances.size())
    {
#if 0
        {
            SGVec3d N, E, D;
            CalculateNED(0, 0, N, E, D); // (0, 0, 1), (0, 1, 0), (-1, 0, 0)

            DIS::Orientation o1;
            o1.setPsi(osg::DegreesToRadians(90.0));
            o1.setTheta(0);
            o1.setPhi(0);
            
            SGVec3d Nr, Er, Dr;
            RotateFrame(o1, N, E, D, Nr, Er, Dr);   // (0, 0, 1), (-1, 0, 0), (0, -1, 0) // Psi = +90    

            double psi_radians, theta_radians, phi_radians;
//            CalculateEulerAnglesBetweenFrames(N, E, D, Nr, Er, Dr, psi_radians, theta_radians, phi_radians);
// Nope            CalculateEulerAnglesBetweenFrames(E, N, D, Er, Nr, Dr, psi_radians, theta_radians, phi_radians);
            CalculateEulerAnglesBetweenFrames(E, D, N, Er, Dr, Nr, psi_radians, theta_radians, phi_radians);
// Nope     CalculateEulerAnglesBetweenFrames(N, D, E, Nr, Dr, Er, psi_radians, theta_radians, phi_radians);

            double psi_degrees = osg::RadiansToDegrees(psi_radians);
            double theta_degrees = osg::RadiansToDegrees(theta_radians);
            double phi_degrees = osg::RadiansToDegrees(phi_radians);

// Theta +90    RotateFrame(o1, N, E, D, Nr, Er, Dr);   // (-1, 0, 0), (0, 1, 0), (0, 0, -1)
// Phi = +90    RotateFrame(o1, N, E, D, Nr, Er, Dr);   // (0, -1, 0), (0, 0, 1), (-1, 0, 0)

            DIS::Vector3Double XYZ;
            XYZ.setX(-3.93 * 10e6);
            XYZ.setY(3.48 * 10e6);
            XYZ.setZ(-3.63 * 10e6);

            // Unit test code for LLA->ECEF
            double adelaide_latitude_radians = osg::DegreesToRadians(-34.9);
            double adelaide_longitude_radians = osg::DegreesToRadians(138.5);

            CalculateNED(adelaide_latitude_radians, adelaide_longitude_radians, N, E, D);

            // NED should match figure 4.8

            DIS::Orientation o;
            o.setPsi(osg::DegreesToRadians(135.0));
            o.setTheta(osg::DegreesToRadians(20.0));
            o.setPhi(osg::DegreesToRadians(30.0));

            SGVec3d x3, y3, z3;
            //CalculateOrientedECEFAxes(o, x3, y3, z3); // Should match figure 4.26 - BUSTED
            RotateFrame(o, N, E, D, x3, y3, z3);

            // Unit test code for ECEF->LLA conversions
            //(X,Y,Z) = (−3.93, 3.48, −3.63) ×106 m.  (Adelaide AU)

            DIS::Orientation ptp;
            ptp.setPsi(osg::DegreesToRadians(-123.0));
            ptp.setTheta(osg::DegreesToRadians(47.8));
            ptp.setPhi(osg::DegreesToRadians(-29.7));  

            double latitude_radians, longitude_radians, altitude;
            ECEFtoLLA(XYZ, latitude_radians, longitude_radians, altitude);
            double latitude_deg = osg::RadiansToDegrees(latitude_radians);
            double longitude_deg = osg::RadiansToDegrees(longitude_radians);

            double heading_radians, pitch_radians, roll_radians;
            ECEFtoHPR(ptp, latitude_radians, longitude_radians, heading_radians, pitch_radians, roll_radians);
            double heading_deg = osg::RadiansToDegrees(heading_radians);
            double pitch_deg = osg::RadiansToDegrees(pitch_radians);
            double roll_deg = osg::RadiansToDegrees(roll_radians);

            heading_deg = 0.0;
        }
#endif



        // Set properties on entity model
//        auto modelInstance = modelInstances[entity.m_modelIndex];

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

        // Calculate orientation
        // Step 1 - Get the local "base" NED frame (in ECEF space) at the entity's lat/lon.

        const auto baseECEF = Frame::fromECEFBase();
        const auto baseNED = Frame::fromLatLon(entityLLA.GetLatitude(), entityLLA.GetLongitude());

        // Step 2 - Rotate the local NED frame (in ECEF) space around the ECEF axes
        //          by the Euler angles stored in the incoming DIS orientation.
        // auto entityNED = baseNED;
        // entityNED.rotate(entityPDU.getEntityOrientation());
        auto entityOrientationECEF = baseECEF;
        entityOrientationECEF.rotate(entityPDU.getEntityOrientation());

        // Step 3 - Calculate the quarternion that rotates 'baseNED' to 'NED'.
        // auto q = Frame::GetRotateTo(baseNED, entityNED);
        auto q = Frame::GetRotateTo(baseNED, entityOrientationECEF);

        // NOTE/HACK: we write to both the model and the property system.  This must be done because sometimes (UFO mode), based on the
        // FDM in use, the property system updates won't make it down into the model and other times (non-UFO mode) they will
        // and will overwrite what's written in the model.
        // auto model = modelInstance->model;
        // model->setPosition(position);

        // auto q = SGQuatd::fromEulerRad(psi, theta, phi);
        // model->setOrientation(q);
        // model->update();

        // Set the values in the property system.
        const std::string propertyPath("/models/model" + (entity.m_modelIndex == 0 ? "" : ("[" + std::to_string(entity.m_modelIndex) + "]")));

        fgSetDouble(propertyPath + "/latitude-deg", entityLLA.GetLatitude().inDegrees());
        fgSetDouble(propertyPath + "/longitude-deg", entityLLA.GetLongitude().inDegrees());
        fgSetDouble(propertyPath + "/elevation-ft", entityLLA.GetAltitude().inFeet() + 30);

        double heading, pitch, roll;
        q.getEulerDeg(heading, pitch, roll);

        fgSetDouble(propertyPath + "/heading-deg", heading);
        //fgSetDouble(propertyPath + "/pitch-deg", pitch);
        //fgSetDouble(propertyPath + "/roll-deg", roll);

        SG_LOG(SG_IO, SG_ALERT, "Location/Orientation: " 
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
        auto specificPropertyPath = propertyPath + "/surface-positions/turret-pos-deg";
        if (fgHasNode(specificPropertyPath))
        {
            auto value = fgGetDouble(specificPropertyPath);
            value += 5;
            fgSetDouble(specificPropertyPath, value);
        }
#if 0
        if (value != 45)
        {
            fgSetDouble(specificPropertyPath, 45);
        }

        specificPropertyPath = propertyPath + "/pitch-deg";
        value = fgGetDouble(specificPropertyPath);
        value += 1;
        fgSetDouble(specificPropertyPath, value);

        specificPropertyPath = propertyPath + "/roll-deg";
        value = fgGetDouble(specificPropertyPath);
        value += 2;
        fgSetDouble(specificPropertyPath, value);
#endif        
    }
#endif    
}
#endif // !NDEBUG
