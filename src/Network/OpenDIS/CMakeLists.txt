set(SOURCES 
    opendis_connection.cpp
)

set(HEADERS
    opendis_connection.h
)

flightgear_component(OpenDIS "${SOURCES}" "${HEADERS}")
