// opendis_manager.h
//
// Copyright (C) 2021 - AlphaPixel (http://www.alphapixel.com)
#include <memory>
#include <FDM/flightProperties.hxx>

class OpenDISManager
{
public:
    static std::unique_ptr<OpenDISManager> create();
    virtual ~OpenDISManager();

    void update(double delta_time_sec);

private:
    OpenDISManager();

    // m_flightProperties are used to retrieve all "ownship" properties for emitting via OpenDIS.
    std::unique_ptr<FlightProperties> m_flightProperties;
};
