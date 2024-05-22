cmake_minimum_required(VERSION 3.10.2)

project(telemetry_stream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pull in external proejcts

add_subdirectory(extern)

add_executable(telemetry_stream_async
    telemetry_stream_async.cpp
)

target_link_libraries(telemetry_stream_async PRIVATE
    mavsdk nlohmann_json::nlohmann_json
)

if(NOT MSVC)
    add_compile_options(telemetry_stream_async PRIVATE -Wall -Wextra)
else()
    add_compile_options(telemetry_stream_async PRIVATE -WX -W2)
endif()
