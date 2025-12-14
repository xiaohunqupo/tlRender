// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IOPy/Bindings.h>

#include <tlRender/IO/IO.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void io(py::module_& m)
        {
            using namespace io;
            
            py::enum_<FileType>(m, "FileType")
                .value("Unknown", FileType::Unknown)
                .value("Media", FileType::Media)
                .value("Seq", FileType::Seq);
            
            py::class_<Info>(m, "Info")
                .def(py::init())
                .def_readwrite("video", &Info::video)
                .def_readwrite("videoTime", &Info::videoTime)
                .def_readwrite("audio", &Info::audio)
                .def_readwrite("audioTime", &Info::audioTime)
                .def_readwrite("tags", &Info::tags)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
