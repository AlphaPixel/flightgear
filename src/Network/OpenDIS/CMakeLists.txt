set(SOURCES
    EntityStateProcessor.cxx
)

set(HEADERS
    EntityStateProcessor.hxx
)

flightgear_component(OpenDIS "${SOURCES}" "${HEADERS}")
