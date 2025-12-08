// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/CorePy/Audio.h>

#include <tlRender/Core/Audio.h>

#include <ftk/CorePy/Util.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void audio(py::module_& m)
        {
            using namespace audio;

            auto mAudio = m.def_submodule("audio", "Audio");
            
            py::enum_<DataType>(mAudio, "DataType")
                .value("_None", DataType::None)
                .value("S8", DataType::S8)
                .value("S16", DataType::S16)
                .value("S32", DataType::S32)
                .value("F32", DataType::F32)
                .value("F64", DataType::F64);
            FTK_ENUM_BIND(mAudio, DataType);
            
            /*mAudio.attr("S8Range") = S8Range;
            mAudio.attr("S16Range") = S16Range;
            mAudio.attr("S32Range") = S32Range;
            mAudio.attr("F32Range") = F32Range;
            mAudio.attr("F64Range") = F64Range;*/
            
            mAudio.def("getByteCount", &getByteCount);
            mAudio.def("getIntType", &getIntType);
            mAudio.def("getFloatType", &getFloatType);
            
            py::class_<Info>(mAudio, "Info")
                .def(py::init())
                .def_readwrite("name", &Info::name)
                .def_readwrite("channelCount", &Info::channelCount)
                .def_readwrite("dataType", &Info::dataType)
                .def_readwrite("sampleRate", &Info::sampleRate)
                .def_property_readonly("isValid", &Info::isValid)
                .def_property_readonly("byteCount", &Info::getByteCount)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
            
            py::class_<Audio, std::shared_ptr<Audio> >(mAudio, "Audio")
                .def(py::init(&Audio::create),
                    py::arg("info"),
                    py::arg("sampleCount"))
                .def_property_readonly("info", &Audio::getInfo)
                .def_property_readonly("channelCount", &Audio::getChannelCount)
                .def_property_readonly("dataType", &Audio::getDataType)
                .def_property_readonly("sampleRate", &Audio::getSampleRate)
                .def_property_readonly("sampleCount", &Audio::getSampleCount)
                .def_property_readonly("isValid", &Audio::isValid)
                .def_property_readonly("byteCount", &Audio::getByteCount)
                .def("zero", &Audio::zero);
        }
    }
}
