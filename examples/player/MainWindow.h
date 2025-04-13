// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/TimelineWidget.h>
#include <tlTimelineUI/Viewport.h>
#include <tlTimelineUI/Window.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/MainWindow.h>
#include <dtk/ui/MenuBar.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/Splitter.h>

namespace tl
{
    namespace examples
    {
        namespace player
        {
            class App;
            class MenuBar;
            class PlaybackBar;
            class ToolBar;

            //! Main window.
            class MainWindow : public timelineui::Window
            {
                DTK_NON_COPYABLE(MainWindow);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                MainWindow() = default;

            public:
                ~MainWindow();

                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<App>&);

                void setPlayer(const std::shared_ptr<timeline::Player>&);

                void keyPressEvent(dtk::KeyEvent&) override;
                void keyReleaseEvent(dtk::KeyEvent&) override;

            private:
                std::shared_ptr<timeline::TimeUnitsModel> _timeUnitsModel;
                std::shared_ptr<MenuBar> _menuBar;
                std::shared_ptr<ToolBar> _toolBar;
                std::shared_ptr<timelineui::Viewport> _viewport;
                std::shared_ptr<PlaybackBar> _playbackBar;
                std::shared_ptr<timelineui::TimelineWidget> _timelineWidget;
                std::shared_ptr<dtk::Splitter> _splitter;
                std::shared_ptr<dtk::VerticalLayout> _layout;
            };
        }
    }
}
