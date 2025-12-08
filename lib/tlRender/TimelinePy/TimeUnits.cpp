// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/TimelinePy/TimeUnits.h>

#include <tlRender/Timeline/TimeUnits.h>

#include <ftk/CorePy/Util.h>
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
            using namespace timeline;
            
            py::enum_<TimeUnits>(m, "TimeUnits")
                .value("Frames", TimeUnits::Frames)
                .value("Seconds", TimeUnits::Seconds)
                .value("Timecode", TimeUnits::Timecode);
            FTK_ENUM_BIND(m, TimeUnits);

            py::class_<ITimeUnitsModel, std::shared_ptr<ITimeUnitsModel> >(m, "ITimeUnitsModel")
                .def("getLabel", &ITimeUnitsModel::getLabel, py::arg("time"));

            py::class_<TimeUnitsModel, ITimeUnitsModel, std::shared_ptr<TimeUnitsModel> >(m, "TimeUnitsModel")
                .def(py::init(&TimeUnitsModel::create), py::arg("context"))
                .def_property("timeUnits", &TimeUnitsModel::getTimeUnits, &TimeUnitsModel::setTimeUnits);
        }
    }
}
