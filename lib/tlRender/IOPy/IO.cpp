// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IOPy/IO.h>

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
            py::enum_<io::FileType>(m, "FileType")
                .value("Unknown", io::FileType::Unknown)
                .value("Media", io::FileType::Media)
                .value("Seq", io::FileType::Seq);

            py::class_<io::Info>(m, "Info")
                .def(py::init())
                .def_readwrite("video", &io::Info::video)
                .def_readwrite("videoTime", &io::Info::videoTime)
                .def_readwrite("audio", &io::Info::audio)
                .def_readwrite("audioTime", &io::Info::audioTime)
                .def_readwrite("tags", &io::Info::tags)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
        }
    }
}
