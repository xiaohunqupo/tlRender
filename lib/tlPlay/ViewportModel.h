// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        //! Viewport model.
        class ViewportModel : public std::enable_shared_from_this<ViewportModel>
        {
            TLRENDER_NON_COPYABLE(ViewportModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            ViewportModel();

        public:
            ~ViewportModel();

            //! Create a new model.
            static std::shared_ptr<ViewportModel> create(const std::shared_ptr<system::Context>&);

            //! Get the timeline viewport background options.
            const timelineui::ViewportBackgroundOptions& getBackgroundOptions() const;

            //! Observer the timeline viewport background options.
            std::shared_ptr<observer::IValue<timelineui::ViewportBackgroundOptions> > observeBackgroundOptions() const;

            //! Set the timeline viewport background options.
            void setBackgroundOptions(const timelineui::ViewportBackgroundOptions&);

        private:
            TLRENDER_PRIVATE();
        };
    }
}