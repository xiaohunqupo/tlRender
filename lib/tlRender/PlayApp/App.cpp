// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "App.h"

#include "FilesModel.h"
#include "MainWindow.h"
#include "SettingsModel.h"

#include <tlRender/Timeline/Util.h>

#include <ftk/UI/DialogSystem.h>
#include <ftk/UI/FileBrowser.h>
#include <ftk/Core/Path.h>

namespace tl
{
    namespace play
    {
        void App::_init(
            const std::shared_ptr<ftk::Context>& context,
            std::vector<std::string>& argv)
        {
            // Command line arguments.
            _cmdLine.inputs = ftk::CmdLineListArg<std::string>::create(
                "input",
                "One or more timelines, movies, or image sequences.",
                true);

            // Command line options.
            _cmdLine.debugLoop = ftk::CmdLineOption<int>::create(
                { "-debugLoop" },
                "Load the command line inputs in a loop. This value is the number of seconds for each cycle.",
                "Testing",
                10);

            ftk::App::_init(
                context,
                argv,
                "tlplay",
                "Example player application.",
                { _cmdLine.inputs },
                { _cmdLine.debugLoop });
        }

        App::~App()
        {
            if (_settingsModel)
            {
                if (_recentFilesModel)
                {
                    std::vector<std::string> recentFiles;
                    for (const auto& i : _recentFilesModel->getRecent())
                    {
                        recentFiles.push_back(i.u8string());
                    }
                    _settingsModel->set("/Files/Recent", recentFiles);
                    _settingsModel->set("/Files/RecentMax", _recentFilesModel->getRecentMax());
                }

                _settingsModel->set(
                    "/TimeUnits",
                    to_string(_timeUnitsModel->getTimeUnits()));
            }
        }

        std::shared_ptr<App> App::create(
            const std::shared_ptr<ftk::Context>& context,
            std::vector<std::string>& argv)
        {
            auto out = std::shared_ptr<App>(new App);
            out->_init(context, argv);
            return out;
        }

        const std::shared_ptr<SettingsModel>& App::getSettingsModel() const
        {
            return _settingsModel;
        }

        const std::shared_ptr<ftk::SysLogModel>& App::getSysLogModel() const
        {
            return _sysLogModel;
        }

        const std::shared_ptr<TimeUnitsModel>& App::getTimeUnitsModel() const
        {
            return _timeUnitsModel;
        }

        const std::shared_ptr<ftk::RecentFilesModel>& App::getRecentFilesModel() const
        {
            return _recentFilesModel;
        }

        const std::shared_ptr<FilesModel>& App::getFilesModel() const
        {
            return _filesModel;
        }

        void App::open(const ftk::Path& path)
        {
            try
            {
                _filesModel->open(path);
            }
            catch (const std::exception& e)
            {
                auto dialogSystem = _context->getSystem<ftk::DialogSystem>();
                dialogSystem->message("ERROR", e.what(), _window);
            }
            _recentFilesModel->addRecent(path.get());
        }

        void App::open()
        {
            auto fileBrowserSystem = _context->getSystem<ftk::FileBrowserSystem>();
            fileBrowserSystem->open(
                _window,
                [this](const ftk::Path& value)
                {
                    open(value);
                });
        }

        void App::reload()
        {
            try
            {
                _filesModel->reload();
            }
            catch (const std::exception& e)
            {
                auto dialogSystem = _context->getSystem<ftk::DialogSystem>();
                dialogSystem->message("ERROR", e.what(), _window);
            }
        }

        void App::run()
        {
            // Create the settings model.
            _settingsModel = SettingsModel::create(
                _context,
                ftk::getSettingsPath("tlRender", "tlplay.json"));

            // Create the system log model.
            _sysLogModel = ftk::SysLogModel::create(_context);

            // Create the time units model.
            _timeUnitsModel = TimeUnitsModel::create(_context);
            std::string settings;
            if (_settingsModel->get("/TimeUnits", settings))
            {
                TimeUnits timeUnits = TimeUnits::Timecode;
                from_string(settings, timeUnits);
                _timeUnitsModel->setTimeUnits(timeUnits);
            }

            // Create the recent files model.
            _recentFilesModel = ftk::RecentFilesModel::create(_context);
            std::vector<std::string> recentFiles;
            if (_settingsModel->get("/Files/Recent", recentFiles))
            {
                std::vector<std::filesystem::path> recentPaths;
                for (const auto& i : recentFiles)
                {
                    recentPaths.push_back(std::filesystem::u8path(i));
                }
                _recentFilesModel->setRecent(recentPaths);
            }
            size_t recentFilesMax = 10;
            if (_settingsModel->get("/Files/RecentMax", recentFilesMax))
            {
                _recentFilesModel->setRecentMax(recentFilesMax);
            }

            // Create the files model.
            _filesModel = FilesModel::create(_context, _settingsModel);

            // Initialize the file browser.
            auto fileBrowserSystem = _context->getSystem<ftk::FileBrowserSystem>();
            fileBrowserSystem->getModel()->setExts(getExts(_context));
            ftk::FileBrowserOptions fileBrowserOptions;
            fileBrowserOptions.dirList.seqExts = tl::getExts(_context, static_cast<int>(tl::FileType::Seq));
            fileBrowserSystem->getModel()->setOptions(fileBrowserOptions);
            fileBrowserSystem->setRecentFilesModel(_recentFilesModel);

#if defined(TLRENDER_BMD)
            // Initialize the BMD output device.
            _bmdOutputDevice = bmd::OutputDevice::create(_context);
            bmd::DeviceConfig bmdConfig;
            bmdConfig.deviceIndex = 0;
            bmdConfig.displayModeIndex = 3;
            bmdConfig.pixelType = bmd::PixelType::_8BitBGRA;
            _bmdOutputDevice->setConfig(bmdConfig);
            /*ForegroundOptions fgOptions;
            fgOptions.grid.enabled = true;
            //fgOptions.grid.size = 1;
            fgOptions.grid.labels = GridLabels::Alphanumeric;
            //fgOptions.grid.lineWidth = 10;
            fgOptions.outline.enabled = true;
            _bmdOutputDevice->setForegroundOptions(fgOptions);*/
            //_bmdOutputDevice->setEnabled(true);
#endif // TLRENDER_BMD

            // Create the main window.
            _window = MainWindow::create(
                _context,
                std::dynamic_pointer_cast<App>(shared_from_this()));

            // Create an observer to update the BMD output device.
#if defined(TLRENDER_BMD)
            _playerObserver = ftk::Observer<std::shared_ptr<Player> >::create(
                _filesModel->observePlayer(),
                [this](const std::shared_ptr<Player>& value)
                {
                    _bmdOutputDevice->setPlayer(value);
                });
#endif // TLRENDER_BMD

            for (const auto& input : _cmdLine.inputs->getList())
            {
                ftk::Path path(input);
                if (path.hasSeqWildcard())
                {
                    path = ftk::expandSeq(path);
                }
                open(path);
            }

            if (_cmdLine.debugLoop->found() &&
                !_cmdLine.inputs->getList().empty())
            {
                _debugTimer = ftk::Timer::create(_context);
                _debugTimer->setRepeating(true);
                _debugTimer->start(
                    std::chrono::seconds(_cmdLine.debugLoop->getValue()),
                    [this]
                    {
                        if (!_filesModel->observePlayers()->isEmpty())
                        {
                            _filesModel->closeAll();
                        }
                        else
                        {
                            ftk::Path path(_cmdLine.inputs->getList()[_debugInput]);
                            if (path.hasSeqWildcard())
                            {
                                path = ftk::expandSeq(path);
                            }
                            open(path);
                            if (auto player = _filesModel->observePlayer()->get())
                            {
                                player->forward();
                            }
                            ++_debugInput;
                            if (_debugInput >= _cmdLine.inputs->getList().size())
                            {
                                _debugInput = 0;
                            }
                        }
                    });
            }

            ftk::App::run();
        }
    }
}
