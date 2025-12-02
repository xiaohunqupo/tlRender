// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/TimelineWidget.h>

#include <tlRender/UI/TimelineWidget.h>

#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void timelineWidget(py::module_& m)
        {
            py::class_<timelineui::TimelineWidget, ftk::IWidget, std::shared_ptr<timelineui::TimelineWidget> >(m, "TimelineWidget")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<ftk::IWidget>&>(&timelineui::TimelineWidget::create)),
                    py::arg("context"),
                    py::arg("parent") = nullptr)
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<timeline::ITimeUnitsModel>&,
                        const std::shared_ptr<ftk::IWidget>&>(&timelineui::TimelineWidget::create)),
                    py::arg("context"),
                    py::arg("timeUnitsModel"),
                    py::arg("parent") = nullptr)
                .def_property_readonly(
                    "timeUnitsModel",
                    &timelineui::TimelineWidget::getTimeUnitsModel)
                .def_property(
                    "player",
                    &timelineui::TimelineWidget::getPlayer,
                    &timelineui::TimelineWidget::setPlayer)
                .def_property(
                    "displayOptions",
                    &timelineui::TimelineWidget::getDisplayOptions,
                    &timelineui::TimelineWidget::setDisplayOptions)
                .def_property_readonly(
                    "observeDisplayOptions",
                    &timelineui::TimelineWidget::observeDisplayOptions);
        }
    }
}

