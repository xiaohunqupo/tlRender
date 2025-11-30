// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/CompareOptions.h>

#include <tlRender/Timeline/CompareOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void compareOptions(py::module_& m)
        {
            py::enum_<timeline::Compare>(m, "Compare")
                .value("A", timeline::Compare::A)
                .value("B", timeline::Compare::B)
                .value("Wipe", timeline::Compare::Wipe)
                .value("Overlay", timeline::Compare::Overlay)
                .value("Difference", timeline::Compare::Difference)
                .value("Horizontal", timeline::Compare::Horizontal)
                .value("Vertical", timeline::Compare::Vertical)
                .value("Tile", timeline::Compare::Tile);

            py::enum_<timeline::CompareTime>(m, "CompareTime")
                .value("Relative", timeline::CompareTime::Relative)
                .value("Absolute", timeline::CompareTime::Absolute);

            py::class_<timeline::CompareOptions>(m, "CompareOptions")
                .def(py::init())
                .def_readwrite("compare", &timeline::CompareOptions::compare)
                .def_readwrite("wipeCenter", &timeline::CompareOptions::wipeCenter)
                .def_readwrite("wipeRotation", &timeline::CompareOptions::wipeRotation)
                .def_readwrite("overlay", &timeline::CompareOptions::overlay)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
