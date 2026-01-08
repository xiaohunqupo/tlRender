// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/Timeline/Video.h>

#include <ftk/Core/Context.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void video(py::module_& m)
        {
            py::class_<VideoLayer>(m, "VideoLayer")
                .def(py::init())
                .def_readwrite("image", &VideoLayer::image)
                .def_readwrite("imageOptions", &VideoLayer::imageOptions)
                .def_readwrite("imageB", &VideoLayer::imageB)
                .def_readwrite("imageOptionsB", &VideoLayer::imageOptionsB)
                .def_readwrite("transition", &VideoLayer::transition)
                .def_readwrite("transitionValue", &VideoLayer::transitionValue)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<VideoFrame>(m, "VideoFrame")
                .def(py::init())
                .def_readwrite("size", &VideoFrame::size)
                .def_readwrite("time", &VideoFrame::time)
                .def_readwrite("layers", &VideoFrame::layers)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            m.def(
                "isTimeEqual",
                [](const VideoFrame& a, const VideoFrame& b)
                {
                    return isTimeEqual(a, b);
                });
        }
    }
}
