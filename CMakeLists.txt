cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(telemetry_stream)

# add_executable(telemetry_stream
#     telemetry_stream.cpp
# )

add_executable(telemetry_stream_async
    telemetry_stream_async.cpp
)

find_package(MAVSDK REQUIRED)

# target_link_libraries(telemetry_stream
#     MAVSDK::mavsdk
# )

target_link_libraries(telemetry_stream_async
    MAVSDK::mavsdk
)

if(NOT MSVC)
    add_compile_options(telemetry_stream PRIVATE -Wall -Wextra)
else()
    add_compile_options(telemetry_stream PRIVATE -WX -W2)
endif()
