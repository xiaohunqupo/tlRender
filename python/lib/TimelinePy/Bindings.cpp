// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <TimelinePy/Bindings.h>

#include <TimelinePy/Player.h>
#include <TimelinePy/PlayerOptions.h>
#include <TimelinePy/TimeUnits.h>
#include <TimelinePy/Timeline.h>
#include <TimelinePy/TimelineOptions.h>

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

            timeUnits(m);
            timelineOptions(m);
            timeline(m);
            playerOptions(m);
            player(m);
        }
    }
}

