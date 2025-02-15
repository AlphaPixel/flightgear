// FGAIAircraft - AIBase derived class creates an AI aircraft
//
// Written by David Culp, started October 2003.
//
// Copyright (C) 2003  David P. Culp - davidculp2@comcast.net
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef _FG_AIAircraft_HXX
#define _FG_AIAircraft_HXX

#include "AIBaseAircraft.hxx"

#include <string>
#include <iostream>

class PerformanceData;
class FGAISchedule;
class FGAIFlightPlan;
class FGATCController;
class FGATCInstruction;
class FGAIWaypoint;

class FGAIAircraft : public FGAIBaseAircraft {

public:
    FGAIAircraft(FGAISchedule *ref=0);
    ~FGAIAircraft();

    void readFromScenario(SGPropertyNode* scFileNode) override;

    void bind() override;
    void update(double dt) override;
    void unbind() override;

    void setPerformance(const std::string& acType, const std::string& perfString);

    void setFlightPlan(const std::string& fp, bool repat = false);
        
    void initializeFlightPlan();
    FGAIFlightPlan* GetFlightPlan() const { return fp.get(); };
    void ProcessFlightPlan( double dt, time_t now );
    time_t checkForArrivalTime(const std::string& wptName);
    
    void AccelTo(double speed);
    void PitchTo(double angle);
    void RollTo(double angle);
    void YawTo(double angle);
    void ClimbTo(double altitude);
    void TurnTo(double heading);
    
    void getGroundElev(double dt); //TODO these 3 really need to be public?
    void doGroundAltitude();
    bool loadNextLeg  (double dist=0);
    void resetPositionFromFlightPlan();
    double getBearing(double crse);

    void setAcType(const std::string& ac) { acType = ac; };
    const std::string& getAcType() const { return acType; }

    const std::string& getCompany() const { return company; }
    void setCompany(const std::string& comp) { company = comp;};

    void announcePositionToController(); //TODO have to be public?
    void processATC(const FGATCInstruction& instruction);
    void setTaxiClearanceRequest(bool arg) { needsTaxiClearance = arg; };
    bool getTaxiClearanceRequest() { return needsTaxiClearance; };
    FGAISchedule * getTrafficRef() { return trafficRef; };
    void setTrafficRef(FGAISchedule *ref) { trafficRef = ref; };
    void resetTakeOffStatus() { takeOffStatus = 0;};
    void setTakeOffStatus(int status) { takeOffStatus = status; };
    void scheduleForATCTowerDepartureControl(int state);

    const char* getTypeString(void) const override { return "aircraft"; }

    const std::string& GetTransponderCode() { return transponderCode; };
    void SetTransponderCode(const std::string& tc) { transponderCode = tc;};

    // included as performance data needs them, who else?
    inline PerformanceData* getPerformance() { return _performance; };
    inline bool onGround() const { return no_roll; };
    inline double getSpeed() const { return speed; };
    inline double getRoll() const { return roll; };
    inline double getPitch() const { return pitch; };
    inline double getAltitude() const { return altitude_ft; };
    inline double getVerticalSpeedFPM() const { return vs_fps * 60; };
    inline double altitudeAGL() const { return props->getFloatValue("position/altitude-agl-ft");};
    inline double airspeed() const { return props->getFloatValue("velocities/airspeed-kt");};
    const std::string& atGate();
    std::string acwakecategory;
    
    int getTakeOffStatus() { return takeOffStatus; };

    void checkTcas();
    double calcVerticalSpeed(double vert_ft, double dist_m, double speed, double error);

    FGATCController * getATCController() { return controller; };
    
    void clearATCController();
    void dumpCSVHeader(std::ofstream& o);
    void dumpCSV(std::ofstream& o, int lineIndex);
protected:
    void Run(double dt);

private:
    FGAISchedule *trafficRef;
    FGATCController *controller,
                    *prevController,
                    *towerController; // Only needed to make a pre-announcement

    bool hdg_lock;
    bool alt_lock;
    double dt_count;
    double dt_elev_count;
    double headingChangeRate;
    double headingError;
    double minBearing;
    double speedFraction;
    /**Zero if FP is not active*/
    double groundTargetSpeed;
    double groundOffset;

    bool use_perf_vs;
    SGPropertyNode_ptr refuel_node;
    SGPropertyNode_ptr tcasThreatNode;
    SGPropertyNode_ptr tcasRANode;
    
    // helpers for Run
    //TODO sort out which ones are better protected virtuals to allow
    //subclasses to override specific behaviour
    bool fpExecutable(time_t now);
    void handleFirstWaypoint(void);
    bool leadPointReached(FGAIWaypoint* curr, FGAIWaypoint* next, int nextTurnAngle);
    bool handleAirportEndPoints(FGAIWaypoint* prev, time_t now);
    bool reachedEndOfCruise(double&);
    bool aiTrafficVisible(void);
    void controlHeading(FGAIWaypoint* curr);
    void controlSpeed(FGAIWaypoint* curr,
                      FGAIWaypoint* next);
    
    void updatePrimaryTargetValues(double dt, bool& flightplanActive, bool& aiOutOfSight);
    
    void updateSecondaryTargetValues(double dt);
    void updateHeading(double dt);
    void updateBankAngleTarget();
    void updateVerticalSpeedTarget(double dt);
    void updatePitchAngleTarget();
    void updateActualState(double dt);
    void updateModelProperties(double dt);
    void handleATCRequests(double dt);
    inline bool isStationary() { return ((fabs(speed)<=0.0001)&&(fabs(tgt_speed)<=0.0001));}
    inline bool needGroundElevation() { if (!isStationary()) _needsGroundElevation=true;return _needsGroundElevation;}

    double sign(double x);
    std::string getTimeString(int timeOffset);

    void lazyInitControlsNodes();

    std::string acType;
    std::string company;
    std::string transponderCode;

    int spinCounter;
    /**Kills a flight when it's stuck */
    const int AI_STUCK_LIMIT = 100;
    int stuckCounter; 
    /**
     * Signals a reset to leg 1 at a different airport. 
     * The leg loading happens at a different place than the parking loading.
     * */
    bool repositioned;
    double prevSpeed;
    double prev_dist_to_go;

    bool holdPos;

    const char * _getTransponderCode() const;

    bool needsTaxiClearance;
    bool _needsGroundElevation;
    int  takeOffStatus; // 1 = joined departure queue; 2 = Passed DepartureHold waypoint; handover control to tower; 0 = any other state. 
    time_t timeElapsed;

    PerformanceData* _performance; // the performance data for this aircraft
    
   void assertSpeed(double speed);

   struct
   {
       double remainingLength;
       std::string startWptName;
       std::string finalWptName;
   } trackCache;

   // these are init-ed on first se by lazyInitControlsNodes()
   SGPropertyNode_ptr _controlsLateralModeNode,
       _controlsVerticalModeNode,
       _controlsTargetHeadingNode,
       _controlsTargetRollNode,
       _controlsTargetAltitude,
       _controlsTargetPitch,
       _controlsTargetSpeed;
};

#endif  // _FG_AIAircraft_HXX
