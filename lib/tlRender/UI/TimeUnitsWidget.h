// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/TimeUnits.h>

#include <ftk/UI/IWidget.h>

namespace tl
{
    namespace timelineui
    {
        //! Time units widget.
        class TimeUnitsWidget : public ftk::IWidget
        {
            FTK_NON_COPYABLE(TimeUnitsWidget);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent);

            TimeUnitsWidget();

        public:
            virtual ~TimeUnitsWidget();

            //! Create a new widget.
            static std::shared_ptr<TimeUnitsWidget> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<timeline::TimeUnitsModel>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            void _widgetUpdate();

            FTK_PRIVATE();
        };
    }
}
