// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Player.h>

#include <tlRender/Timeline/Player.h>

#include <ftk/CorePy/ObservableValue.h>
#include <ftk/Core/Context.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void player(py::module_& m)
        {
            py::enum_<timeline::Playback>(m, "Playback")
                .value("Stop", timeline::Playback::Stop)
                .value("Forward", timeline::Playback::Forward)
                .value("Reverse", timeline::Playback::Reverse);

            ftk::python::observableValue<timeline::Playback>(m, "Playback");
            ftk::python::observableValue<OTIO_NS::RationalTime>(m, "RationalTime");

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
                .def_property_readonly("observePlayback", &timeline::Player::observePlayback)
                .def_property("currentTime", &timeline::Player::getCurrentTime, &timeline::Player::seek)
                .def_property_readonly("observeCurrentTime", &timeline::Player::observeCurrentTime)
                .def_property_readonly("observeSeek", &timeline::Player::observeSeek)
                .def("tick", &timeline::Player::tick);
        }
    }
}

