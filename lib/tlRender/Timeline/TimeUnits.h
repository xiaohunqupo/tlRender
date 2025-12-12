// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Time.h>
#include <tlRender/Core/Util.h>

#include <ftk/Core/Observable.h>

namespace ftk
{
    class Context;
}

namespace tl
{
    namespace timeline
    {
        //! Time units.
        enum class TL_API_TYPE TimeUnits
        {
            Frames,
            Seconds,
            Timecode,

            Count,
            First = Frames
        };
        TL_ENUM(TimeUnits);

        //! Convert a time value to text.
        TL_API std::string timeToText(const OTIO_NS::RationalTime&, timeline::TimeUnits);

        //! Convert text to a time value.
        TL_API OTIO_NS::RationalTime textToTime(
            const std::string&     text,
            double                 rate,
            timeline::TimeUnits    units,
            opentime::ErrorStatus* error = nullptr);

        //! Get a time units format string.
        TL_API TL_API std::string formatString(timeline::TimeUnits);

        //! Get a time units validator regular expression.
        TL_API std::string validator(timeline::TimeUnits);

        //! Base class for time units models.
        class TL_API_TYPE ITimeUnitsModel : public std::enable_shared_from_this<ITimeUnitsModel>
        {
            FTK_NON_COPYABLE(ITimeUnitsModel);

        protected:
            void _init(const std::shared_ptr<ftk::Context>&);

            ITimeUnitsModel();

        public:
            TL_API virtual ~ITimeUnitsModel() = 0;

            //! Observer when the time units are changed.
            TL_API std::shared_ptr<ftk::IObservable<bool> > observeTimeUnitsChanged() const;

            //! Get a time label in the current time units.
            TL_API virtual std::string getLabel(const OTIO_NS::RationalTime&) const = 0;

        protected:
            std::shared_ptr<ftk::Observable<bool> > _timeUnitsChanged;
        };

        //! Time units model.
        class TL_API_TYPE TimeUnitsModel : public ITimeUnitsModel
        {
            FTK_NON_COPYABLE(TimeUnitsModel);

        protected:
            void _init(const std::shared_ptr<ftk::Context>&);

            TimeUnitsModel();

        public:
            TL_API virtual ~TimeUnitsModel();

            //! Create a new model.
            TL_API static std::shared_ptr<TimeUnitsModel> create(
                const std::shared_ptr<ftk::Context>&);

            //! Get the time units.
            TL_API TimeUnits getTimeUnits() const;

            //! Observer the time units.
            TL_API std::shared_ptr<ftk::IObservable<TimeUnits> > observeTimeUnits() const;
            
            //! Set the time units.
            TL_API void setTimeUnits(TimeUnits);

            TL_API std::string getLabel(const OTIO_NS::RationalTime&) const override;

        private:
            FTK_PRIVATE();
        };
    }
}
