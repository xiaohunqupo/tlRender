// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/ColorOptions.h>

#include <tlRender/Timeline/ColorOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void colorOptions(py::module_& m)
        {
            py::enum_<timeline::OCIOConfig>(m, "OCIOConfig")
                .value("BuiltIn", timeline::OCIOConfig::BuiltIn)
                .value("EnvVar", timeline::OCIOConfig::EnvVar)
                .value("File", timeline::OCIOConfig::File);

            py::class_<timeline::OCIOOptions>(m, "OCIOOptions")
                .def(py::init())
                .def_readwrite("enabled", &timeline::OCIOOptions::enabled)
                .def_readwrite("config", &timeline::OCIOOptions::config)
                .def_readwrite("fileName", &timeline::OCIOOptions::fileName)
                .def_readwrite("input", &timeline::OCIOOptions::input)
                .def_readwrite("display", &timeline::OCIOOptions::display)
                .def_readwrite("view", &timeline::OCIOOptions::view)
                .def_readwrite("look", &timeline::OCIOOptions::look)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
