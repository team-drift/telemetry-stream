/**
 * @file pdts.cpp
 * @author Owen Cochell (owencochell@gmail.com)
 * @brief Python binding code
 * @version 0.1
 * @date 2024-09-20
 * 
 * @copyright Copyright (c) 2024
 * 
 * This file contains binding code for certain components
 * that need to be exposed to python.
 */

#include <pybind11/pybind11.h>

#include <string>

#include <dts.hpp>

namespace py = pybind11;

PYBIND11_MODULE(_pdts, m) {  // NOLINT

    // Define version:

    m.attr("__version__") = "0.0.3";

    // Define the module doc:

    m.doc() = "Python wrapper for Drift Telemetry Stream";

    // Create binding for DTStream class:

    py::class_<DTStream>(m, "DTStream")
        .def(py::init<std::string>())
        .def(py::init())
        .def("start", &DTStream::start)
        .def("stop", &DTStream::stop)
        .def("get_data", &DTStream::get_data)
        .def("get_cstr", &DTStream::get_cstr)
        .def("set_cstr", &DTStream::set_cstr)
        .def("get_drop_rate", &DTStream::get_drop_rate)
        .def("set_drop_rate", &DTStream::set_drop_rate);
}
