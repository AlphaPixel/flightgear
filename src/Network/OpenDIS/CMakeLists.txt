set(SOURCES
    EntityManager.cxx
)

set(HEADERS
    EntityManager.hxx
)

flightgear_component(OpenDIS "${SOURCES}" "${HEADERS}")
