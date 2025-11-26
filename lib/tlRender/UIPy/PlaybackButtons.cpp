// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/PlaybackButtons.h>

#include <tlRender/UI/PlaybackButtons.h>

#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void playbackButtons(py::module_& m)
        {
            py::class_<timelineui::PlaybackButtons, ftk::IWidget, std::shared_ptr<timelineui::PlaybackButtons> >(m, "PlaybackButtons")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<ftk::IWidget>&>(&timelineui::PlaybackButtons::create)),
                    py::arg("context"),
                    py::arg("parent") = nullptr)
                .def("setPlayback", &timelineui::PlaybackButtons::setPlayback)
                .def("setCallback", &timelineui::PlaybackButtons::setCallback);
        }
    }
}

