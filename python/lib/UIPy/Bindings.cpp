// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <UIPy/Bindings.h>

#include <UIPy/PlaybackButtons.h>
#include <UIPy/TimeEdit.h>
#include <UIPy/TimelineWidget.h>
#include <UIPy/Viewport.h>

#include <tlRender/UI/Init.h>

#include <ftk/Core/Context.h>

namespace py = pybind11;

namespace tl
{
    namespace python
        {
        void uiBind(py::module_& m)
        {
            m.def(
                "uiInit",
                &timelineui::init,
                py::arg("context"),
                "Initialize the library.");

            playbackButtons(m);
            timeEdit(m);
            timelineWidget(m);
            viewport(m);
        }
    }
}

