// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <TimelinePy/Player.h>

#include <tlRender/Timeline/Player.h>

#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    void playerBind(py::module_& m)
    {
        py::enum_<timeline::Playback>(m, "Playback")
            .value("Stop", timeline::Playback::Stop)
            .value("Forward", timeline::Playback::Forward)
            .value("Reverse", timeline::Playback::Reverse);

        py::class_<timeline::Player, std::shared_ptr<timeline::Player> >(m, "Player")
            .def(
                py::init(py::overload_cast<
                    const std::shared_ptr<ftk::Context>&,
                    const std::shared_ptr<timeline::Timeline>&,
                    const timeline::PlayerOptions&>(&timeline::Player::create)),
                py::arg("context"),
                py::arg("timeline"),
                py::arg("options") = timeline::PlayerOptions())
            .def_property("playback", &timeline::Player::getPlayback, &timeline::Player::setPlayback)
            .def("tick", &timeline::Player::tick);
    }
}
