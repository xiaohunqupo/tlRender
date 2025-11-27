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

            py::enum_<timeline::Loop>(m, "Loop")
                .value("Loop", timeline::Loop::Loop)
                .value("Once", timeline::Loop::Once)
                .value("PingPong", timeline::Loop::PingPong);

            py::enum_<timeline::TimeAction>(m, "TimeAction")
                .value("Start", timeline::TimeAction::Start)
                .value("End", timeline::TimeAction::End)
                .value("FramePrev", timeline::TimeAction::FramePrev)
                .value("FramePrevX10", timeline::TimeAction::FramePrevX10)
                .value("FramePrevX100", timeline::TimeAction::FramePrevX100)
                .value("FrameNext", timeline::TimeAction::FrameNext)
                .value("FrameNextX10", timeline::TimeAction::FrameNextX10)
                .value("JumpBack1s", timeline::TimeAction::JumpBack1s)
                .value("JumpBack10s", timeline::TimeAction::JumpBack10s)
                .value("JumpForward1s", timeline::TimeAction::JumpForward1s)
                .value("JumpForward10s", timeline::TimeAction::JumpForward10s);

            ftk::python::observableValue<timeline::Playback>(m, "Playback");
            ftk::python::observableValue<OTIO_NS::RationalTime>(m, "RationalTime");
            ftk::python::observableValue<std::shared_ptr<timeline::Player> >(m, "Player");

            py::class_<timeline::Player, std::shared_ptr<timeline::Player> >(m, "Player")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<timeline::Timeline>&,
                        const timeline::PlayerOptions&>(&timeline::Player::create)),
                    py::arg("context"),
                    py::arg("timeline"),
                    py::arg("options") = timeline::PlayerOptions())

                .def_property_readonly("context", &timeline::Player::getContext)
                .def_property_readonly("timeline", &timeline::Player::getTimeline)
                .def_property_readonly("path", &timeline::Player::getPath)
                .def_property_readonly("audioPath", &timeline::Player::getAudioPath)
                .def_property_readonly("path", &timeline::Player::getPath)
                .def_property_readonly("playerOptions", &timeline::Player::getPlayerOptions)
                .def_property_readonly("options", &timeline::Player::getOptions)
                .def_property_readonly("timeRange", &timeline::Player::getTimeRange)
                .def_property_readonly("duration", &timeline::Player::getDuration)
                .def_property_readonly("ioInfo", &timeline::Player::getIOInfo)

                .def_property_readonly("defaultSpeed", &timeline::Player::getDefaultSpeed)
                .def_property("speed", &timeline::Player::getSpeed, &timeline::Player::setSpeed)
                .def_property_readonly("observeSpeed", &timeline::Player::observeSpeed)
                .def_property("playback", &timeline::Player::getPlayback, &timeline::Player::setPlayback)
                .def_property_readonly("observePlayback", &timeline::Player::observePlayback)
                .def("isStopped", &timeline::Player::isStopped)
                .def("stop", &timeline::Player::stop)
                .def("forward", &timeline::Player::forward)
                .def("reverse", &timeline::Player::reverse)
                .def_property("loop", &timeline::Player::getLoop, &timeline::Player::setLoop)
                .def_property_readonly("observeLoop", &timeline::Player::observeLoop)

                .def_property("currentTime", &timeline::Player::getCurrentTime, &timeline::Player::seek)
                .def_property_readonly("observeCurrentTime", &timeline::Player::observeCurrentTime)
                .def_property_readonly("observeSeek", &timeline::Player::observeSeek)
                .def("timeAction", &timeline::Player::timeAction)
                .def("gotoStart", &timeline::Player::gotoStart)
                .def("gotoEnd", &timeline::Player::gotoEnd)
                .def("framePrev", &timeline::Player::framePrev)
                .def("frameNext", &timeline::Player::frameNext)

                .def_property("inOutRange", &timeline::Player::getInOutRange, &timeline::Player::setInOutRange)
                .def_property_readonly("observeInOutRange", &timeline::Player::observeInOutRange)
                .def("setInPoint", &timeline::Player::setInPoint)
                .def("resetInPoint", &timeline::Player::resetInPoint)
                .def("setOutPoint", &timeline::Player::setOutPoint)
                .def("resetOutPoint", &timeline::Player::resetOutPoint)

                .def_property("compare", &timeline::Player::getCompare, &timeline::Player::setCompare)
                .def_property_readonly("observeCompare", &timeline::Player::observeCompare)
                .def_property("compareTime", &timeline::Player::getCompareTime, &timeline::Player::setCompareTime)
                .def_property_readonly("observeCompareTime", &timeline::Player::observeCompareTime)

                .def_property("ioOptions", &timeline::Player::getIOOptions, &timeline::Player::setIOOptions)
                .def_property_readonly("observeIOOptions", &timeline::Player::observeIOOptions)
                
                .def_property("videoLayer", &timeline::Player::getVideoLayer, &timeline::Player::setVideoLayer)
                .def_property_readonly("observeVideoLayer", &timeline::Player::observeVideoLayer)
                .def_property("compareVideoLayers", &timeline::Player::getCompareVideoLayers, &timeline::Player::setCompareVideoLayers)
                .def_property_readonly("observeCompareVideoLayers", &timeline::Player::observeCompareVideoLayers)
                .def_property_readonly("currentVideo", &timeline::Player::getCurrentVideo)
                .def_property_readonly("observeCurrentVideo", &timeline::Player::observeCurrentVideo)

                .def_property("audioDevice", &timeline::Player::getAudioDevice, &timeline::Player::setAudioDevice)
                .def_property_readonly("observeAudioDevice", &timeline::Player::observeAudioDevice)
                .def_property("volume", &timeline::Player::getVolume, &timeline::Player::setVolume)
                .def_property_readonly("observeVolume", &timeline::Player::observeVolume)
                .def_property("mute", &timeline::Player::isMuted, &timeline::Player::setMute)
                .def_property_readonly("observeMute", &timeline::Player::observeMute)
                .def_property("channelMute", &timeline::Player::getChannelMute, &timeline::Player::setChannelMute)
                .def_property_readonly("observeChannelMute", &timeline::Player::observeChannelMute)
                .def_property("audioOffset", &timeline::Player::getAudioOffset, &timeline::Player::setAudioOffset)
                .def_property_readonly("observeAudioOffset", &timeline::Player::observeAudioOffset)
                .def_property_readonly("getCurrentAudio", &timeline::Player::getCurrentAudio)
                .def_property_readonly("observeCurrentAudio", &timeline::Player::observeCurrentAudio)

                .def_property("cacheOptions", &timeline::Player::getCacheOptions, &timeline::Player::setCacheOptions)
                .def_property_readonly("observeCacheOptions", &timeline::Player::observeCacheOptions)
                .def("clearCache", &timeline::Player::clearCache)

                .def("tick", &timeline::Player::tick);
        }
    }
}

