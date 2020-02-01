/*
 * Copyright (C) 2019 James Turner
 *
 * This file is part of the program FlightGear.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_rnav_procedures.hxx"

#include <memory>
#include <cstring>

#include "test_suite/FGTestApi/testGlobals.hxx"
#include "test_suite/FGTestApi/NavDataCache.hxx"
#include "test_suite/FGTestApi/TestPilot.hxx"

#include <simgear/structure/exception.hxx>

#include <Airports/airport.hxx>
#include <Navaids/NavDataCache.hxx>
#include <Navaids/navrecord.hxx>
#include <Navaids/navlist.hxx>
#include <Navaids/FlightPlan.hxx>

#include <Instrumentation/gps.hxx>

#include <Autopilot/route_mgr.hxx>

using namespace flightgear;

/////////////////////////////////////////////////////////////////////////////

namespace {
class TestFPDelegate : public FlightPlan::Delegate
{
public:
    FlightPlanRef thePlan;
    int sequenceCount = 0;

    virtual ~TestFPDelegate()
    {
    }
    
    void sequence() override
    {
        
        ++sequenceCount;
        int newIndex = thePlan->currentIndex() + 1;
        if (newIndex >= thePlan->numLegs()) {
            thePlan->finish();
            return;
        }
        
        thePlan->setCurrentIndex(newIndex);
    }
    
    void currentWaypointChanged() override
    {
    }
    
    virtual void departureChanged() override
    {
        // mimic the default delegate, inserting the SID waypoints
        
        // clear anything existing
        thePlan->clearWayptsWithFlag(WPT_DEPARTURE);
        
        // insert waypt for the dpearture runway
        auto dr = new RunwayWaypt(thePlan->departureRunway(), thePlan);
        dr->setFlag(WPT_DEPARTURE);
        thePlan->insertWayptAtIndex(dr, 0);
        
        if (thePlan->sid()) {
            WayptVec sidRoute;
            bool ok = thePlan->sid()->route(thePlan->departureRunway(), nullptr, sidRoute);
            if (!ok)
                throw sg_exception("failed to route via SID");
            int insertIndex = 1;
            for (auto w : sidRoute) {
                thePlan->insertWayptAtIndex(w, insertIndex++);
            }
        }
    }
};

} // of anonymous namespace

/////////////////////////////////////////////////////////////////////////////

// Set up function for each test.
void RNAVProcedureTests::setUp()
{
    FGTestApi::setUp::initTestGlobals("rnav-procedures");
    FGTestApi::setUp::initNavDataCache();
    
    SGPath proceduresPath = SGPath::fromEnv("FG_PROCEDURES_PATH");
    if (proceduresPath.exists()) {
        globals->append_fg_scenery(proceduresPath);
    }
    
    setupRouteManager();
}

// Clean up after each test.
void RNAVProcedureTests::tearDown()
{
    FGTestApi::tearDown::shutdownTestGlobals();
}

GPS* RNAVProcedureTests::setupStandardGPS(SGPropertyNode_ptr config,
                                const std::string name, const int index)
{
    SGPropertyNode_ptr configNode(config.valid() ? config
                                                 : SGPropertyNode_ptr{new SGPropertyNode});
    configNode->setStringValue("name", name);
    configNode->setIntValue("number", index);
    
    GPS* gps(new GPS(configNode));
    m_gps = gps;
    
    m_gpsNode = globals->get_props()->getNode("instrumentation", true)->getChild(name, index, true);
    m_gpsNode->setBoolValue("serviceable", true);
    globals->get_props()->setDoubleValue("systems/electrical/outputs/gps", 6.0);
    
    gps->bind();
    gps->init();
    
    globals->add_subsystem("gps", gps, SGSubsystemMgr::POST_FDM);
    return gps;
}

void RNAVProcedureTests::setupRouteManager()
{
    auto rm = globals->add_new_subsystem<FGRouteMgr>();
    rm->bind();
    rm->init();
    rm->postinit();
}

/////////////////////////////////////////////////////////////////////////////

#if 0
void RNAVProcedureTests::testBasic()
{
    setupStandardGPS();
    
    FGPositioned::TypeFilter f{FGPositioned::VOR};
    auto bodrumVOR = fgpositioned_cast<FGNavRecord>(FGPositioned::findClosestWithIdent("BDR", SGGeod::fromDeg(27.6, 37), &f));
    SGGeod p1 = SGGeodesy::direct(bodrumVOR->geod(), 45.0, 5.0 * SG_NM_TO_METER);
    
    FGTestApi::setPositionAndStabilise(p1);
    

}
#endif


void RNAVProcedureTests::testEGPH_TLA6C()
{
    
    auto egph = FGAirport::findByIdent("EGPH");
    
    auto sid = egph->findSIDWithIdent("TLA6C");
    // procedures not loaded, abandon test
    if (!sid)
        return;
    
    FGTestApi::setUp::logPositionToKML("procedure_egph_tla6c");

    auto rm = globals->get_subsystem<FGRouteMgr>();
    auto fp = new FlightPlan;
    
    auto testDelegate = new TestFPDelegate;
    testDelegate->thePlan = fp;
    fp->addDelegate(testDelegate);
    
    rm->setFlightPlan(fp);
    FGTestApi::setUp::populateFPWithoutNasal(fp, "EGPH", "24", "EGLL", "27R", "DCS POL DTY");
    
    fp->setSID(sid);
    
    FGRunwayRef departureRunway = fp->departureRunway();
    CPPUNIT_ASSERT_EQUAL(std::string{"24"}, fp->legAtIndex(0)->waypoint()->source()->name());
    
    CPPUNIT_ASSERT_EQUAL(std::string{"UW"}, fp->legAtIndex(1)->waypoint()->ident());
    
    auto d242Wpt =  fp->legAtIndex(2)->waypoint();
    CPPUNIT_ASSERT_EQUAL(std::string{"D242H"}, d242Wpt->ident());
    CPPUNIT_ASSERT_EQUAL(true, d242Wpt->flag(WPT_OVERFLIGHT));
    
    CPPUNIT_ASSERT_EQUAL(std::string{"D346T"}, fp->legAtIndex(3)->waypoint()->ident());
    
    FGTestApi::writeFlightPlanToKML(fp);

    CPPUNIT_ASSERT(rm->activate());
   
    
    setupStandardGPS();
    
    FGTestApi::setPositionAndStabilise(departureRunway->threshold());
    m_gpsNode->setStringValue("command", "leg");

    auto pilot = SGSharedPtr<FGTestApi::TestPilot>(new FGTestApi::TestPilot);
    pilot->resetAtPosition(globals->get_aircraft_position());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(departureRunway->headingDeg(), m_gpsNode->getDoubleValue("wp/leg-true-course-deg"), 0.5);
    pilot->setCourseTrue(m_gpsNode->getDoubleValue("wp/leg-true-course-deg"));
    pilot->setSpeedKts(220);
    pilot->flyGPSCourse(m_gps);
    
    FGTestApi::runForTime(20.0);
    // check we're somewhere along the runway, on the centerline
    // and still on waypoint zero
    
    bool ok = FGTestApi::runForTimeWithCheck(180.0, [fp] () {
        if (fp->currentIndex() == 1) {
            return true;
        }
        return false;
    });
    CPPUNIT_ASSERT(ok);
    
    // check what we sequenced to
    double elapsed = globals->get_sim_time_sec();
    ok = FGTestApi::runForTimeWithCheck(180.0, [fp] () {
        if (fp->currentIndex() == 2) {
            return true;
        }
        return false;
    });
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, m_gpsNode->getDoubleValue("wp/wp[1]/course-error-nm"), 0.05);

    elapsed = globals->get_sim_time_sec();
    
    ok = FGTestApi::runForTimeWithCheck(180.0, [fp] () {
        if (fp->currentIndex() == 3) {
            return true;
        }
        return false;
    });
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, m_gpsNode->getDoubleValue("wp/wp[1]/course-error-nm"), 0.05);

    elapsed = globals->get_sim_time_sec();
    
    ok = FGTestApi::runForTimeWithCheck(180.0, [fp] () {
        if (fp->currentIndex() == 4) {
            return true;
        }
        return false;
    });
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, m_gpsNode->getDoubleValue("wp/wp[1]/course-error-nm"), 0.05);

    ok = FGTestApi::runForTimeWithCheck(180.0, [fp] () {
        if (fp->currentIndex() == 5) {
            return true;
        }
        return false;
    });
    
    CPPUNIT_ASSERT(ok);

    CPPUNIT_ASSERT_EQUAL(std::string{"TLA"}, fp->legAtIndex(5)->waypoint()->ident());
    CPPUNIT_ASSERT_EQUAL(std::string{"TLA"}, std::string{m_gpsNode->getStringValue("wp/wp[1]/ID")});
}
