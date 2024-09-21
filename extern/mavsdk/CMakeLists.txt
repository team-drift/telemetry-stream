cmake_minimum_required(VERSION 3.10.2)

include(FetchContent)

set(PACKAGE_NAME MAVSDK)

FetchContent_Declare(
    ${PACKAGE_NAME}
    GIT_REPOSITORY https://github.com/mavlink/MAVSDK.git
    # GIT_TAG v2.10.2  # Lock MAVSDK to a specific version
    GIT_TAG v2.12.6
    SYSTEM
)

FetchContent_MakeAvailable(${PACKAGE_NAME})
