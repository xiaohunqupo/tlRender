// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/Timeline/ColorOptions.h>

#include <ftk/CorePy/Bindings.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void colorOptions(py::module_& m)
        {
            using namespace timeline;
            
            py::enum_<OCIOConfig>(m, "OCIOConfig")
                .value("BuiltIn", OCIOConfig::BuiltIn)
                .value("EnvVar", OCIOConfig::EnvVar)
                .value("File", OCIOConfig::File);
            FTK_ENUM_BIND(m, OCIOConfig);

            py::class_<OCIOOptions>(m, "OCIOOptions")
                .def(py::init())
                .def_readwrite("enabled", &OCIOOptions::enabled)
                .def_readwrite("config", &OCIOOptions::config)
                .def_readwrite("fileName", &OCIOOptions::fileName)
                .def_readwrite("input", &OCIOOptions::input)
                .def_readwrite("display", &OCIOOptions::display)
                .def_readwrite("view", &OCIOOptions::view)
                .def_readwrite("look", &OCIOOptions::look)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            m.def("to_json",
                [](const timeline::OCIOOptions& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });
            m.def("to_json",
                [](const timeline::LUTOptions& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });

            m.def("from_json",
                [](const std::string& value, timeline::OCIOOptions& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
            m.def("from_json",
                [](const std::string& value, timeline::LUTOptions& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
        }
    }
}
