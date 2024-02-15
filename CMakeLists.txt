cmake_minimum_required(VERSION 3.15)
project(telemetry-stream)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable strict handling of warnings
add_definitions("-Wall -Wextra -Werror")

# Find MAVSDK package
find_package(MAVSDK REQUIRED)

# Add executable target with source files
add_executable(telemetry-stream main.cpp)

# Link MAVSDK library
target_link_libraries(telemetry-stream
  MAVSDK::mavsdk
)
