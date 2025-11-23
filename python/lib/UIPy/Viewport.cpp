// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <UIPy/Viewport.h>

#include <tlRender/UI/Viewport.h>

#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    void viewportBind(py::module_& m)
    {
        py::class_<timelineui::Viewport, ftk::IWidget, std::shared_ptr<timelineui::Viewport> >(m, "Viewport")
            .def(
                py::init(py::overload_cast<
                    const std::shared_ptr<ftk::Context>&,
                    const std::shared_ptr<ftk::IWidget>&>(&timelineui::Viewport::create)),
                py::arg("context"),
                py::arg("parent") = nullptr)
            .def("setPlayer", &timelineui::Viewport::setPlayer);
    }
}
