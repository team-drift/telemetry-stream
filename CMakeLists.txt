cmake_minimum_required(VERSION 3.25)

project(telemetry_stream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MAVSDK REQUIRED)


# Pull in external projects (e.g., nlohmann_json)
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

# Compiler options for better warnings
if(NOT MSVC)
    target_compile_options(telemetry_stream PRIVATE -Wall -Wextra)
else()
    target_compile_options(telemetry_stream PRIVATE /WX /W2)
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/python_bindings)
    add_subdirectory(python_bindings)
else()
    message(FATAL_ERROR "'python_bindings' directory not found!")
endif()


# install(TARGETS _telemetry_stream DESTINATION /TelemetryModule)