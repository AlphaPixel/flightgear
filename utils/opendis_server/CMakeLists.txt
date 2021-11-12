cmake_minimum_required(VERSION 3.2)
project(OpenDIS-Server)

## Libraries

find_package(OpenDIS)

## Example Programs
include_directories(.)

# if Windows add M_PI definition
# - issues occurred during testing in Visual Studio
if (WIN32)
  add_definitions(/DM_PI=3.14159265358979323846)
endif (WIN32)

# create list of ExampleSender source files
file(GLOB OPENDIS_SERVER_SOURCES
  "main.cpp"
  "Connection.cpp"
  "Utils.cpp"
  "Timer.cpp"
)

# Define ExampleSender Executable
add_executable(OpenDISServer ${OPENDIS_SERVER_SOURCES})
# Link OpenDIS into ExampleSender
target_link_libraries(OpenDISServer PRIVATE OpenDIS6::OpenDIS6)

