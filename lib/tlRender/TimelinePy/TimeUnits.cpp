// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/TimeUnits.h>

#include <tlRender/Timeline/TimeUnits.h>

#include <ftk/Core/Context.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace tl
{
    namespace python
    {
        void timeUnits(py::module_& m)
        {
            py::enum_<timeline::TimeUnits>(m, "TimeUnits")
                .value("Frames", timeline::TimeUnits::Frames)
                .value("Seconds", timeline::TimeUnits::Seconds)
                .value("Timecode", timeline::TimeUnits::Timecode);
            m.def("getTimeUnitsLabels", &timeline::getTimeUnitsLabels);
            m.def("timeUnitsToString", py::overload_cast<timeline::TimeUnits>(&timeline::to_string));
            m.def("timeUnitsFromString",
                [](const std::string& s)
                {
                    timeline::TimeUnits value = timeline::TimeUnits::First;
                    return std::make_pair(timeline::from_string(s, value), value);
                });

            py::class_<timeline::ITimeUnitsModel, std::shared_ptr<timeline::ITimeUnitsModel> >(m, "ITimeUnitsModel")
                .def("getLabel", &timeline::ITimeUnitsModel::getLabel, py::arg("time"));

            py::class_<timeline::TimeUnitsModel, timeline::ITimeUnitsModel, std::shared_ptr<timeline::TimeUnitsModel> >(m, "TimeUnitsModel")
                .def(py::init(&timeline::TimeUnitsModel::create), py::arg("context"))
                .def_property("timeUnits", &timeline::TimeUnitsModel::getTimeUnits, &timeline::TimeUnitsModel::setTimeUnits);
        }
    }
}
