// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <UIPy/Window.h>

#include <tlRender/UI/Window.h>

#include <ftk/UI/App.h>
#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    void windowBind(py::module_& m)
    {
        py::class_<timelineui::Window, ftk::Window, std::shared_ptr<timelineui::Window> >(m, "Window")
            .def(
                py::init(py::overload_cast<
                    const std::shared_ptr<ftk::Context>&,
                    const std::shared_ptr<ftk::App>&,
                    const std::string&,
                    const ftk::Size2I&>(&timelineui::Window::create)),
                py::arg("context"),
                py::arg("app"),
                py::arg("name"),
                py::arg("size"));
    }
}
