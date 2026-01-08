// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/TimeUnitsWidget.h>

#include <tlRender/UI/TimeUnitsWidget.h>

#include <ftk/Core/Context.h>

#include <pybind11/stl.h>
#include <pybind11/operators.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void timeUnitsWidget(py::module_& m)
        {
            using namespace ui;

            py::class_<TimeUnitsWidget, ftk::IWidget, std::shared_ptr<TimeUnitsWidget> >(m, "TimeUnitsWidget")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<TimeUnitsModel>&,
                        const std::shared_ptr<ftk::IWidget>&>(&TimeUnitsWidget::create)),
                    py::arg("context"),
                    py::arg("timeUnitsModel"),
                    py::arg("parent") = nullptr);
        }
    }
}

