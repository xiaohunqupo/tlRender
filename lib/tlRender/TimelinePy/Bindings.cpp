// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/Timeline/Init.h>

#include <ftk/Core/Context.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void timelineBind(py::module_& m)
        {
            m.def(
                "init",
                &init,
                py::arg("context"),
                "Initialize the library.");

            audio(m);
            compareOptions(m);
            timeUnits(m);
            timelineOptions(m);
            timeline(m);
            playerOptions(m);
            player(m);
            transition(m);
            util(m);
            video(m);
        }
    }
}

