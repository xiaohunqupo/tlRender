// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/Bindings.h>

#include <tlRender/UI/PlaybackToolBar.h>

#include <ftk/UI/Action.h>
#include <ftk/Core/Context.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void playbackToolBar(py::module_& m)
        {
            using namespace ui;

            py::class_<PlaybackToolBar, ftk::ToolBar, std::shared_ptr<PlaybackToolBar> >(m, "PlaybackToolBar")
                .def(
                    py::init(&PlaybackToolBar::create),
                    py::arg("context"),
                    py::arg("parent") = nullptr)
                .def_property_readonly("actions", &PlaybackToolBar::getActions)
                .def_property("player", &PlaybackToolBar::getPlayer, &PlaybackToolBar::setPlayer);
        }
    }
}

