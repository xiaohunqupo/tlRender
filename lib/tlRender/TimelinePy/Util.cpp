// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/Timeline/Util.h>

#include <ftk/Core/Context.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void util(py::module_& m)
        {
            m.def(
                "getExts",
                &timeline::getExts,
                py::arg("context"),
                py::arg("types") =
                    static_cast<int>(FileType::Media) |
                    static_cast<int>(FileType::Seq));
        }
    }
}
