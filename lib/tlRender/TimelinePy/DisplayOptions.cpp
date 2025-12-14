// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/Timeline/DisplayOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void displayOptions(py::module_& m)
        {
            py::class_<timeline::Color>(m, "Color")
                .def(py::init())
                .def_readwrite("enabled", &timeline::Color::enabled)
                .def_readwrite("add", &timeline::Color::add)
                .def_readwrite("brightness", &timeline::Color::brightness)
                .def_readwrite("contrast", &timeline::Color::contrast)
                .def_readwrite("saturation", &timeline::Color::saturation)
                .def_readwrite("tint", &timeline::Color::tint)
                .def_readwrite("invert", &timeline::Color::invert)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);
            
            m.def("brightness", &timeline::brightness);
            m.def("contrast", &timeline::contrast);
            m.def("saturation", &timeline::saturation);
            m.def("tint", &timeline::tint);
            m.def("color", &timeline::color);

            py::class_<timeline::Levels>(m, "Levels")
                .def(py::init())
                .def_readwrite("enabled", &timeline::Levels::enabled)
                .def_readwrite("inLow", &timeline::Levels::inLow)
                .def_readwrite("inHigh", &timeline::Levels::inHigh)
                .def_readwrite("gamma", &timeline::Levels::gamma)
                .def_readwrite("outLow", &timeline::Levels::outLow)
                .def_readwrite("outHigh", &timeline::Levels::outHigh)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<timeline::EXRDisplay>(m, "EXRDisplay")
                .def(py::init())
                .def_readwrite("enabled", &timeline::EXRDisplay::enabled)
                .def_readwrite("exposure", &timeline::EXRDisplay::exposure)
                .def_readwrite("defog", &timeline::EXRDisplay::defog)
                .def_readwrite("kneeLow", &timeline::EXRDisplay::kneeLow)
                .def_readwrite("kneeHigh", &timeline::EXRDisplay::kneeHigh)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<timeline::SoftClip>(m, "SoftClip")
                .def(py::init())
                .def_readwrite("enabled", &timeline::SoftClip::enabled)
                .def_readwrite("value", &timeline::SoftClip::value)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<timeline::DisplayOptions>(m, "DisplayOptions")
                .def(py::init())
                .def_readwrite("channels", &timeline::DisplayOptions::channels)
                .def_readwrite("mirror", &timeline::DisplayOptions::mirror)
                .def_readwrite("color", &timeline::DisplayOptions::color)
                .def_readwrite("levels", &timeline::DisplayOptions::levels)
                .def_readwrite("exrDisplay", &timeline::DisplayOptions::exrDisplay)
                .def_readwrite("softClip", &timeline::DisplayOptions::softClip)
                .def_readwrite("imageFilters", &timeline::DisplayOptions::imageFilters)
                .def_readwrite("videoLevels", &timeline::DisplayOptions::videoLevels)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            m.def("to_json",
                [](const timeline::Color& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });
            m.def("to_json",
                [](const timeline::Levels& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });
            m.def("to_json",
                [](const timeline::EXRDisplay& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });
            m.def("to_json",
                [](const timeline::SoftClip& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });
            m.def("to_json",
                [](const timeline::DisplayOptions& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });

            m.def("from_json",
                [](const std::string& value, timeline::Color& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
            m.def("from_json",
                [](const std::string& value, timeline::Levels& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
            m.def("from_json",
                [](const std::string& value, timeline::EXRDisplay& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
            m.def("from_json",
                [](const std::string& value, timeline::SoftClip& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
            m.def("from_json",
                [](const std::string& value, timeline::DisplayOptions& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
        }
    }
}
