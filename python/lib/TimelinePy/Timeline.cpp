// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <TimelinePy/Timeline.h>

#include <tlRender/Timeline/Timeline.h>

#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void timeline(py::module_& m)
        {
            py::class_<timeline::Timeline, std::shared_ptr<timeline::Timeline> >(m, "Timeline")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const ftk::Path&,
                        const timeline::Options&>(&timeline::Timeline::create)),
                    py::arg("context"),
                    py::arg("path"),
                    py::arg("options") = timeline::Options());
        }
    }
}
