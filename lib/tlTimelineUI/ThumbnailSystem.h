// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/IO.h>

#include <tlCore/ISystem.h>
#include <tlCore/Path.h>

#include <feather-tk/core/FileIO.h>
#include <feather-tk/core/Image.h>
#include <feather-tk/core/Mesh.h>

#include <future>

namespace feather_tk
{
    namespace gl
    {
        class Window;
    }
}

namespace tl
{
    namespace timelineui
    {
        //! Information request.
        struct InfoRequest
        {
            uint64_t id = 0;
            std::future<io::Info> future;
        };

        //! Video thumbnail request.
        struct ThumbnailRequest
        {
            uint64_t id = 0;
            int height = 0;
            OTIO_NS::RationalTime time = time::invalidTime;
            std::future<std::shared_ptr<feather_tk::Image> > future;
        };

        //! Audio waveform request.
        struct WaveformRequest
        {
            uint64_t id = 0;
            feather_tk::Size2I size;
            OTIO_NS::TimeRange timeRange = time::invalidTimeRange;
            std::future<std::shared_ptr<feather_tk::TriMesh2F> > future;
        };

        //! Thumbnail cache.
        class ThumbnailCache : public std::enable_shared_from_this<ThumbnailCache>
        {
        protected:
            void _init(const std::shared_ptr<feather_tk::Context>&);

            ThumbnailCache();

        public:
            ~ThumbnailCache();

            //! Create a new thumbnail cache.
            static std::shared_ptr<ThumbnailCache> create(
                const std::shared_ptr<feather_tk::Context>&);

            //! Get the maximum cache size.
            size_t getMax() const;

            //! Set the maximum cache size.
            void setMax(size_t);

            //! Get the current cache size.
            size_t getSize() const;

            //! Get the current cache size as a percentage.
            float getPercentage() const;
            
            //! Get an I/O information cache key.
            static std::string getInfoKey(
                intptr_t id,
                const file::Path&,
                const io::Options&);

            //! Add I/O information to the cache.
            void addInfo(const std::string& key, const io::Info&);

            //! Get whether the cache contains I/O information.
            bool containsInfo(const std::string& key);

            //! Get I/O information from the cache.
            bool getInfo(const std::string& key, io::Info&) const;
            
            //! Get a thumbnail cache key.
            static std::string getThumbnailKey(
                intptr_t id,
                const file::Path&,
                int height,
                const OTIO_NS::RationalTime&,
                const io::Options&);

            //! Add a thumbnail to the cache.
            void addThumbnail(
                const std::string& key,
                const std::shared_ptr<feather_tk::Image>&);

            //! Get whether the cache contains a thumbnail.
            bool containsThumbnail(const std::string& key);

            //! Get a thumbnail from the cache.
            bool getThumbnail(
                const std::string& key,
                std::shared_ptr<feather_tk::Image>&) const;

            //! Get a waveform cache key.
            static std::string getWaveformKey(
                intptr_t id,
                const file::Path&,
                const feather_tk::Size2I&,
                const OTIO_NS::TimeRange&,
                const io::Options&);

            //! Add a waveform to the cache.
            void addWaveform(
                const std::string& key,
                const std::shared_ptr<feather_tk::TriMesh2F>&);

            //! Get whether the cache contains a waveform.
            bool containsWaveform(const std::string& key);

            //! Get a waveform from the cache.
            bool getWaveform(
                const std::string& key,
                std::shared_ptr<feather_tk::TriMesh2F>&) const;

            //! Clear the cache.
            void clear();

        private:
            void _maxUpdate();

            FEATHER_TK_PRIVATE();
        };

        //! Thumbnail generator.
        class ThumbnailGenerator : public std::enable_shared_from_this<ThumbnailGenerator>
        {
        protected:
            void _init(
                const std::shared_ptr<ThumbnailCache>&,
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::gl::Window>&);

            ThumbnailGenerator();

        public:
            ~ThumbnailGenerator();

            //! Create a new thumbnail generator.
            static std::shared_ptr<ThumbnailGenerator> create(
                const std::shared_ptr<ThumbnailCache>&,
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::gl::Window>& = nullptr);

            //! Get information.
            InfoRequest getInfo(
                intptr_t id,
                const file::Path&,
                const io::Options& = io::Options());

            //! Get information.
            InfoRequest getInfo(
                intptr_t id,
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                intptr_t id,
                const file::Path&,
                int height,
                const OTIO_NS::RationalTime& = time::invalidTime,
                const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                intptr_t id,
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                int height,
                const OTIO_NS::RationalTime& = time::invalidTime,
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                intptr_t id,
                const file::Path&,
                const feather_tk::Size2I&,
                const OTIO_NS::TimeRange& = time::invalidTimeRange,
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                intptr_t id,
                const file::Path&,
                const std::vector<feather_tk::InMemoryFile>&,
                const feather_tk::Size2I&,
                const OTIO_NS::TimeRange& = time::invalidTimeRange,
                const io::Options& = io::Options());

            //! Cancel pending requests.
            void cancelRequests(const std::vector<uint64_t>&);

        private:
            void _infoRun();
            void _thumbnailRun();
            void _waveformRun();
            void _infoCancel();
            void _thumbnailCancel();
            void _waveformCancel();

            FEATHER_TK_PRIVATE();
        };

        //! Thumbnail system.
        class ThumbnailSystem : public system::ISystem
        {
        protected:
            ThumbnailSystem(const std::shared_ptr<feather_tk::Context>&);

        public:
            ~ThumbnailSystem();

            //! Create a new system.
            static std::shared_ptr<ThumbnailSystem> create(
                const std::shared_ptr<feather_tk::Context>&);

            //! Get information.
            InfoRequest getInfo(
                intptr_t id,
                const file::Path&,
                const io::Options& = io::Options());

            //! Get a video thumbnail.
            ThumbnailRequest getThumbnail(
                intptr_t id,
                const file::Path&,
                int height,
                const OTIO_NS::RationalTime& = time::invalidTime,
                const io::Options& = io::Options());

            //! Get an audio waveform.
            WaveformRequest getWaveform(
                intptr_t id,
                const file::Path&,
                const feather_tk::Size2I&,
                const OTIO_NS::TimeRange& = time::invalidTimeRange,
                const io::Options& = io::Options());

            //! Cancel pending requests.
            void cancelRequests(const std::vector<uint64_t>&);

            //! Get the thumbnail cache.
            const std::shared_ptr<ThumbnailCache>& getCache() const;

        private:
            FEATHER_TK_PRIVATE();
        };
    }
}
