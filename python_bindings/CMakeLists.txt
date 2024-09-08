cmake_minimum_required(VERSION 3.25)

include(FetchContent)

# Fetch pybind11
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.11.1
  SYSTEM
)

find_package(MAVSDK REQUIRED)

# Ask pybind11 to locate python, and make pybind11 available
set(PYBIND11_FINDPYTHON ON)
FetchContent_MakeAvailable(pybind11)

# Define the Python module
pybind11_add_module(_telemetry_stream MODULE python_bindings.cpp)

target_link_libraries(_telemetry_stream PRIVATE MAVSDK::mavsdk)

target_include_directories(_telemetry_stream PRIVATE ${MAVSDK_INCLUDE_DIRS})

