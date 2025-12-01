// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/ForegroundOptions.h>

#include <tlRender/Timeline/ForegroundOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void foregroundOptions(py::module_& m)
        {
            py::class_<timeline::Grid>(m, "Grid")
                .def(py::init())
                .def_readwrite("enabled", &timeline::Grid::enabled)
                .def_readwrite("size", &timeline::Grid::size)
                .def_readwrite("lineWidth", &timeline::Grid::lineWidth)
                .def_readwrite("color", &timeline::Grid::color)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<timeline::Outline>(m, "Grid")
                .def(py::init())
                .def_readwrite("enabled", &timeline::Outline::enabled)
                .def_readwrite("width", &timeline::Outline::width)
                .def_readwrite("color", &timeline::Outline::color)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<timeline::ForegroundOptions>(m, "ForegroundOptions")
                .def(py::init())
                .def_readwrite("grid", &timeline::ForegroundOptions::grid)
                .def_readwrite("outline", &timeline::ForegroundOptions::outline)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
