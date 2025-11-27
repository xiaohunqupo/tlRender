// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/Viewport.h>

#include <tlRender/UI/TimeEdit.h>

#include <tlRender/Timeline/TimeUnits.h>

#include <ftk/Core/Context.h>

#include <pybind11/stl.h>
#include <pybind11/operators.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void timeEdit(py::module_& m)
        {
            //py::object rt = (py::object)py::module_::import("opentimelineio.opentime").attr("RationalTime");
            //py::object rt = (py::object)py::module_::import("_opentime").attr("RationalTime");
            py::class_<timelineui::TimeEdit, ftk::IWidget, std::shared_ptr<timelineui::TimeEdit> >(m, "TimeEdit")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<timeline::TimeUnitsModel>&,
                        const std::shared_ptr<ftk::IWidget>&>(&timelineui::TimeEdit::create)),
                    py::arg("context"),
                    py::arg("timeUnitsModel"),
                    py::arg("parent") = nullptr)
                .def_property("value",
                    //[rt](const std::shared_ptr<timelineui::TimeEdit>& timeEdit)
                    //{
                    //    return rt(timeEdit->getValue());
                    //});
                    &timelineui::TimeEdit::getValue, &timelineui::TimeEdit::setValue);
        }
    }
}

