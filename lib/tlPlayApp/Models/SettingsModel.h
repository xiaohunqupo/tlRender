// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <tlTimeline/Player.h>

#include <dtk/ui/App.h>
#include <dtk/ui/FileBrowser.h>
#include <dtk/core/ObservableValue.h>

#include <tlIO/SequenceIO.h>
#if defined(TLRENDER_FFMPEG)
#include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_USD)
#include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <nlohmann/json.hpp>

namespace dtk
{
    class Context;
    class Settings;
}

namespace tl
{
    namespace play
    {
        //! Cache settings.
        struct CacheSettings
        {
            size_t sizeGB = 4;
            double readAhead = 4.F;
            double readBehind = .5F;

            bool operator == (const CacheSettings&) const;
            bool operator != (const CacheSettings&) const;
        };

        //! Export render size.
        enum class ExportRenderSize
        {
            Default,
            _1920_1080,
            _3840_2160,
            _4096_2160,
            Custom,

            Count,
            First = Default
        };
        DTK_ENUM(ExportRenderSize);

        //! Get an export render size.
        const dtk::Size2I& getSize(ExportRenderSize);

        //! Export file type.
        enum class ExportFileType
        {
            Images,
            Movie,

            Count,
            First = Images
        };
        DTK_ENUM(ExportFileType);

        //! Export settings.
        struct ExportSettings
        {
            std::string directory;
            ExportRenderSize renderSize = ExportRenderSize::Default;
            dtk::Size2I customRenderSize = dtk::Size2I(1920, 1080);
            ExportFileType fileType = ExportFileType::Images;
            std::string imageBaseName;
            size_t imagePad = 0;
            std::string imageExtension;
            std::string movieBaseName;
            std::string movieExtension;
            std::string movieCodec;

            bool operator == (const ExportSettings&) const;
            bool operator != (const ExportSettings&) const;
        };

        //! File browser settings.
        struct FileBrowserSettings
        {
            bool nativeFileDialog = true;
            std::string path;
            dtk::FileBrowserOptions options;

            bool operator == (const FileBrowserSettings&) const;
            bool operator != (const FileBrowserSettings&) const;
        };

        //! File sequence settings.
        struct FileSequenceSettings
        {
            timeline::FileSequenceAudio audio = timeline::FileSequenceAudio::BaseName;
            std::string audioFileName;
            std::string audioDirectory;
            size_t maxDigits = 9;
            io::SequenceOptions io;

            bool operator == (const FileSequenceSettings&) const;
            bool operator != (const FileSequenceSettings&) const;
        };

        //! Miscellaneous settings.
        struct MiscSettings
        {
            bool tooltipsEnabled = true;

            bool operator == (const MiscSettings&) const;
            bool operator != (const MiscSettings&) const;
        };

        //! Mouse actions.
        enum class MouseAction
        {
            PanView,
            CompareWipe,
            ColorPicker,
            FrameShuttle,

            Count,
            First = ColorPicker
        };
        DTK_ENUM(MouseAction);

        //! Mouse settings.
        struct MouseSettings
        {
            std::map<MouseAction, dtk::KeyModifier> actions =
            {
                { MouseAction::ColorPicker, dtk::KeyModifier::None },
                { MouseAction::PanView, dtk::KeyModifier::Control },
                { MouseAction::FrameShuttle, dtk::KeyModifier::Shift },
                { MouseAction::CompareWipe, dtk::KeyModifier::Alt }
            };

            bool operator == (const MouseSettings&) const;
            bool operator != (const MouseSettings&) const;
        };

        //! Performance settings.
        struct PerformanceSettings
        {
            size_t audioBufferFrameCount = timeline::PlayerOptions().audioBufferFrameCount;
            size_t videoRequestCount = 16;
            size_t audioRequestCount = 16;

            bool operator == (const PerformanceSettings&) const;
            bool operator != (const PerformanceSettings&) const;
        };

        //! Style settings.
        struct StyleSettings
        {
            dtk::ColorStyle colorStyle = dtk::ColorStyle::Dark;
            float displayScale = 0.F;

            bool operator == (const StyleSettings&) const;
            bool operator != (const StyleSettings&) const;
        };

        //! Timeline settings.
        struct TimelineSettings
        {
            bool editable = false;
            bool frameView = true;
            bool scroll = true;
            bool stopOnScrub = false;
            timelineui::ItemOptions item;
            timelineui::DisplayOptions display;
            bool firstTrack = false;

            bool operator == (const TimelineSettings&) const;
            bool operator != (const TimelineSettings&) const;
        };

        //! Window settings.
        struct WindowSettings
        {
            dtk::Size2I size = dtk::Size2I(1920, 1080);
            bool fileToolBar = true;
            bool compareToolBar = true;
            bool windowToolBar = true;
            bool viewToolBar = true;
            bool toolsToolBar = true;
            bool timeline = true;
            bool bottomToolBar = true;
            bool statusToolBar = true;
            float splitter = .7F;
            float splitter2 = .7F;

            bool operator == (const WindowSettings&) const;
            bool operator != (const WindowSettings&) const;
        };

        //! Settings model.
        class SettingsModel : public std::enable_shared_from_this<SettingsModel>
        {
            DTK_NON_COPYABLE(SettingsModel);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            SettingsModel();

        public:
            ~SettingsModel();

            //! Create a new model.
            static std::shared_ptr<SettingsModel> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Settings>&);

            //! Reset to default values.
            void reset();

            //! \name Cache
            ///@{

            const CacheSettings& getCache() const;
            std::shared_ptr<dtk::IObservableValue<CacheSettings> > observeCache() const;
            void setCache(const CacheSettings&);

            ///@}

            //! \name Export
            ///@{

            const ExportSettings& getExport() const;
            std::shared_ptr<dtk::IObservableValue<ExportSettings> > observeExport() const;
            void setExport(const ExportSettings&);

            ///@}

            //! \name File Browser
            ///@{

            const FileBrowserSettings& getFileBrowser() const;
            std::shared_ptr<dtk::IObservableValue<FileBrowserSettings> > observeFileBrowser() const;
            void setFileBrowser(const FileBrowserSettings&);

            ///@}

            //! \name File Sequences
            ///@{

            const FileSequenceSettings& getFileSequence() const;
            std::shared_ptr<dtk::IObservableValue<FileSequenceSettings> > observeFileSequence() const;
            void setFileSequence(const FileSequenceSettings&);

            ///@}

            //! \name Miscellaneous
            ///@{

            const MiscSettings& getMisc() const;
            std::shared_ptr<dtk::IObservableValue<MiscSettings> > observeMisc() const;
            void setMisc(const MiscSettings&);

            ///@}

            //! \name Mouse
            ///@{

            const MouseSettings& getMouse() const;
            std::shared_ptr<dtk::IObservableValue<MouseSettings> > observeMouse() const;
            void setMouse(const MouseSettings&);

            ///@}

            //! \name Performance
            ///@{

            const PerformanceSettings& getPerformance() const;
            std::shared_ptr<dtk::IObservableValue<PerformanceSettings> > observePerformance() const;
            void setPerformance(const PerformanceSettings&);

            ///@}

            //! \name Style
            ///@{

            const StyleSettings& getStyle() const;
            std::shared_ptr<dtk::IObservableValue<StyleSettings> > observeStyle() const;
            void setStyle(const StyleSettings&);

            ///@}

            //! \name Timeline
            ///@{

            const TimelineSettings& getTimeline() const;
            std::shared_ptr<dtk::IObservableValue<TimelineSettings> > observeTimeline() const;
            void setTimeline(const TimelineSettings&);

            ///@}

            //! \name Window
            ///@{

            const WindowSettings& getWindow() const;
            std::shared_ptr<dtk::IObservableValue<WindowSettings> > observeWindow() const;
            void setWindow(const WindowSettings&);

            ///@}

#if defined(TLRENDER_FFMPEG)
            //! \name FFmpeg
            ///@{

            const ffmpeg::Options& getFFmpeg() const;

            std::shared_ptr<dtk::IObservableValue<ffmpeg::Options> > observeFFmpeg() const;

            void setFFmpeg(const ffmpeg::Options&);

            ///@}
#endif // TLRENDER_FFMPEG

#if defined(TLRENDER_USD)
            //! \name USD
            ///@{

            const usd::Options& getUSD() const;

            std::shared_ptr<dtk::IObservableValue<usd::Options> > observeUSD() const;

            void setUSD(const usd::Options&);

            ///@}
#endif // TLRENDER_USD

        private:
            DTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const CacheSettings&);
        void to_json(nlohmann::json&, const ExportSettings&);
        void to_json(nlohmann::json&, const FileBrowserSettings&);
        void to_json(nlohmann::json&, const FileSequenceSettings&);
        void to_json(nlohmann::json&, const MiscSettings&);
        void to_json(nlohmann::json&, const MouseSettings&);
        void to_json(nlohmann::json&, const PerformanceSettings&);
        void to_json(nlohmann::json&, const StyleSettings&);
        void to_json(nlohmann::json&, const TimelineSettings&);
        void to_json(nlohmann::json&, const WindowSettings&);

        void from_json(const nlohmann::json&, CacheSettings&);
        void from_json(const nlohmann::json&, ExportSettings&);
        void from_json(const nlohmann::json&, FileBrowserSettings&);
        void from_json(const nlohmann::json&, FileSequenceSettings&);
        void from_json(const nlohmann::json&, MiscSettings&);
        void from_json(const nlohmann::json&, MouseSettings&);
        void from_json(const nlohmann::json&, PerformanceSettings&);
        void from_json(const nlohmann::json&, StyleSettings&);
        void from_json(const nlohmann::json&, TimelineSettings&);
        void from_json(const nlohmann::json&, WindowSettings&);

        ///@}
    }
}
