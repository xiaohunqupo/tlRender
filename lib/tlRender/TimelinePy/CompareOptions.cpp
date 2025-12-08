// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/CompareOptions.h>

#include <tlRender/Timeline/CompareOptions.h>

#include <ftk/CorePy/Util.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void compareOptions(py::module_& m)
        {
            using namespace timeline;
            
            py::enum_<Compare>(m, "Compare")
                .value("A", Compare::A)
                .value("B", Compare::B)
                .value("Wipe", Compare::Wipe)
                .value("Overlay", Compare::Overlay)
                .value("Difference", Compare::Difference)
                .value("Horizontal", Compare::Horizontal)
                .value("Vertical", Compare::Vertical)
                .value("Tile", Compare::Tile);
            FTK_ENUM_BIND(m, Compare);

            py::enum_<CompareTime>(m, "CompareTime")
                .value("Relative", CompareTime::Relative)
                .value("Absolute", CompareTime::Absolute);
            FTK_ENUM_BIND(m, CompareTime);

            py::class_<CompareOptions>(m, "CompareOptions")
                .def(py::init())
                .def_readwrite("compare", &CompareOptions::compare)
                .def_readwrite("wipeCenter", &CompareOptions::wipeCenter)
                .def_readwrite("wipeRotation", &CompareOptions::wipeRotation)
                .def_readwrite("overlay", &CompareOptions::overlay)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
