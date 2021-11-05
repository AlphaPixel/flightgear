include(FetchContent)

# Fetch OpenDIS
FetchContent_Declare(
    OpenDIS
    GIT_REPOSITORY https://github.com/AlphaPixel/open-dis-cpp.git
    GIT_TAG 8503ac98dfbbfa3a8f8c19720d4c3d720894b96d
)

FetchContent_MakeAvailable(OpenDIS)

# Create a library aliases
Add_Library(OpenDIS6::OpenDIS6 ALIAS OpenDIS6)
Add_Library(OpenDIS7::OpenDIS7 ALIAS OpenDIS7)
