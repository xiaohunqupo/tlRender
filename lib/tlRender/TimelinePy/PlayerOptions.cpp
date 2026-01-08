// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

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
            py::class_<PlayerCacheOptions>(m, "PlayerCacheOptions")
                .def(py::init())
                .def_readwrite("videoGB", &PlayerCacheOptions::videoGB)
                .def_readwrite("audioGB", &PlayerCacheOptions::audioGB)
                .def_readwrite("readBehind", &PlayerCacheOptions::readBehind)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<PlayerOptions>(m, "PlayerOptions")
                .def(py::init())
                .def_readwrite("audioDevice", &PlayerOptions::audioDevice)
                .def_readwrite("cache", &PlayerOptions::cache)
                .def_readwrite("videoRequestMax", &PlayerOptions::videoRequestMax)
                .def_readwrite("audioRequestMax", &PlayerOptions::audioRequestMax)
                .def_readwrite("audioBufferFrameCount", &PlayerOptions::audioBufferFrameCount)
                .def_readwrite("muteTimeout", &PlayerOptions::muteTimeout)
                .def_readwrite("sleepTimeout", &PlayerOptions::sleepTimeout)
                .def_readwrite("currentTime", &PlayerOptions::currentTime)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            m.def("to_json",
                [](const PlayerCacheOptions& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });

            m.def("from_json",
                [](const std::string& value, PlayerCacheOptions& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
        }
    }
}

