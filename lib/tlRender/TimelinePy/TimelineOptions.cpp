// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/TimelineOptions.h>

#include <tlRender/Timeline/TimelineOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void timelineOptions(py::module_& m)
        {
            py::enum_<timeline::ImageSeqAudio>(m, "ImageSeqAudio")
                .value("None", timeline::ImageSeqAudio::None)
                .value("Ext", timeline::ImageSeqAudio::Ext)
                .value("FileName", timeline::ImageSeqAudio::FileName);

            py::class_<timeline::Options>(m, "Options")
                .def_readwrite("imageSeqAudio", &timeline::Options::imageSeqAudio)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
