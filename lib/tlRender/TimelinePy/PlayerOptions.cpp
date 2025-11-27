// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/PlayerOptions.h>

#include <tlRender/Timeline/PlayerOptions.h>

#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void playerOptions(py::module_& m)
        {
            py::class_<timeline::PlayerCacheOptions>(m, "PlayerCacheOptions")
                .def_readwrite("videoGB", &timeline::PlayerCacheOptions::videoGB)
                .def_readwrite("audioGB", &timeline::PlayerCacheOptions::audioGB)
                .def_readwrite("readBehind", &timeline::PlayerCacheOptions::readBehind)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

                py::class_<timeline::PlayerOptions>(m, "PlayerOptions")
                .def_readwrite("audioDevice", &timeline::PlayerOptions::audioDevice)
                .def_readwrite("cache", &timeline::PlayerOptions::cache)
                .def_readwrite("videoRequestMax", &timeline::PlayerOptions::videoRequestMax)
                .def_readwrite("audioRequestMax", &timeline::PlayerOptions::audioRequestMax)
                .def_readwrite("audioBufferFrameCount", &timeline::PlayerOptions::audioBufferFrameCount)
                .def_readwrite("muteTimeout", &timeline::PlayerOptions::muteTimeout)
                .def_readwrite("sleepTimeout", &timeline::PlayerOptions::sleepTimeout)
                .def_readwrite("currentTime", &timeline::PlayerOptions::currentTime)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}

