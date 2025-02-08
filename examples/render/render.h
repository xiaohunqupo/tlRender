// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

#include <dtk/ui/App.h>
#include <dtk/ui/MainWindow.h>

namespace dtk
{
    namespace gl
    {
        class Window;
    }
}

namespace tl
{
    namespace examples
    {
        //! Example rendering application.
        namespace render
        {
            //! Application options.
            struct Options
            {
                std::string compareFileName;
                dtk::Size2I windowSize = dtk::Size2I(1920, 1080);
                bool fullscreen = false;
                bool hud = true;
                timeline::Playback playback = timeline::Playback::Forward;
                OTIO_NS::RationalTime seek = time::invalidTime;
                OTIO_NS::TimeRange inOutRange = time::invalidTimeRange;
                timeline::OCIOOptions ocioOptions;
                timeline::LUTOptions lutOptions;
            };

            //! Window.
            class MainWindow : public dtk::MainWindow
            {
                DTK_NON_COPYABLE(MainWindow);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<dtk::App>&);

                MainWindow() = default;

            public:
                ~MainWindow();

                static std::shared_ptr<MainWindow> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<dtk::App>&);

            protected:
                std::shared_ptr<dtk::IRender> _createRender(const std::shared_ptr<dtk::Context>&) override;
            };

            //! Application.
            class App : public dtk::App
            {
                DTK_NON_COPYABLE(App);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::vector<std::string>&);

                App();

            public:
                ~App();

                //! Create a new application.
                static std::shared_ptr<App> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::vector<std::string>&);

                void run() override;

            private:
                void _keyCallback(int, int, int, int);

                void _printShortcutsHelp();

                void _tick();

                void _draw();
                void _drawViewport(
                    const dtk::Box2I&,
                    uint16_t fontSize,
                    const timeline::CompareOptions&,
                    float rotation);

                void _hudCallback(bool);

                void _playbackCallback(timeline::Playback);

                std::string _input;
                Options _options;

                std::shared_ptr<timeline::Player> _player;

                std::shared_ptr<dtk::gl::Window> _window;
                dtk::Size2I _frameBufferSize;
                dtk::V2F _contentScale = dtk::V2F(1.F, 1.F);
                timeline::CompareOptions _compareOptions;
                float _rotation = 0.F;
                bool _hud = false;
                std::shared_ptr<timeline::IRender> _render;
                bool _renderDirty = true;
                std::vector<timeline::VideoData> _videoData;
                std::shared_ptr<dtk::ListObserver<timeline::VideoData> > _videoDataObserver;
                std::chrono::steady_clock::time_point _startTime;

                bool _running = true;
            };
        }
    }
}
