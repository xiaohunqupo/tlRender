// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/BackgroundOptions.h>

#include <tlRender/Timeline/BackgroundOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void backgroundOptions(py::module_& m)
        {
            py::enum_<timeline::Background>(m, "Background")
                .value("Solid", timeline::Background::Solid)
                .value("Checkers", timeline::Background::Checkers)
                .value("Gradient", timeline::Background::Gradient);

            py::class_<timeline::BackgroundOptions>(m, "BackgroundOptions")
                .def(py::init())
                .def_readwrite("type", &timeline::BackgroundOptions::type)
                .def_readwrite("solidColor", &timeline::BackgroundOptions::solidColor)
                .def_readwrite("checkersColor", &timeline::BackgroundOptions::checkersColor)
                .def_readwrite("gradientColor", &timeline::BackgroundOptions::gradientColor)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
