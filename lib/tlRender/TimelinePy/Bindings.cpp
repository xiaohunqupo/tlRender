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
            auto mTimeline = m.def_submodule("timeline", "Timeline");
            
            mTimeline.def(
                "init",
                &init,
                py::arg("context"),
                "Initialize the library.");

            audio(mTimeline);
            compareOptions(mTimeline);
            timeUnits(mTimeline);
            timelineOptions(mTimeline);
            timeline(mTimeline);
            playerOptions(mTimeline);
            player(mTimeline);
            transition(mTimeline);
            util(mTimeline);
            video(mTimeline);
        }
    }
}

