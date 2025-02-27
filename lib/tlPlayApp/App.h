// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/SettingsModel.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/App.h>

#include <filesystem>

namespace dtk
{
    class Settings;
}

namespace tl
{
#if defined(TLRENDER_BMD)
    namespace bmd
    {
        class OutputDevice;
    }
#endif // TLRENDER_BMD

    namespace play
    {
        struct FilesModelItem;

        class AudioModel;
        class ColorModel;
        class FilesModel;
        class RecentFilesModel;
        class RenderModel;
        class SettingsModel;
        class TimeUnitsModel;
        class ViewportModel;

#if defined(TLRENDER_BMD)
        class BMDDevicesModel;
#endif // TLRENDER_BMD
    }

    //! tlplay application
    namespace play_app
    {
        class MainWindow;
        class ToolsModel;

        //! Application.
        class App : public dtk::App
        {
            DTK_NON_COPYABLE(App);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                std::vector<std::string>&);

            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::shared_ptr<dtk::Context>&,
                std::vector<std::string>&);

            //! Open a file.
            void open(
                const file::Path& path,
                const file::Path& audioPath = file::Path());

            //! Open a file dialog.
            void openDialog();

            //! Open a file and separate audio dialog.
            void openSeparateAudioDialog();

            //! Get the settings model.
            const std::shared_ptr<play::SettingsModel>& getSettingsModel() const;

            //! Get the time units model.
            const std::shared_ptr<play::TimeUnitsModel>& getTimeUnitsModel() const;

            //! Get the files model.
            const std::shared_ptr<play::FilesModel>& getFilesModel() const;

            //! Get the recent files model.
            const std::shared_ptr<play::RecentFilesModel>& getRecentFilesModel() const;

            //! Reload the active files.
            void reload();

            //! Observe the timeline player.
            std::shared_ptr<dtk::IObservableValue<std::shared_ptr<timeline::Player> > > observePlayer() const;

            //! Get the color model.
            const std::shared_ptr<play::ColorModel>& getColorModel() const;

            //! Get the viewport model.
            const std::shared_ptr<play::ViewportModel>& getViewportModel() const;

            //! Get the render model.
            const std::shared_ptr<play::RenderModel>& getRenderModel() const;

            //! Get the audio model.
            const std::shared_ptr<play::AudioModel>& getAudioModel() const;

            //! Get the tools model.
            const std::shared_ptr<ToolsModel>& getToolsModel() const;

            //! Get the main window.
            const std::shared_ptr<MainWindow>& getMainWindow() const;

            //! Observe whether the secondary window is active.
            std::shared_ptr<dtk::IObservableValue<bool> > observeSecondaryWindow() const;

            //! Set whether the secondary window is active.
            void setSecondaryWindow(bool);

#if defined(TLRENDER_BMD)
            //! Get the BMD devices model.
            const std::shared_ptr<play::BMDDevicesModel>& getBMDDevicesModel() const;

            //! Get the BMD output device.
            const std::shared_ptr<bmd::OutputDevice>& getBMDOutputDevice() const;
#endif // TLRENDER_BMD

            void run() override;

        protected:
            void _tick() override;

        private:
            void _modelsInit();
            void _devicesInit();
            void _observersInit();
            void _inputFilesInit();
            void _windowsInit();

            std::filesystem::path _appDocsPath();
            std::filesystem::path _getLogFilePath(
                const std::string& appName,
                const std::filesystem::path& appDocsPath);
            std::filesystem::path _getSettingsPath(
                const std::string& appName,
                const std::filesystem::path& appDocsPath);
            std::vector<std::shared_ptr<dtk::ICmdLineArg> > _getCmdLineArgs();
            std::vector<std::shared_ptr<dtk::ICmdLineOption> > _getCmdLineOptions();
            io::Options _getIOOptions() const;

            void _filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _activeUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _layersUpdate(const std::vector<int>&);
            void _cacheUpdate(const play::CacheOptions&);
            void _viewUpdate(const dtk::V2I& pos, double zoom, bool frame);
            void _audioUpdate();

            DTK_PRIVATE();
        };
    }
}
