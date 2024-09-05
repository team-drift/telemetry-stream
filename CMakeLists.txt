cmake_minimum_required(VERSION 3.10.2)

project(telemetry_stream)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the directory where pybind11Config.cmake is located
set(pybind11_DIR /Users/swabhankatkoori/.pyenv/versions/3.9.2/lib/python3.9/site-packages/pybind11/share/cmake/pybind11)

# Find Python and pybind11 packages
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(MAVSDK REQUIRED)
find_package(pybind11 REQUIRED)

# Include directories
include_directories(${Python3_INCLUDE_DIRS})

# Pull in external projects (e.g., nlohmann_json)
add_subdirectory(extern)  # Ensure this directory has required CMakeLists.txt files

# Define the Python module (bindings.cpp)
pybind11_add_module(test
    bindings.cpp
)

# Link Python and MAVSDK libraries to the Python module
target_link_libraries(test PRIVATE
    MAVSDK::mavsdk
    nlohmann_json::nlohmann_json
    ${Python3_LIBRARIES}
)

# Define the C++ executable (telemetry_stream.cpp)
add_executable(telemetry_stream
    telemetry_stream.cpp
)

# Link external libraries (MAVSDK, nlohmann_json) to the C++ executable
target_link_libraries(telemetry_stream PRIVATE
    MAVSDK::mavsdk
    nlohmann_json::nlohmann_json
    ${Python3_LIBRARIES}
)

# Compiler options for better warnings
if(NOT MSVC)
    target_compile_options(telemetry_stream PRIVATE -Wall -Wextra)
else()
    target_compile_options(telemetry_stream PRIVATE /WX /W2)
endif()

# Specify the output directory for the Python module
set_target_properties(test PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python_bindings"
)

# Ensure that the Python module and C++ executable are built together
add_dependencies(test telemetry_stream)

