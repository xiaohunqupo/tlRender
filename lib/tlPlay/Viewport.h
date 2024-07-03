// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace play
    {
        class Viewport : public timelineui::TimelineViewport
        {
            TLRENDER_NON_COPYABLE(Viewport);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Viewport();

        public:
            virtual ~Viewport();

            static std::shared_ptr<Viewport> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get whether the HUD is enabled.
            bool hasHUD() const;

            //! Observe whether the HUD is enabled.
            std::shared_ptr<observer::IValue<bool> > observeHUD() const;

            //! Set whether the HUD is enabled.
            void setHUD(bool);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}

