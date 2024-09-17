cmake_minimum_required(VERSION 3.25)

project(dts)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pull in external projects (nlohmann_json, MAVsdk)
add_subdirectory(extern)  

# Define the DRIFT Telemetry Stream (DTS) library

add_library(${PROJECT_NAME}
    src/dts.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}>

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set compile options:

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Define C++ standard:

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Link external libraries (MAVSDK, nlohmann_json) to the C++ library
target_link_libraries(${PROJECT_NAME} PRIVATE
    MAVSDK::mavsdk
    nlohmann_json::nlohmann_json
)

# TODO: Define python bindings and demos...