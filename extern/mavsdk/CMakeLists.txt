cmake_minimum_required(VERSION 3.10.2)

include(FetchContent)

set(PACKAGE_NAME MAVSDK)

FetchContent_Declare(
    ${PACKAGE_NAME}
    GIT_REPOSITORY https://github.com/mavlink/MAVSDK.git
    GIT_TAG v2.10.2  # Lock MAVSDK to a specific version
    SYSTEM
)

FetchContent_MakeAvailable(${PACKAGE_NAME})

include(CMakePrintHelpers)

cmake_print_variables(MAVSDK_INCLUDE_DIRS)

set(MAVSDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/mavsdk-build/src)

cmake_print_variables(MAVSDK_DIR)

# Find MAVSDK
find_package(MAVSDK REQUIRED)

# Add MAVSDK include directories
include_directories(${MAVSDK_INCLUDE_DIRS})
