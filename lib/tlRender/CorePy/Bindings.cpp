// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/CorePy/Bindings.h>

#include <tlRender/Core/Init.h>

#include <ftk/Core/Context.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void coreBind(py::module_& m)
        {
            m.def(
                "init",
                &init,
                py::arg("context"),
                "Initialize the library.");

            audio(m);
            time(m);
        }
    }
}

