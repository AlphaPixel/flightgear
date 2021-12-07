set(SOURCES
    CoordinateSystems.cxx
    EntityManager.cxx
    Frame.cxx
)

set(HEADERS
    CoordinateSystems.hxx
    EntityManager.hxx
    Frame.hxx
    Math.hxx
    UnitTypes.hxx
)

flightgear_component(OpenDIS "${SOURCES}" "${HEADERS}")
