cmake_minimum_required(VERSION 3.25)

project(telemetry_stream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find MAVSDK
find_package(MAVSDK REQUIRED)


# Add MAVSDK include directories
include_directories(${MAVSDK_INCLUDE_DIRS})


# Pull in external projects (nlohmann_json, MAVsdk)
add_subdirectory(extern)  

# Define the C++ executable (telemetry_stream.cpp)
add_executable(telemetry_stream
    telemetry_stream.cpp
)

# Link external libraries (MAVSDK, nlohmann_json) to the C++ executable
target_link_libraries(telemetry_stream PRIVATE
    MAVSDK::mavsdk
    nlohmann_json::nlohmann_json
)
