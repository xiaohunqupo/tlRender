// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#include <tlRender/UIPy/Bindings.h>

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/CorePy/Bindings.h>

#include <opentimelineio/version.h>

#include <pybind11/pybind11.h>

#include <iostream>

namespace py = pybind11;

PYBIND11_MODULE(tlRenderPy, m)
{
    m.doc() = "tlRender is an open source library for building playback and review applications for visual effects, film, and animation.";

    py::module_::import("opentimelineio");
    py::module_::import("ftkPy");

    tl::python::coreBind(m);
    tl::python::timelineBind(m);
    tl::python::uiBind(m);
}

