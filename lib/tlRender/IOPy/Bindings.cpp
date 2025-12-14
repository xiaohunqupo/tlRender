// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IOPy/Bindings.h>

#include <tlRender/IO/Init.h>

#include <ftk/Core/Context.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void ioBind(py::module_& m)
        {
            auto mIO = m.def_submodule("io", "I/O");
            
            mIO.def(
                "init",
                &io::init,
                py::arg("context"),
                "Initialize the library.");

            io(mIO);
        }
    }
}

