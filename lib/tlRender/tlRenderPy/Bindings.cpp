// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#include <tlRender/TimelinePy/Bindings.h>
#include <tlRender/UIPy/Bindings.h>

#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(tlRenderPy, m)
{
    py::module_::import("ftkPy");
    m.doc() = "tlRender is an open source library for building playback and review applications for visual effects, film, and animation.";
    tl::python::timelineBind(m);
    tl::python::uiBind(m);
}
