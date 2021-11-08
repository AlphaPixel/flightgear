set(SOURCES 
    opendis_manager.cpp
    opendis_subsystem.cpp
)

set(HEADERS
    opendis_manager.h
    opendis_subsystem.h
)

flightgear_component(OpenDIS "${SOURCES}" "${HEADERS}")
