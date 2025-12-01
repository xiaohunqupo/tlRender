// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/CorePy/Audio.h>

#include <tlRender/Core/Audio.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void audio(py::module_& m)
        {
            auto mAudio = m.def_submodule("audio", "Audio");
            
            py::enum_<audio::DataType>(mAudio, "DataType")
                .value("_None", audio::DataType::None)
                .value("S8", audio::DataType::S8)
                .value("S16", audio::DataType::S16)
                .value("S32", audio::DataType::S32)
                .value("F32", audio::DataType::F32)
                .value("F64", audio::DataType::F64);
            
            /*mAudio.attr("S8Range") = audio::S8Range;
            mAudio.attr("S16Range") = audio::S16Range;
            mAudio.attr("S32Range") = audio::S32Range;
            mAudio.attr("F32Range") = audio::F32Range;
            mAudio.attr("F64Range") = audio::F64Range;*/
            
            mAudio.def("getByteCount", &audio::getByteCount);
            mAudio.def("getIntType", &audio::getIntType);
            mAudio.def("getFloatType", &audio::getFloatType);
            
            py::class_<audio::Info>(mAudio, "Info")
                .def(py::init())
                .def_readwrite("name", &audio::Info::name)
                .def_readwrite("channelCount", &audio::Info::channelCount)
                .def_readwrite("dataType", &audio::Info::dataType)
                .def_readwrite("sampleRate", &audio::Info::sampleRate)
                .def_property_readonly("isValid", &audio::Info::isValid)
                .def_property_readonly("byteCount", &audio::Info::getByteCount)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
            
            py::class_<audio::Audio, std::shared_ptr<audio::Audio> >(mAudio, "Audio")
                .def(py::init(&audio::Audio::create),
                    py::arg("info"),
                    py::arg("sampleCount"))
                .def_property_readonly("info", &audio::Audio::getInfo)
                .def_property_readonly("channelCount", &audio::Audio::getChannelCount)
                .def_property_readonly("dataType", &audio::Audio::getDataType)
                .def_property_readonly("sampleRate", &audio::Audio::getSampleRate)
                .def_property_readonly("sampleCount", &audio::Audio::getSampleCount)
                .def_property_readonly("isValid", &audio::Audio::isValid)
                .def_property_readonly("byteCount", &audio::Audio::getByteCount)
                .def("zero", &audio::Audio::zero);
        }
    }
}
