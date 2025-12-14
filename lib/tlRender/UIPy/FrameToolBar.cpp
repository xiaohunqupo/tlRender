// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/Bindings.h>

#include <tlRender/UI/FrameToolBar.h>

#include <ftk/UI/Action.h>
#include <ftk/Core/Context.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void frameToolBar(py::module_& m)
        {
            using namespace timelineui;

            py::class_<FrameToolBar, ftk::ToolBar, std::shared_ptr<FrameToolBar> >(m, "FrameToolBar")
                .def(
                    py::init(&FrameToolBar::create),
                    py::arg("context"),
                    py::arg("parent") = nullptr)
                .def_property_readonly("actions", &FrameToolBar::getActions)
                .def_property("player", &FrameToolBar::getPlayer, &FrameToolBar::setPlayer);
        }
    }
}

