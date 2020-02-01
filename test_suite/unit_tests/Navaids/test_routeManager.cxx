#include "test_routeManager.hxx"

#include <memory>
#include <cstring>

#include "test_suite/FGTestApi/testGlobals.hxx"
#include "test_suite/FGTestApi/NavDataCache.hxx"
#include "test_suite/FGTestApi/TestPilot.hxx"

#include <Navaids/FlightPlan.hxx>
#include <Navaids/NavDataCache.hxx>
#include <Navaids/navrecord.hxx>
#include <Navaids/navlist.hxx>

// we need a default GPS instrument, hard to test seperately for now
#include <Instrumentation/gps.hxx>

#include <Autopilot/route_mgr.hxx>

using namespace flightgear;

static FlightPlanRef makeTestFP(const std::string& depICAO, const std::string& depRunway,
                         const std::string& destICAO, const std::string& destRunway,
                         const std::string& waypoints)
{
    FlightPlanRef f = new FlightPlan;
    FGTestApi::setUp::populateFPWithNasal(f, depICAO, depRunway, destICAO, destRunway, waypoints);
    return f;
}


// Set up function for each test.
void RouteManagerTests::setUp()
{
    FGTestApi::setUp::initTestGlobals("routemanager");
    FGTestApi::setUp::initNavDataCache();
    
    globals->add_new_subsystem<FGRouteMgr>();
    
// setup the default GPS, which is needed for waypoint
// sequencing to work
    SGPropertyNode_ptr configNode(new SGPropertyNode);
    configNode->setStringValue("name", "gps");
    configNode->setIntValue("number", 0);
    GPS* gps(new GPS(configNode, true /* default mode */));
    m_gps = gps;
    
    SGPropertyNode_ptr node = globals->get_props()->getNode("instrumentation", true)->getChild("gps", 0, true);
  //  node->setBoolValue("serviceable", true);
   // globals->get_props()->setDoubleValue("systems/electrical/outputs/gps", 6.0);
    globals->add_subsystem("gps", gps, SGSubsystemMgr::POST_FDM);
    
    globals->get_subsystem_mgr()->bind();
    globals->get_subsystem_mgr()->init();
    
    FGTestApi::setUp::initStandardNasal();
    globals->get_subsystem_mgr()->postinit();
}


// Clean up after each test.
void RouteManagerTests::tearDown()
{
    m_gps = nullptr;
    FGTestApi::tearDown::shutdownTestGlobals();
}

 void RouteManagerTests::setPositionAndStabilise(const SGGeod& g)
 {
     FGTestApi::setPosition(g);
     for (int i=0; i<60; ++i) {
         globals->get_subsystem_mgr()->update(0.02);
     }
 }

void RouteManagerTests::testBasic()
{
    //FGTestApi::setUp::logPositionToKML("rm_basic");
    
    FlightPlanRef fp1 = makeTestFP("EGLC", "27", "EHAM", "06",
                                   "CLN IDESI RINIS VALKO RIVER RTM EKROS");
    fp1->setIdent("testplan");
    fp1->setCruiseFlightLevel(360);
    
    auto rm = globals->get_subsystem<FGRouteMgr>();
    rm->setFlightPlan(fp1);
    
    auto gpsNode = globals->get_props()->getNode("instrumentation/gps", true);
    CPPUNIT_ASSERT(!strcmp("obs", gpsNode->getStringValue("mode")));

    rm->activate();
    
    CPPUNIT_ASSERT(fp1->isActive());
    
    // Nasal deleagte should have placed GPS into leg mode
    auto rmNode = globals->get_props()->getNode("autopilot/route-manager", true);
    
    CPPUNIT_ASSERT(!strcmp("leg", gpsNode->getStringValue("mode")));

    CPPUNIT_ASSERT(!strcmp("EGLC", rmNode->getStringValue("departure/airport")));
    CPPUNIT_ASSERT(!strcmp("27", rmNode->getStringValue("departure/runway")));
    CPPUNIT_ASSERT(!strcmp("", rmNode->getStringValue("departure/sid")));
    CPPUNIT_ASSERT(!strcmp("London City", rmNode->getStringValue("departure/name")));

    CPPUNIT_ASSERT(!strcmp("EHAM", rmNode->getStringValue("destination/airport")));
    CPPUNIT_ASSERT(!strcmp("06", rmNode->getStringValue("destination/runway")));
    
    CPPUNIT_ASSERT_EQUAL(360, rmNode->getIntValue("cruise/flight-level"));
    CPPUNIT_ASSERT_EQUAL(false, rmNode->getBoolValue("airborne"));

    CPPUNIT_ASSERT_EQUAL(0, rmNode->getIntValue("current-wp"));
    auto wp0Node = rmNode->getNode("wp");
    CPPUNIT_ASSERT(!strcmp("EGLC-27", wp0Node->getStringValue("id")));
    
    auto wp1Node = rmNode->getNode("wp[1]");
    CPPUNIT_ASSERT(!strcmp("CLN", wp1Node->getStringValue("id")));

    FGPositioned::TypeFilter f{FGPositioned::VOR};
    auto clactonVOR = fgpositioned_cast<FGNavRecord>(FGPositioned::findClosestWithIdent("CLN", SGGeod::fromDeg(0.0, 51.0), &f));
    
    // verify hold entry course
    auto pilot = SGSharedPtr<FGTestApi::TestPilot>(new FGTestApi::TestPilot);
    FGTestApi::setPosition(fp1->departureRunway()->geod());
    pilot->resetAtPosition(fp1->departureRunway()->geod());
    
    pilot->setSpeedKts(220);
    pilot->setCourseTrue(fp1->departureRunway()->headingDeg());
    pilot->setTargetAltitudeFtMSL(10000);
    
    bool ok = FGTestApi::runForTimeWithCheck(30.0, [rmNode] () {
        if (rmNode->getIntValue("current-wp") == 1) return true;
        return false;
    });
    
    CPPUNIT_ASSERT(ok);
    
    // continue outbound for some time
    pilot->setSpeedKts(250);
    FGTestApi::runForTime(30.0);
    
    // turn towards Clacton VOR
    pilot->flyDirectTo(clactonVOR->geod());
    pilot->setTargetAltitudeFtMSL(30000);
    pilot->setSpeedKts(280);
    ok = FGTestApi::runForTimeWithCheck(6000.0, [rmNode] () {
        if (rmNode->getIntValue("current-wp") == 2) return true;
        return false;
    });
    CPPUNIT_ASSERT(ok);

    // short straight leg for testing
    pilot->flyHeading(90.0);
    pilot->setSpeedKts(330);
    FGTestApi::runForTime(30.0);

    // let's engage LNAV mode :)
    pilot->flyGPSCourse(m_gps);
    
    ok = FGTestApi::runForTimeWithCheck(6000.0, [rmNode] () {
        if (rmNode->getIntValue("current-wp") == 5) return true;
        return false;
    });
    CPPUNIT_ASSERT(ok);
    
    // check where we are - should be heading to RIVER from VALKO
    CPPUNIT_ASSERT_EQUAL(5, rmNode->getIntValue("current-wp"));
    CPPUNIT_ASSERT(!strcmp("RIVER", wp0Node->getStringValue("id")));
    CPPUNIT_ASSERT(!strcmp("RTM", wp1Node->getStringValue("id")));
    // slightly rapid descent 
    pilot->setTargetAltitudeFtMSL(3000);

    ok = FGTestApi::runForTimeWithCheck(6000.0, [rmNode] () {
        if (rmNode->getIntValue("current-wp") == 7) return true;
        return false;
    });
    CPPUNIT_ASSERT(ok);
    
    // run until the GPS reverts to OBS mode at the end of the flight plan
    ok = FGTestApi::runForTimeWithCheck(6000.0, [gpsNode] () {
        std::string mode = gpsNode->getStringValue("mode");
        if (mode == "obs") return true;
        return false;
    });
    
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(-1, fp1->currentIndex());
    CPPUNIT_ASSERT(!fp1->isActive());
    CPPUNIT_ASSERT_EQUAL(-1, rmNode->getIntValue("current-wp"));
    CPPUNIT_ASSERT_EQUAL(false, rmNode->getBoolValue("active"));
}

void RouteManagerTests::testDefaultSID()
{
    FlightPlanRef fp1 = makeTestFP("EGLC", "27", "EHAM", "24",
                                   "CLN IDRID VALKO");
    fp1->setIdent("testplan");
    
    auto rm = globals->get_subsystem<FGRouteMgr>();
    rm->setFlightPlan(fp1);
    
    auto rmNode = globals->get_props()->getNode("autopilot/route-manager", true);
    rmNode->setStringValue("departure/sid", "DEFAULT");
    
    // let's see what we got :)
    
    rm->activate();
    
    CPPUNIT_ASSERT(fp1->isActive());
}

void RouteManagerTests::testDirectToLegOnFlightplanAndResume()
{
 //   FGTestApi::setUp::logPositionToKML("rm_dto_resume_leg");

    // this is very similar to the identiucally name dtest in GPSTests, but relies on the Nasal
    // route manager delegate to perform the same task
    FlightPlanRef fp1 = makeTestFP("EBBR", "07L", "EGGD", "27",
                                   "NIK COA DVR TAWNY WOD");
    auto rm = globals->get_subsystem<FGRouteMgr>();
    rm->setFlightPlan(fp1);
   // FGTestApi::writeFlightPlanToKML(fp1);

    auto gpsNode = globals->get_props()->getNode("instrumentation/gps", true);
    auto rmNode = globals->get_props()->getNode("autopilot/route-manager", true);

    CPPUNIT_ASSERT(!strcmp("obs", gpsNode->getStringValue("mode")));
    rm->activate();
    
    CPPUNIT_ASSERT(fp1->isActive());

    FGTestApi::setPosition(fp1->departureRunway()->pointOnCenterline(0.0));
    FGTestApi::runForTime(10.0); // let the GPS stabilize

    CPPUNIT_ASSERT_EQUAL(std::string{"leg"}, std::string{gpsNode->getStringValue("mode")});
    CPPUNIT_ASSERT_EQUAL(std::string{"EBBR-07L"}, std::string{gpsNode->getStringValue("wp/wp[1]/ID")});
    
    CPPUNIT_ASSERT_EQUAL(0, rmNode->getIntValue("current-wp"));
    auto wp0Node = rmNode->getNode("wp");
    CPPUNIT_ASSERT(!strcmp("EBBR-07L", wp0Node->getStringValue("id")));
    
    auto wp1Node = rmNode->getNode("wp[1]");
    CPPUNIT_ASSERT(!strcmp("NIK", wp1Node->getStringValue("id")));

    // initiate a direct to
    SGGeod p2 = fp1->departureRunway()->pointOnCenterline(5.0* SG_NM_TO_METER);
    FGTestApi::setPosition(p2);

    auto doverVOR = fp1->legAtIndex(3)->waypoint()->source();
    
    double distanceToDover = SGGeodesy::distanceNm(p2, doverVOR->geod());
    double bearingToDover = SGGeodesy::courseDeg(p2, doverVOR->geod());
    
    CPPUNIT_ASSERT_EQUAL(std::string{"DVR"}, doverVOR->ident());
    gpsNode->setStringValue("scratch/ident", "DVR");
    gpsNode->setDoubleValue("scratch/longitude-deg", doverVOR->geod().getLongitudeDeg());
    gpsNode->setDoubleValue("scratch/latitude-deg", doverVOR->geod().getLatitudeDeg());
    gpsNode->setStringValue("command", "direct");
    CPPUNIT_ASSERT_EQUAL(std::string{"dto"}, std::string{gpsNode->getStringValue("mode")});
    
    // check that upon reaching DOVER, we sequence to TAWNY and resume leg mode
    // this is handled by the default delegate in Nasal
    
    SGGeod posNearDover = SGGeodesy::direct(p2, bearingToDover, (distanceToDover - 8.0) * SG_NM_TO_METER);
    FGTestApi::setPosition(posNearDover);

    auto pilot = SGSharedPtr<FGTestApi::TestPilot>(new FGTestApi::TestPilot);
    pilot->resetAtPosition(posNearDover);
    pilot->setSpeedKts(250);
    pilot->flyGPSCourse(m_gps);
    
    bool ok = FGTestApi::runForTimeWithCheck(180.0, [fp1] () {
        if (fp1->currentIndex() == 4) {
            return true;
        }
        return false;
    });
    
    CPPUNIT_ASSERT(ok);
    CPPUNIT_ASSERT_EQUAL(std::string{"leg"}, std::string{gpsNode->getStringValue("mode")});
}

void RouteManagerTests::testDefaultApproach()
{
    
}

void RouteManagerTests::testHoldFromNasal()
{
   // FGTestApi::setUp::logPositionToKML("rm_hold_from_nasal");

    // test that Nasal can set a hold-count (implicitly converting a leg
    // to a Hold waypt), configure the hold radial, and exit the hold
    
    FlightPlanRef fp1 = makeTestFP("NZCH", "02", "NZAA", "05L",
                                   "ALADA NS WB WN MAMOD KAPTI OH");
    fp1->setIdent("testplan");
    fp1->setCruiseFlightLevel(360);
    
    auto rm = globals->get_subsystem<FGRouteMgr>();
    rm->setFlightPlan(fp1);
  //  FGTestApi::writeFlightPlanToKML(fp1);

    auto gpsNode = globals->get_props()->getNode("instrumentation/gps", true);
    CPPUNIT_ASSERT(!strcmp("obs", gpsNode->getStringValue("mode")));
    
    rm->activate();
    
    CPPUNIT_ASSERT(fp1->isActive());
    CPPUNIT_ASSERT(!strcmp("leg", gpsNode->getStringValue("mode")));

    SGGeod posEnrouteToWB = fp1->pointAlongRoute(3, -10.0);
    FGTestApi::setPositionAndStabilise(posEnrouteToWB);
    
    // sequence everything to the correct wp
    fp1->setCurrentIndex(3);

    // setup some hold data from Nasal. To make it more challenging,
    // do it once the wp is already active
    bool ok = FGTestApi::executeNasal(R"(
        var fp = flightplan(); # retrieve the global flightplan
        var leg = fp.getWP(3);
        leg.hold_count = 4;
        leg.hold_heading_radial_deg = 310;
    )");
    CPPUNIT_ASSERT(ok);

    // check the value updated in the leg
    CPPUNIT_ASSERT_EQUAL(4, fp1->legAtIndex(3)->holdCount());

    // check we converted to a hold
    auto wp = fp1->legAtIndex(3)->waypoint();
    auto holdWpt = static_cast<flightgear::Hold*>(wp);

    CPPUNIT_ASSERT_EQUAL(wp->type(), std::string{"hold"});
    CPPUNIT_ASSERT_DOUBLES_EQUAL(310.0, holdWpt->headingRadialDeg(), 0.5);
    
    // establish the test pilot at this position too
    auto pilot = SGSharedPtr<FGTestApi::TestPilot>(new FGTestApi::TestPilot);
    pilot->resetAtPosition(posEnrouteToWB);
    pilot->setSpeedKts(250);
    pilot->flyGPSCourse(m_gps);
    pilot->setCourseTrue(gpsNode->getDoubleValue("wp/leg-true-course-deg"));
    
    // run for a bit to stabilize everything
    FGTestApi::runForTime(5.0);
    
    // check we upgraded to a hold controller internally, and are flying to it :)
    auto statusNode = gpsNode->getNode("rnav-controller-status");
    CPPUNIT_ASSERT_EQUAL(std::string{"leg-to-hold"}, std::string{statusNode->getStringValue()});
    
    // check we're on the leg
    
    auto wbPos = fp1->legAtIndex(3)->waypoint()->position();
    auto nsPos = fp1->legAtIndex(2)->waypoint()->position();
    const double crsToWB = SGGeodesy::courseDeg(globals->get_aircraft_position(), wbPos);
    const double crsNSWB = SGGeodesy::courseDeg(nsPos,wbPos);
    
    CPPUNIT_ASSERT_DOUBLES_EQUAL(crsToWB, gpsNode->getDoubleValue("wp/wp[1]/bearing-true-deg"), 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(crsNSWB, gpsNode->getDoubleValue("wp/leg-true-course-deg"), 0.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, gpsNode->getDoubleValue("wp/wp[1]/course-error-nm"), 0.05);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, gpsNode->getDoubleValue("wp/wp[1]/course-deviation-deg"), 0.5);
    
    // fly into the hold, should be teardrop entry
    ok = FGTestApi::runForTimeWithCheck(600.0, [statusNode] () {
        std::string s = statusNode->getStringValue();
        if (s == "entry-teardrop") return true;
        return false;
    });
    
    CPPUNIT_ASSERT(ok);
    
    ok = FGTestApi::runForTimeWithCheck(600.0, [statusNode] () {
        std::string s = statusNode->getStringValue();
        if (s == "hold-inbound") return true;
        return false;
    });
    
    ok = FGTestApi::runForTimeWithCheck(600.0, [statusNode] () {
        std::string s = statusNode->getStringValue();
        if (s == "hold-outbound") return true;
        return false;
    });
    CPPUNIT_ASSERT(ok);

    // half way through the outbound turn
    FGTestApi::runForTime(30.0);

    ok = FGTestApi::executeNasal(R"(
                                 setprop("/instrumentation/gps/command", "exit-hold");
                                )");
    CPPUNIT_ASSERT(ok);

    // no change yet
    CPPUNIT_ASSERT_EQUAL(std::string{"hold-outbound"}, std::string{statusNode->getStringValue()});
    
    // then we fly inbound
    ok = FGTestApi::runForTimeWithCheck(600.0, [statusNode] () {
        std::string s = statusNode->getStringValue();
        if (s == "hold-inbound") return true;
        return false;
    });
    CPPUNIT_ASSERT(ok);

    // and then we exit
    ok = FGTestApi::runForTimeWithCheck(600.0, [fp1] () {
        if (fp1->currentIndex() == 4) return true;
        return false;
    });
    CPPUNIT_ASSERT(ok);
    
    // get back on course
    FGTestApi::runForTime(60.0);
}
