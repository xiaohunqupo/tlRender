// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimeEdit.h>
#include <tlTimelineUI/TimeLabel.h>

#include <tlTimeline/Player.h>

#include <feather-tk/ui/TabBar.h>

namespace tl
{
    namespace play
    {
        class App;

        //! Tab bar.
        class TabBar : public feather_tk::IWidget
        {
            FEATHER_TK_NON_COPYABLE(TabBar);

        protected:
            void _init(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent);

            TabBar() = default;

        public:
            ~TabBar();

            static std::shared_ptr<TabBar> create(
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const feather_tk::Box2I&) override;
            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;

        private:
            std::shared_ptr<feather_tk::TabBar> _tabBar;
            std::shared_ptr<feather_tk::ListObserver<std::shared_ptr<timeline::Player> > > _playersObserver;
            std::shared_ptr<feather_tk::ValueObserver<int> > _playerIndexObserver;
        };
    }
}