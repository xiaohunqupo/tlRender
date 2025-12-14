// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/Timeline/Transition.h>

#include <ftk/CorePy/Bindings.h>
#include <ftk/Core/Context.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void transition(py::module_& m)
        {
            using namespace timeline;

            py::enum_<Transition>(m, "Transition")
                .value("_None", Transition::None)
                .value("Dissolve", Transition::Dissolve);
            FTK_ENUM_BIND(m, Transition);
            
            m.def("toTransition", &toTransition);
        }
    }
}
