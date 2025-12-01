// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/Viewport.h>

#include <tlRender/UI/Viewport.h>

#include <ftk/CorePy/ObservableValue.h>
#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void viewport(py::module_& m)
        {
            ftk::python::observableValue<std::pair<ftk::V2I, double> >(m, "ViewPosAndZoom");
            
            py::class_<timelineui::Viewport, ftk::IWidget, std::shared_ptr<timelineui::Viewport> >(m, "Viewport")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<ftk::IWidget>&>(&timelineui::Viewport::create)),
                    py::arg("context"),
                    py::arg("parent") = nullptr)
                .def_property("compareOptions",
                    &timelineui::Viewport::getCompareOptions,
                    &timelineui::Viewport::setCompareOptions)
                .def_property_readonly(
                    "observeCompareOptions",
                    &timelineui::Viewport::observeCompareOptions)
                .def_property(
                    "ocioOptions",
                    &timelineui::Viewport::getOCIOOptions,
                    &timelineui::Viewport::setOCIOOptions)
                .def_property_readonly(
                    "observeOCIOOptions",
                    &timelineui::Viewport::observeOCIOOptions)
                .def_property(
                    "LUTOptions",
                    &timelineui::Viewport::getLUTOptions,
                    &timelineui::Viewport::setLUTOptions)
                .def_property_readonly(
                    "observeLUTOptions",
                    &timelineui::Viewport::observeLUTOptions)
                .def_property(
                    "imageOptions",
                    &timelineui::Viewport::getImageOptions,
                    &timelineui::Viewport::setImageOptions)
                .def_property_readonly(
                    "observeImageOptions",
                    &timelineui::Viewport::observeImageOptions)
                .def_property(
                    "displayOptions",
                    &timelineui::Viewport::getDisplayOptions,
                    &timelineui::Viewport::setDisplayOptions)
                .def_property_readonly(
                    "observeDisplayOptions",
                    &timelineui::Viewport::observeDisplayOptions)
                .def_property(
                    "backgroundOptions",
                    &timelineui::Viewport::getBackgroundOptions,
                    &timelineui::Viewport::setBackgroundOptions)
                .def_property_readonly(
                    "observeBackgroundOptions",
                    &timelineui::Viewport::observeBackgroundOptions)
                .def_property(
                    "foregroundOptions",
                    &timelineui::Viewport::getForegroundOptions,
                    &timelineui::Viewport::setForegroundOptions)
                .def_property_readonly(
                    "observeForegroundOptions",
                    &timelineui::Viewport::observeForegroundOptions)
                .def_property(
                    "colorBuffer",
                    &timelineui::Viewport::getColorBuffer,
                    &timelineui::Viewport::setColorBuffer)
                .def_property_readonly(
                    "observeColorBuffer",
                    &timelineui::Viewport::observeColorBuffer)
                .def_property(
                    "player",
                    &timelineui::Viewport::getPlayer,
                    &timelineui::Viewport::setPlayer)
                .def_property_readonly(
                    "viewPos",
                    &timelineui::Viewport::getViewPos)
                .def_property_readonly(
                    "observeViewPos",
                    &timelineui::Viewport::observeViewPos)
                .def_property_readonly(
                    "viewZoom",
                    &timelineui::Viewport::getViewZoom)
                .def_property_readonly(
                    "observeViewZoom",
                    &timelineui::Viewport::observeViewZoom)
                .def_property_readonly(
                    "viewPosAndZoom",
                    &timelineui::Viewport::getViewPosAndZoom)
                .def(
                    "setViewPosAndZoom",
                    &timelineui::Viewport::setViewPosAndZoom,
                    py::arg("pos"),
                    py::arg("zoom"))
                .def_property_readonly(
                    "observeViewPosAndZoom",
                    &timelineui::Viewport::observeViewPosAndZoom)
                .def(
                    "setViewZoom",
                    &timelineui::Viewport::setViewPosAndZoom,
                    py::arg("zoom"),
                    py::arg("focus"))
                .def_property(
                    "frameView",
                    &timelineui::Viewport::hasFrameView,
                    &timelineui::Viewport::setFrameView)
                .def_property_readonly(
                    "observeFrameView",
                    &timelineui::Viewport::observeFrameView)
                .def_property_readonly(
                    "observeFramed",
                    &timelineui::Viewport::observeFramed)
                .def("viewZoomReset", &timelineui::Viewport::viewZoomReset)
                .def("viewZoomIn", &timelineui::Viewport::viewZoomIn)
                .def("viewZoomOut", &timelineui::Viewport::viewZoomOut)
                .def_property_readonly(
                    "FPS",
                    &timelineui::Viewport::getFPS)
                .def_property_readonly(
                    "observeFPS",
                    &timelineui::Viewport::observeFPS)
                .def_property_readonly(
                    "droppedFrames",
                    &timelineui::Viewport::getDroppedFrames)
                .def_property_readonly(
                    "observeDroppedFrames",
                    &timelineui::Viewport::observeDroppedFrames)
                .def("getColorSample", &timelineui::Viewport::getColorSample)
                .def(
                    "setPanBinding",
                    &timelineui::Viewport::setPanBinding,
                    py::arg("button"),
                    py::arg("modifier"))
                .def(
                    "setWipeBinding",
                    &timelineui::Viewport::setPanBinding,
                    py::arg("button"),
                    py::arg("modifier"))
                .def(
                    "setMouseWheelScale",
                    &timelineui::Viewport::setMouseWheelScale);
        }
    }
}

