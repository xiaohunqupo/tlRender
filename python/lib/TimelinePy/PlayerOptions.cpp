// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <TimelinePy/PlayerOptions.h>

#include <tlRender/Timeline/PlayerOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void playerOptions(py::module_& m)
        {
            py::class_<timeline::PlayerOptions>(m, "PlayerOptions")
                .def_readwrite("videoRequestMax", &timeline::PlayerOptions::videoRequestMax)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}

