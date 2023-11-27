// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/App.h>

#include <tlPlayGLApp/MainWindow.h>
#include <tlPlayGLApp/SecondaryWindow.h>
#include <tlPlayGLApp/SeparateAudioDialog.h>
#include <tlPlayGLApp/Style.h>
#include <tlPlayGLApp/Tools.h>

#include <tlPlay/App.h>
#include <tlPlay/AudioModel.h>
#include <tlPlay/ColorModel.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/Settings.h>
#include <tlPlay/ViewportModel.h>
#include <tlPlay/Util.h>

#include <tlUI/FileBrowser.h>
#include <tlUI/RecentFilesModel.h>

#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/AudioSystem.h>
#include <tlCore/File.h>
#include <tlCore/FileLogSystem.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        struct App::Private
        {
            play::Options options;
            std::shared_ptr<file::FileLogSystem> fileLogSystem;
            std::string settingsFileName;
            std::shared_ptr<play::Settings> settings;
            std::shared_ptr<play::FilesModel> filesModel;
            std::vector<std::shared_ptr<play::FilesModelItem> > files;
            std::vector<std::shared_ptr<play::FilesModelItem> > activeFiles;
            std::vector<std::shared_ptr<timeline::Player> > players;
            std::shared_ptr<observer::List<std::shared_ptr<timeline::Player> > > activePlayers;
            std::shared_ptr<play::ViewportModel> viewportModel;
            std::shared_ptr<play::ColorModel> colorModel;
            bool deviceActive = false;
            audio::Info audioInfo;
            std::shared_ptr<play::AudioModel> audioModel;
            std::shared_ptr<ToolsModel> toolsModel;

            std::shared_ptr<observer::Value<bool> > secondaryWindowActive;
            std::shared_ptr<MainWindow> mainWindow;
            std::shared_ptr<SecondaryWindow> secondaryWindow;
            std::shared_ptr<SeparateAudioDialog> separateAudioDialog;

            std::shared_ptr<observer::ValueObserver<std::string> > settingsObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > activeObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<size_t> > recentFilesMaxObserver;
            std::shared_ptr<observer::ListObserver<file::Path> > recentFilesObserver;
            std::shared_ptr<observer::ValueObserver<bool> > mainWindowObserver;
            std::shared_ptr<observer::ValueObserver<bool> > secondaryWindowObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ColorConfigOptions> > colorConfigOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> > lutOptionsObserver;
            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
            std::shared_ptr<observer::ValueObserver<bool> > muteObserver;
            std::shared_ptr<observer::ValueObserver<double> > syncOffsetObserver;
        };

        void App::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            const std::string appName = "tlplay-gl";
            const std::string appDocsPath = play::appDocsPath();
            const std::string logFileName = play::logFileName(appName, appDocsPath);
            const std::string settingsFileName = play::settingsName(appName, appDocsPath);
            IApp::_init(
                argv,
                context,
                appName,
                "Example GLFW playback application.",
                play::getCmdLineArgs(p.options),
                play::getCmdLineOptions(p.options, logFileName, settingsFileName));
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            _fileLogInit(logFileName);
            _settingsInit(settingsFileName);
            _modelsInit();
            _observersInit();
            _inputFilesInit();
            _windowsInit();
        }

        App::App() :
            _p(new Private)
        {}

        App::~App()
        {
            TLRENDER_P();
            if (p.settings)
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                p.settings->setValue("FileBrowser/Path", fileBrowserSystem->getPath());
                p.settings->setValue("FileBrowser/Options", fileBrowserSystem->getOptions());
            }
        }

        std::shared_ptr<App> App::create(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(argv, context);
            return out;
        }

        void App::openDialog()
        {
            TLRENDER_P();
            auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
            fileBrowserSystem->open(
                p.mainWindow,
                [this](const file::FileInfo& value)
                {
                    open(value.getPath());
                });
        }

        void App::openSeparateAudioDialog()
        {
            TLRENDER_P();
            p.separateAudioDialog = SeparateAudioDialog::create(_context);
            p.separateAudioDialog->open(p.mainWindow);
            p.separateAudioDialog->setCallback(
                [this](const file::Path& value, const file::Path& audio)
                {
                    open(value, audio);
                    _p->separateAudioDialog->close();
                });
            p.separateAudioDialog->setCloseCallback(
                [this]
                {
                    _p->separateAudioDialog.reset();
                });
        }

        void App::open(const file::Path& path, const file::Path& audioPath)
        {
            TLRENDER_P();
            file::PathOptions pathOptions;
            pathOptions.maxNumberDigits = p.settings->getValue<size_t>("FileSequence/MaxDigits");
            for (const auto& i : timeline::getPaths(path, pathOptions, _context))
            {
                auto item = std::make_shared<play::FilesModelItem>();
                item->path = i;
                item->audioPath = audioPath;
                p.filesModel->add(item);

                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
                recentFilesModel->addRecent(path);
            }
        }

        const std::shared_ptr<play::Settings>& App::getSettings() const
        {
            return _p->settings;
        }

        const std::shared_ptr<play::FilesModel>& App::getFilesModel() const
        {
            return _p->filesModel;
        }

        std::shared_ptr<observer::IList<std::shared_ptr<timeline::Player> > > App::observeActivePlayers() const
        {
            return _p->activePlayers;
        }

        const std::shared_ptr<play::ViewportModel>& App::getViewportModel() const
        {
            return _p->viewportModel;
        }

        const std::shared_ptr<play::ColorModel>& App::getColorModel() const
        {
            return _p->colorModel;
        }

        const std::shared_ptr<play::AudioModel>& App::getAudioModel() const
        {
            return _p->audioModel;
        }

        const std::shared_ptr<ToolsModel>& App::getToolsModel() const
        {
            return _p->toolsModel;
        }

        const std::shared_ptr<MainWindow>& App::getMainWindow() const
        {
            return _p->mainWindow;
        }

        std::shared_ptr<observer::IValue<bool> > App::observeSecondaryWindow() const
        {
            return _p->secondaryWindowActive;
        }

        void App::setSecondaryWindow(bool value)
        {
            TLRENDER_P();
            if (p.secondaryWindowActive->setIfChanged(value))
            {
                if (value)
                {
                    std::vector<int> screens;
                    for (int i = 0; i < getScreenCount(); ++i)
                    {
                        screens.push_back(i);
                    }
                    auto i = std::find(
                        screens.begin(),
                        screens.end(),
                        p.mainWindow->getScreen());
                    if (i != screens.end())
                    {
                        screens.erase(i);
                    }
                    if (!screens.empty())
                    {
                        p.secondaryWindow = SecondaryWindow::create(
                            std::dynamic_pointer_cast<App>(shared_from_this()),
                            _context);
                        addWindow(p.secondaryWindow);
                        p.secondaryWindow->setFullScreen(true, screens.front());
                        p.secondaryWindow->show();

                        p.secondaryWindowObserver = observer::ValueObserver<bool>::create(
                            p.secondaryWindow->observeClose(),
                            [this](bool value)
                            {
                                if (value)
                                {
                                    _p->secondaryWindowActive->setIfChanged(false);
                                    _p->secondaryWindow.reset();
                                    _p->secondaryWindowObserver.reset();
                                }
                            });
                    }
                }
                else
                {
                    removeWindow(p.secondaryWindow);
                    p.secondaryWindow.reset();
                    p.secondaryWindowObserver.reset();
                }
            }
        }

        void App::_tick()
        {
            TLRENDER_P();
            for (const auto& player : p.players)
            {
                if (player)
                {
                    player->tick();
                }
            }
        }

        void App::_fileLogInit(const std::string& logFileName)
        {
            TLRENDER_P();
            std::string logFileName2 = logFileName;
            if (!p.options.logFileName.empty())
            {
                logFileName2 = p.options.logFileName;
            }
            p.fileLogSystem = file::FileLogSystem::create(logFileName2, _context);
        }

        void App::_settingsInit(const std::string& settingsFileName)
        {
            TLRENDER_P();
            if (!p.options.settingsFileName.empty())
            {
                p.settingsFileName = p.options.settingsFileName;
            }
            else
            {
                p.settingsFileName = settingsFileName;
            }
            p.settings = play::Settings::create(
                p.settingsFileName,
                p.options.resetSettings,
                _context);
            p.settings->setDefaultValue("Files/RecentMax", 10);
            p.settings->setDefaultValue("Window/Size", math::Size2i(1920, 1080));
            p.settings->setDefaultValue("Cache/Size", 1);
            p.settings->setDefaultValue("Cache/ReadAhead", 2.0);
            p.settings->setDefaultValue("Cache/ReadBehind", 0.5);
            p.settings->setDefaultValue("FileSequence/Audio",
                timeline::FileSequenceAudio::BaseName);
            p.settings->setDefaultValue("FileSequence/AudioFileName", std::string());
            p.settings->setDefaultValue("FileSequence/AudioDirectory", std::string());
            p.settings->setDefaultValue("FileSequence/MaxDigits", 9);
            p.settings->setDefaultValue("SequenceIO/ThreadCount", 16);
#if defined(TLRENDER_FFMPEG)
            p.settings->setDefaultValue("FFmpeg/YUVToRGBConversion", false);
            p.settings->setDefaultValue("FFmpeg/ThreadCount", 0);
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
            p.settings->setDefaultValue("USD/renderWidth", p.options.usdRenderWidth);
            p.settings->setDefaultValue("USD/complexity", p.options.usdComplexity);
            p.settings->setDefaultValue("USD/drawMode", p.options.usdDrawMode);
            p.settings->setDefaultValue("USD/enableLighting", p.options.usdEnableLighting);
            p.settings->setDefaultValue("USD/sRGB", p.options.usdSRGB);
            p.settings->setDefaultValue("USD/stageCacheCount", p.options.usdStageCache);
            p.settings->setDefaultValue("USD/diskCacheByteCount", p.options.usdDiskCache);
#endif // TLRENDER_USD
            p.settings->setDefaultValue("FileBrowser/NativeFileDialog", true);
            p.settings->setDefaultValue("FileBrowser/Path", file::getCWD());
            p.settings->setDefaultValue("FileBrowser/Options", ui::FileBrowserOptions());
            p.settings->setDefaultValue("Performance/TimerMode",
                timeline::PlayerOptions().timerMode);
            p.settings->setDefaultValue("Performance/AudioBufferFrameCount",
                timeline::PlayerOptions().audioBufferFrameCount);
            p.settings->setDefaultValue("Performance/VideoRequestCount", 16);
            p.settings->setDefaultValue("Performance/AudioRequestCount", 16);
            p.settings->setDefaultValue("Style/Palette", StylePalette::First);
            p.settings->setDefaultValue("Misc/ToolTipsEnabled", true);
        }

        void App::_modelsInit()
        {
            TLRENDER_P();
            
            p.filesModel = play::FilesModel::create(_context);

            p.viewportModel = play::ViewportModel::create(p.settings, _context);

            p.colorModel = play::ColorModel::create(_context);
            p.colorModel->setColorConfigOptions(p.options.colorConfigOptions);
            p.colorModel->setLUTOptions(p.options.lutOptions);

            if (auto audioSystem = _context->getSystem<audio::System>())
            {
                p.audioInfo = audioSystem->getDefaultOutputInfo();
            }
            p.audioModel = play::AudioModel::create(p.settings, _context);

            p.toolsModel = ToolsModel::create();
        }

        void App::_observersInit()
        {
            TLRENDER_P();
            p.settingsObserver = observer::ValueObserver<std::string>::create(
                p.settings->observeValues(),
                [this](const std::string& name)
                {
                    _settingsUpdate(name);
                });

            p.activePlayers = observer::List<std::shared_ptr<timeline::Player> >::create();

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                p.filesModel->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesCallback(value);
                });
            p.activeObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                p.filesModel->observeActive(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _activeCallback(value);
                });
            p.layersObserver = observer::ListObserver<int>::create(
                p.filesModel->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    for (size_t i = 0; i < value.size() && i < _p->players.size(); ++i)
                    {
                        if (auto player = _p->players[i])
                        {
                            auto ioOptions = _getIOOptions();
                            ioOptions["Layer"] = string::Format("{0}").arg(value[i]);
                            player->setIOOptions(ioOptions);
                        }
                    }
                });

            auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
            auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
            p.recentFilesMaxObserver = observer::ValueObserver<size_t>::create(
                recentFilesModel->observeRecentMax(),
                [this](size_t value)
                {
                    _p->settings->setValue("Files/RecentMax", value);
                });
            p.recentFilesObserver = observer::ListObserver<file::Path>::create(
                recentFilesModel->observeRecent(),
                [this](const std::vector<file::Path>& value)
                {
                    std::vector<std::string> fileNames;
                    for (const auto& i : value)
                    {
                        fileNames.push_back(i.get());
                    }
                    _p->settings->setValue("Files/Recent", fileNames);
                });

            p.colorConfigOptionsObserver = observer::ValueObserver<timeline::ColorConfigOptions>::create(
                p.colorModel->observeColorConfigOptions(),
                [this](const timeline::ColorConfigOptions& value)
                {
                    _setColorConfigOptions(value);
                });
            p.lutOptionsObserver = observer::ValueObserver<timeline::LUTOptions>::create(
                p.colorModel->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _setLUTOptions(value);
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                p.audioModel->observeVolume(),
                [this](float)
                {
                    _audioUpdate();
                });
            p.muteObserver = observer::ValueObserver<bool>::create(
                p.audioModel->observeMute(),
                [this](bool)
                {
                    _audioUpdate();
                });
            p.syncOffsetObserver = observer::ValueObserver<double>::create(
                p.audioModel->observeSyncOffset(),
                [this](double)
                {
                    _audioUpdate();
                });
        }

        void App::_inputFilesInit()
        {
            TLRENDER_P();
            if (!p.options.fileName.empty())
            {
                if (!p.options.compareFileName.empty())
                {
                    open(file::Path(p.options.compareFileName));
                    p.filesModel->setCompareOptions(p.options.compareOptions);
                    p.filesModel->setB(0, true);
                }

                open(
                    file::Path(p.options.fileName),
                    file::Path(p.options.audioFileName));

                if (!p.players.empty())
                {
                    if (auto player = p.players[0])
                    {
                        if (p.options.speed > 0.0)
                        {
                            player->setSpeed(p.options.speed);
                        }
                        if (time::isValid(p.options.inOutRange))
                        {
                            player->setInOutRange(p.options.inOutRange);
                            player->seek(p.options.inOutRange.start_time());
                        }
                        if (time::isValid(p.options.seek))
                        {
                            player->seek(p.options.seek);
                        }
                        player->setLoop(p.options.loop);
                        player->setPlayback(p.options.playback);
                    }
                }
            }
        }

        void App::_windowsInit()
        {
            TLRENDER_P();

            p.secondaryWindowActive = observer::Value<bool>::create(false);

            p.mainWindow = MainWindow::create(
                std::dynamic_pointer_cast<App>(shared_from_this()),
                _context);
            addWindow(p.mainWindow);
            p.mainWindow->setWindowSize(
                _options.windowSize.isValid() ?
                _options.windowSize :
                p.settings->getValue<math::Size2i>("Window/Size"));
            p.mainWindow->setFullScreen(_options.fullscreen);
            p.mainWindow->show();

            p.mainWindowObserver = observer::ValueObserver<bool>::create(
                p.mainWindow->observeClose(),
                [this](bool value)
                {
                    if (value)
                    {
                        removeWindow(_p->secondaryWindow);
                        _p->secondaryWindow.reset();
                        _p->secondaryWindowObserver.reset();
                    }
                });
        }

        io::Options App::_getIOOptions() const
        {
            TLRENDER_P();
            io::Options out;

            out["SequenceIO/ThreadCount"] = string::Format("{0}").
                arg(p.settings->getValue<int>("SequenceIO/ThreadCount"));

#if defined(TLRENDER_FFMPEG)
            out["FFmpeg/YUVToRGBConversion"] = string::Format("{0}").
                arg(p.settings->getValue<bool>("FFmpeg/YUVToRGBConversion"));
            out["FFmpeg/AudioChannelCount"] = string::Format("{0}").
                arg(p.audioInfo.channelCount);
            out["FFmpeg/AudioDataType"] = string::Format("{0}").
                arg(p.audioInfo.dataType);
            out["FFmpeg/AudioSampleRate"] = string::Format("{0}").
                arg(p.audioInfo.sampleRate);
            out["FFmpeg/ThreadCount"] = string::Format("{0}").
                arg(p.settings->getValue<int>("FFmpeg/ThreadCount"));
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            {
                std::stringstream ss;
                ss << p.settings->getValue<int>("USD/renderWidth");
                out["USD/renderWidth"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<float>("USD/complexity");
                out["USD/complexity"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<usd::DrawMode>("USD/drawMode");
                out["USD/drawMode"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<bool>("USD/enableLighting");
                out["USD/enableLighting"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<bool>("USD/sRGB");
                out["USD/sRGB"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<size_t>("USD/stageCacheCount");
                out["USD/stageCacheCount"] = ss.str();
            }
            {
                std::stringstream ss;
                ss << p.settings->getValue<size_t>("USD/diskCacheByteCount");
                out["USD/diskCacheByteCount"] = ss.str();
            }
#endif // TLRENDER_USD

            return out;
        }

        std::vector<std::shared_ptr<timeline::Player> > App::_getActivePlayers() const
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<timeline::Player> > out;
            for (size_t i = 0; i < p.activeFiles.size(); ++i)
            {
                const auto j = std::find(
                    p.files.begin(),
                    p.files.end(),
                    p.activeFiles[i]);
                if (j != p.files.end())
                {
                    const auto k = j - p.files.begin();
                    out.push_back(p.players[k]);
                }
            }
            return out;
        }

        otime::RationalTime App::_getCacheReadAhead() const
        {
            TLRENDER_P();
            const size_t activeCount = p.activeFiles.size();
            const double readAhead = p.settings->getValue<double>("Cache/ReadAhead");
            return otime::RationalTime(
                activeCount > 0 ? (readAhead / static_cast<double>(activeCount)) : 0.0,
                1.0);
        }

        otime::RationalTime App::_getCacheReadBehind() const
        {
            TLRENDER_P();
            const size_t activeCount = p.activeFiles.size();
            const double readBehind = p.settings->getValue<double>("Cache/ReadBehind");
            return otime::RationalTime(
                activeCount > 0 ? (readBehind / static_cast<double>(activeCount)) : 0.0,
                1.0);
        }

        void App::_filesCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            // Create the new list of players.
            std::vector<std::shared_ptr<timeline::Player> > players(items.size());
            for (size_t i = 0; i < items.size(); ++i)
            {
                const auto j = std::find(p.files.begin(), p.files.end(), items[i]);
                if (j != p.files.end())
                {
                    const size_t k = j - p.files.begin();
                    players[i] = p.players[k];
                }
            }

            // Find players to destroy.
            std::vector<std::shared_ptr<timeline::Player> > destroy;
            for (size_t i = 0; i < p.files.size(); ++i)
            {
                const auto j = std::find(items.begin(), items.end(), p.files[i]);
                if (j == items.end())
                {
                    destroy.push_back(p.players[i]);
                }
            }

            // Create new timeline players.
            auto audioSystem = _context->getSystem<audio::System>();
            for (size_t i = 0; i < players.size(); ++i)
            {
                if (!players[i])
                {
                    try
                    {
                        timeline::Options options;
                        options.fileSequenceAudio =
                            p.settings->getValue<timeline::FileSequenceAudio>("FileSequence/Audio");
                        options.fileSequenceAudioFileName =
                            p.settings->getValue<std::string>("FileSequence/AudioFileName");
                        options.fileSequenceAudioDirectory =
                            p.settings->getValue<std::string>("FileSequence/AudioDirectory");
                        options.videoRequestCount =
                            p.settings->getValue<size_t>("Performance/VideoRequestCount");
                        options.audioRequestCount =
                            p.settings->getValue<size_t>("Performance/AudioRequestCount");
                        options.ioOptions = _getIOOptions();
                        options.pathOptions.maxNumberDigits =
                            p.settings->getValue<size_t>("FileSequence/MaxDigits");
                        auto otioTimeline = items[i]->audioPath.isEmpty() ?
                            timeline::create(items[i]->path, _context, options) :
                            timeline::create(items[i]->path, items[i]->audioPath, _context, options);
                        auto timeline = timeline::Timeline::create(otioTimeline, _context, options);

                        timeline::PlayerOptions playerOptions;
                        playerOptions.cache.readAhead = time::invalidTime;
                        playerOptions.cache.readBehind = time::invalidTime;
                        playerOptions.timerMode =
                            p.settings->getValue<timeline::TimerMode>("Performance/TimerMode");
                        playerOptions.audioBufferFrameCount =
                            p.settings->getValue<size_t>("Performance/AudioBufferFrameCount");
                        players[i] = timeline::Player::create(timeline, _context, playerOptions);

                        for (const auto& video : players[i]->getIOInfo().video)
                        {
                            items[i]->videoLayers.push_back(video.name);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        _log(e.what(), log::Type::Error);
                    }
                }
            }

            p.files = items;
            p.players = players;
        }

        void App::_activeCallback(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            auto activePlayers = _getActivePlayers();
            if (!activePlayers.empty() && activePlayers[0])
            {
                activePlayers[0]->setPlayback(timeline::Playback::Stop);
            }

            p.activeFiles = items;

            activePlayers = _getActivePlayers();
            p.activePlayers->setIfChanged(activePlayers);
            std::shared_ptr<timeline::Player> first;
            if (!activePlayers.empty())
            {
                first = activePlayers[0];
                if (first)
                {
                    first->setExternalTime(nullptr);
                }
            }
            for (size_t i = 1; i < activePlayers.size(); ++i)
            {
                if (auto player = activePlayers[i])
                {
                    activePlayers[i]->setExternalTime(
                        first.get() != player.get() ?
                        first :
                        nullptr);
                }
            }

            _cacheUpdate();
            _audioUpdate();
        }

        void App::_settingsUpdate(const std::string& name)
        {
            TLRENDER_P();
            const auto split = string::split(name, '/');
            if (!split.empty() || name.empty())
            {
                auto ioSystem = _context->getSystem<io::System>();
                const auto& names = ioSystem->getNames();
                bool match = false;
                if (!split.empty())
                {
                    match = std::find(names.begin(), names.end(), split[0]) != names.end();
                }
                if (match || name.empty())
                {
                    const auto ioOptions = _getIOOptions();
                    for (const auto& player : p.players)
                    {
                        if (player)
                        {
                            player->setIOOptions(ioOptions);
                        }
                    }
                }
            }
            if ("Cache/Size" == name ||
                "Cache/ReadAhead" == name ||
                "Cache/ReadBehind" == name ||
                name.empty())
            {
                _cacheUpdate();
            }
            if ("FileBrowser/Path" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                fileBrowserSystem->setPath(
                    p.settings->getValue<std::string>("FileBrowser/Path"));
            }
            if ("FileBrowser/Options" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto options = p.settings->getValue<ui::FileBrowserOptions>("FileBrowser/Options");
                fileBrowserSystem->setOptions(options);
            }
            if ("FileBrowser/NativeFileDialog" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                fileBrowserSystem->setNativeFileDialog(
                    p.settings->getValue<bool>("FileBrowser/NativeFileDialog"));
            }
            if ("Files/RecentMax" == name || name.empty())
            {
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
                recentFilesModel->setRecentMax(
                    p.settings->getValue<int>("Files/RecentMax"));
            }
            if ("Files/Recent" == name || name.empty())
            {
                std::vector<file::Path> recentPaths;
                for (const auto& recentFile :
                    p.settings->getValue<std::vector<std::string> >("Files/Recent"))
                {
                    recentPaths.push_back(file::Path(recentFile));
                }
                auto fileBrowserSystem = _context->getSystem<ui::FileBrowserSystem>();
                auto recentFilesModel = fileBrowserSystem->getRecentFilesModel();
                recentFilesModel->setRecent(recentPaths);
            }
            if ("Style/Palette" == name || name.empty())
            {
                getStyle()->setColorRoles(getStylePalette(
                    p.settings->getValue<StylePalette>("Style/Palette")));
            }
        }

        void App::_cacheUpdate()
        {
            TLRENDER_P();

            // Update the I/O cache.
            auto ioSystem = _context->getSystem<io::System>();
            ioSystem->getCache()->setMax(
                p.settings->getValue<size_t>("Cache/Size") * memory::gigabyte);

            // Update inactive players.
            timeline::PlayerCacheOptions cacheOptions;
            cacheOptions.readAhead = time::invalidTime;
            cacheOptions.readBehind = time::invalidTime;
            const auto activePlayers = _getActivePlayers();
            for (const auto& player : p.players)
            {
                const auto j = std::find(
                    activePlayers.begin(),
                    activePlayers.end(),
                    player);
                if (j == activePlayers.end())
                {
                    if (player)
                    {
                        player->setCacheOptions(cacheOptions);
                    }
                }
            }

            // Update active players.
            cacheOptions.readAhead = _getCacheReadAhead();
            cacheOptions.readBehind = _getCacheReadBehind();
            for (const auto& player : activePlayers)
            {
                if (player)
                {
                    player->setCacheOptions(cacheOptions);
                }
            }
        }

        void App::_audioUpdate()
        {
            TLRENDER_P();
            const float volume = p.audioModel->getVolume();
            const bool mute = p.audioModel->isMuted();
            for (const auto& player : p.players)
            {
                if (player)
                {
                    player->setVolume(volume);
                    player->setMute(mute || p.deviceActive);
                    player->setAudioOffset(p.audioModel->getSyncOffset());
                }
            }
        }
    }
}
