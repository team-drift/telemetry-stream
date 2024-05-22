cmake_minimum_required(VERSION 3.10.2)

include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.3  # Lock to a specific version
    SYSTEM
)

FetchContent_MakeAvailable(json)
