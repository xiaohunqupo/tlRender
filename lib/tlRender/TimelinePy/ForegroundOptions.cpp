// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/Bindings.h>

#include <tlRender/Timeline/ForegroundOptions.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void foregroundOptions(py::module_& m)
        {
            py::class_<Grid>(m, "Grid")
                .def(py::init())
                .def_readwrite("enabled", &Grid::enabled)
                .def_readwrite("size", &Grid::size)
                .def_readwrite("lineWidth", &Grid::lineWidth)
                .def_readwrite("color", &Grid::color)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<Outline>(m, "Grid")
                .def(py::init())
                .def_readwrite("enabled", &Outline::enabled)
                .def_readwrite("width", &Outline::width)
                .def_readwrite("color", &Outline::color)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            py::class_<ForegroundOptions>(m, "ForegroundOptions")
                .def(py::init())
                .def_readwrite("grid", &ForegroundOptions::grid)
                .def_readwrite("outline", &ForegroundOptions::outline)
                .def(pybind11::self == pybind11::self)
                .def(pybind11::self != pybind11::self);

            m.def("to_json",
                [](const Grid& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });
            m.def("to_json",
                [](const Outline& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });
            m.def("to_json",
                [](const ForegroundOptions& value)
                {
                    nlohmann::json json;
                    to_json(json, value);
                    return json.dump();
                });

            m.def("from_json",
                [](const std::string& value, Grid& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
            m.def("from_json",
                [](const std::string& value, Outline& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
            m.def("from_json",
                [](const std::string& value, ForegroundOptions& out)
                {
                    from_json(nlohmann::json().parse(value), out);
                });
        }
    }
}
