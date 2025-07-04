// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimeUnits.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

#include <cstdlib>

namespace tl
{
    namespace timeline
    {
        FEATHER_TK_ENUM_IMPL(
            TimeUnits,
            "Frames",
            "Seconds",
            "Timecode");

        std::string timeToText(const OTIO_NS::RationalTime& time, timeline::TimeUnits units)
        {
            std::string out;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
                out = feather_tk::Format("{0}").
                    arg(time::isValid(time) ? time.to_frames() : 0);
                break;
            case timeline::TimeUnits::Seconds:
                out = feather_tk::Format("{0}").
                    arg(time::isValid(time) ? time.to_seconds() : 0.0, 2);
                break;
            case timeline::TimeUnits::Timecode:
            {
                if (time::isValid(time))
                {
                    out = time.to_timecode();
                }
                if (out.empty())
                {
                    out = "--:--:--:--";
                }
                break;
            }
            default: break;
            }
            return out;
        }

        OTIO_NS::RationalTime textToTime(
            const std::string& text,
            double rate,
            timeline::TimeUnits units,
            opentime::ErrorStatus* errorStatus)
        {
            OTIO_NS::RationalTime out = time::invalidTime;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
            {
                const int value = std::atoi(text.c_str());
                out = OTIO_NS::RationalTime::from_frames(value, rate);
                break;
            }
            case timeline::TimeUnits::Seconds:
            {
                const double value = std::atof(text.c_str());
                out = OTIO_NS::RationalTime::from_seconds(value).rescaled_to(rate);
                break;
            }
            case timeline::TimeUnits::Timecode:
                out = OTIO_NS::RationalTime::from_timecode(text, rate, errorStatus);
                break;
            default: break;
            }
            return out;
        }

        std::string formatString(timeline::TimeUnits units)
        {
            std::string out;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
                out = "000000";
                break;
            case timeline::TimeUnits::Seconds:
                out = "000000.00";
                break;
            case timeline::TimeUnits::Timecode:
                out = "00:00:00;00";
                break;
            default: break;
            }
            return out;
        }

        std::string validator(timeline::TimeUnits units)
        {
            std::string out;
            switch (units)
            {
            case timeline::TimeUnits::Frames:
                out = "[0-9]*";
                break;
            case timeline::TimeUnits::Seconds:
                out = "[0-9]*\\.[0-9]+|[0-9]+";
                break;
            case timeline::TimeUnits::Timecode:
                out = "[0-9][0-9]:[0-9][0-9]:[0-9][0-9]:[0-9][0-9]";
                break;
            default: break;
            }
            return out;
        }

        void ITimeUnitsModel::_init(const std::shared_ptr<feather_tk::Context>& context)
        {
            _timeUnitsChanged = feather_tk::ObservableValue<bool>::create();
        }

        ITimeUnitsModel::ITimeUnitsModel()
        {}

        ITimeUnitsModel::~ITimeUnitsModel()
        {}

        std::shared_ptr<feather_tk::IObservableValue<bool> > ITimeUnitsModel::observeTimeUnitsChanged() const
        {
            return _timeUnitsChanged;
        }

        struct TimeUnitsModel::Private
        {
            std::shared_ptr<feather_tk::ObservableValue<TimeUnits> > timeUnits;
        };

        void TimeUnitsModel::_init(const std::shared_ptr<feather_tk::Context>& context)
        {
            FEATHER_TK_P();
            ITimeUnitsModel::_init(context);
            p.timeUnits = feather_tk::ObservableValue<TimeUnits>::create(TimeUnits::Timecode);
        }

        TimeUnitsModel::TimeUnitsModel() :
            _p(new Private)
        {}

        TimeUnitsModel::~TimeUnitsModel()
        {}

        std::shared_ptr<TimeUnitsModel> TimeUnitsModel::create(
            const std::shared_ptr<feather_tk::Context>& context)
        {
            auto out = std::shared_ptr<TimeUnitsModel>(new TimeUnitsModel);
            out->_init(context);
            return out;
        }

        TimeUnits TimeUnitsModel::getTimeUnits() const
        {
            return _p->timeUnits->get();
        }

        std::shared_ptr<feather_tk::IObservableValue<TimeUnits> > TimeUnitsModel::observeTimeUnits() const
        {
            return _p->timeUnits;
        }

        void TimeUnitsModel::setTimeUnits(TimeUnits value)
        {
            if (_p->timeUnits->setIfChanged(value))
            {
                _timeUnitsChanged->setAlways(true);
            }
        }

        std::string TimeUnitsModel::getLabel(const OTIO_NS::RationalTime& value) const
        {
            return timeToText(value, _p->timeUnits->get());
        }
    }
}
