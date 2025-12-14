// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UIPy/Viewport.h>

#include <tlRender/UI/Viewport.h>

#include <ftk/CorePy/Bindings.h>
#include <ftk/Core/Context.h>

#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void viewport(py::module_& m)
        {
            using namespace timelineui;

            ftk::python::observable<std::pair<ftk::V2I, double> >(m, "ViewPosAndZoom");
            
            py::class_<Viewport, ftk::IWidget, std::shared_ptr<Viewport> >(m, "Viewport")
                .def(
                    py::init(py::overload_cast<
                        const std::shared_ptr<ftk::Context>&,
                        const std::shared_ptr<ftk::IWidget>&>(&Viewport::create)),
                    py::arg("context"),
                    py::arg("parent") = nullptr)
                .def_property("compareOptions",
                    &Viewport::getCompareOptions,
                    &Viewport::setCompareOptions)
                .def_property_readonly(
                    "observeCompareOptions",
                    &Viewport::observeCompareOptions)
                .def_property(
                    "ocioOptions",
                    &Viewport::getOCIOOptions,
                    &Viewport::setOCIOOptions)
                .def_property_readonly(
                    "observeOCIOOptions",
                    &Viewport::observeOCIOOptions)
                .def_property(
                    "LUTOptions",
                    &Viewport::getLUTOptions,
                    &Viewport::setLUTOptions)
                .def_property_readonly(
                    "observeLUTOptions",
                    &Viewport::observeLUTOptions)
                .def_property(
                    "imageOptions",
                    &Viewport::getImageOptions,
                    &Viewport::setImageOptions)
                .def_property_readonly(
                    "observeImageOptions",
                    &Viewport::observeImageOptions)
                .def_property(
                    "displayOptions",
                    &Viewport::getDisplayOptions,
                    &Viewport::setDisplayOptions)
                .def_property_readonly(
                    "observeDisplayOptions",
                    &Viewport::observeDisplayOptions)
                .def_property(
                    "backgroundOptions",
                    &Viewport::getBackgroundOptions,
                    &Viewport::setBackgroundOptions)
                .def_property_readonly(
                    "observeBackgroundOptions",
                    &Viewport::observeBackgroundOptions)
                .def_property(
                    "foregroundOptions",
                    &Viewport::getForegroundOptions,
                    &Viewport::setForegroundOptions)
                .def_property_readonly(
                    "observeForegroundOptions",
                    &Viewport::observeForegroundOptions)
                .def_property(
                    "colorBuffer",
                    &Viewport::getColorBuffer,
                    &Viewport::setColorBuffer)
                .def_property_readonly(
                    "observeColorBuffer",
                    &Viewport::observeColorBuffer)
                .def_property(
                    "player",
                    &Viewport::getPlayer,
                    &Viewport::setPlayer)
                .def_property_readonly(
                    "viewPos",
                    &Viewport::getViewPos)
                .def_property_readonly(
                    "observeViewPos",
                    &Viewport::observeViewPos)
                .def_property_readonly(
                    "viewZoom",
                    &Viewport::getViewZoom)
                .def_property_readonly(
                    "observeViewZoom",
                    &Viewport::observeViewZoom)
                .def_property_readonly(
                    "viewPosAndZoom",
                    &Viewport::getViewPosAndZoom)
                .def(
                    "setViewPosAndZoom",
                    &Viewport::setViewPosAndZoom,
                    py::arg("pos"),
                    py::arg("zoom"))
                .def_property_readonly(
                    "observeViewPosAndZoom",
                    &Viewport::observeViewPosAndZoom)
                .def(
                    "setViewZoom",
                    &Viewport::setViewPosAndZoom,
                    py::arg("zoom"),
                    py::arg("focus"))
                .def_property(
                    "frameView",
                    &Viewport::hasFrameView,
                    &Viewport::setFrameView)
                .def_property_readonly(
                    "observeFrameView",
                    &Viewport::observeFrameView)
                .def_property_readonly(
                    "observeFramed",
                    &Viewport::observeFramed)
                .def("viewZoomReset", &Viewport::viewZoomReset)
                .def("viewZoomIn", &Viewport::viewZoomIn)
                .def("viewZoomOut", &Viewport::viewZoomOut)
                .def_property_readonly(
                    "FPS",
                    &Viewport::getFPS)
                .def_property_readonly(
                    "observeFPS",
                    &Viewport::observeFPS)
                .def_property_readonly(
                    "droppedFrames",
                    &Viewport::getDroppedFrames)
                .def_property_readonly(
                    "observeDroppedFrames",
                    &Viewport::observeDroppedFrames)
                .def("getColorSample", &Viewport::getColorSample)
                .def(
                    "setPanBinding",
                    &Viewport::setPanBinding,
                    py::arg("button"),
                    py::arg("modifier"))
                .def(
                    "setWipeBinding",
                    &Viewport::setPanBinding,
                    py::arg("button"),
                    py::arg("modifier"))
                .def(
                    "setMouseWheelScale",
                    &Viewport::setMouseWheelScale);
        }
    }
}

