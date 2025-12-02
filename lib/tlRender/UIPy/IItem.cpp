// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/IItem.h>

#include <tlRender/UI/IItem.h>

#include <ftk/Core/Context.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void iItem(py::module_& m)
        {
            py::enum_<timelineui::InOutDisplay>(m, "InOutDisplay")
                .value("InsideRange", timelineui::InOutDisplay::InsideRange)
                .value("OutsideRange", timelineui::InOutDisplay::OutsideRange);

            py::enum_<timelineui::CacheDisplay>(m, "CacheDisplay")
                .value("VideoAndAudio", timelineui::CacheDisplay::VideoAndAudio)
                .value("VideoOnly", timelineui::CacheDisplay::VideoOnly);

            py::enum_<timelineui::WaveformPrim>(m, "WaveformPrim")
                .value("Mesh", timelineui::WaveformPrim::Mesh)
                .value("Image", timelineui::WaveformPrim::Image);

            py::class_<timelineui::ItemOptions>(m, "ItemOptions")
                .def(py::init())
                .def_readwrite("inputEnabled", &timelineui::ItemOptions::inputEnabled)
                .def_readwrite("editAssociatedClips", &timelineui::ItemOptions::editAssociatedClips)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<timelineui::DisplayOptions>(m, "DisplayOptions")
                .def(py::init())
                .def_readwrite("inOutDisplay", &timelineui::DisplayOptions::inOutDisplay)
                .def_readwrite("cacheDisplay", &timelineui::DisplayOptions::cacheDisplay)
                .def_readwrite("minimize", &timelineui::DisplayOptions::minimize)
                .def_readwrite("thumbnails", &timelineui::DisplayOptions::thumbnails)
                .def_readwrite("thumbnailHeight", &timelineui::DisplayOptions::thumbnailHeight)
                .def_readwrite("waveformWidth", &timelineui::DisplayOptions::waveformWidth)
                .def_readwrite("waveformHeight", &timelineui::DisplayOptions::waveformHeight)
                .def_readwrite("waveformPrim", &timelineui::DisplayOptions::waveformPrim)
                .def_readwrite("regularFont", &timelineui::DisplayOptions::regularFont)
                .def_readwrite("monoFont", &timelineui::DisplayOptions::monoFont)
                .def_readwrite("fontSize", &timelineui::DisplayOptions::fontSize)
                .def_readwrite("clipRectScale", &timelineui::DisplayOptions::clipRectScale)
                .def_readwrite("ocio", &timelineui::DisplayOptions::ocio)
                .def_readwrite("lut", &timelineui::DisplayOptions::lut)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
            
        }
    }
}

