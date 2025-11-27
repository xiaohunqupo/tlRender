// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/TimelinePy/CompareOptions.h>
#include <tlRender/TimelinePy/Player.h>
#include <tlRender/TimelinePy/PlayerOptions.h>
#include <tlRender/TimelinePy/TimeUnits.h>
#include <tlRender/TimelinePy/Timeline.h>
#include <tlRender/TimelinePy/TimelineOptions.h>

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
                &timeline::init,
                py::arg("context"),
                "Initialize the library.");

            compareOptions(m);
            timeUnits(m);
            timelineOptions(m);
            timeline(m);
            playerOptions(m);
            player(m);
        }
    }
}

