cmake_minimum_required(VERSION 3.25)
project(pdts VERSION 0.0.1 DESCRIPTION "Python bindings for DTS" HOMEPAGE_URL "URL" LANGUAGES CXX)

include(FetchContent)

# Fetch pybind11:

FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG v2.11.1
  SYSTEM
)

# Ask pybind11 to locate python, and make pybind11 available:

set(PYBIND11_FINDPYTHON ON)
FetchContent_MakeAvailable(pybind11)

# Define a python module:

pybind11_add_module(_pdts MODULE pdts.cpp)

# Link DTS

set_target_properties(dts PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(_pdts PRIVATE dts)

# Install the shared library to a place we expect (required for setuptools install):

install(TARGETS _pdts DESTINATION pdts)
