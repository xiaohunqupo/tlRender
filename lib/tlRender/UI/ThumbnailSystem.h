// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/IO.h>

#include <ftk/Core/FileIO.h>
#include <ftk/Core/ISystem.h>
#include <ftk/Core/Image.h>
#include <ftk/Core/Mesh.h>
#include <ftk/Core/Path.h>

#include <future>

namespace ftk
{
    namespace gl
    {
        class Window;
    }
}

namespace tl
{
    namespace ui
    {
        //! Information request.
        struct TL_API_TYPE InfoRequest
        {
            uint64_t id = 0;
            std::future<IOInfo> future;
        };

        //! Video thumbnail request.
        struct TL_API_TYPE ThumbnailRequest
        {
            uint64_t id = 0;
            int height = 0;
            OTIO_NS::RationalTime time = invalidTime;
            std::future<std::shared_ptr<ftk::Image> > future;
        };

        //! Audio waveform request.
        struct TL_API_TYPE WaveformRequest
        {
            uint64_t id = 0;
            ftk::Size2I size;
            OTIO_NS::TimeRange timeRange = invalidTimeRange;
            std::future<std::shared_ptr<ftk::TriMesh2F> > future;
        };

        //! Thumbnail cache.
        class TL_API_TYPE ThumbnailCache : public std::enable_shared_from_this<ThumbnailCache>
        {
        protected:
            void _init(const std::shared_ptr<ftk::Context>&);

            ThumbnailCache();

        public:
            TL_API ~ThumbnailCache();

            //! Create a new thumbnail cache.
            TL_API static std::shared_ptr<ThumbnailCache> create(
                const std::shared_ptr<ftk::Context>&);

            //! Get the maximum cache size.
            TL_API size_t getMax() const;

            //! Set the maximum cache size.
            TL_API void setMax(size_t);

            //! Get the current cache size.
            TL_API size_t getSize() const;

            //! Get the current cache size as a percentage.
            TL_API float getPercentage() const;
            
            //! Get an I/O information cache key.
            TL_API static std::string getInfoKey(
                intptr_t id,
                const ftk::Path&,
                const IOOptions&);

            //! Add I/O information to the cache.
            TL_API void addInfo(const std::string& key, const IOInfo&);

            //! Get whether the cache contains I/O information.
            TL_API bool containsInfo(const std::string& key);

            //! Get I/O information from the cache.
            TL_API bool getInfo(const std::string& key, IOInfo&) const;
            
            //! Get a thumbnail cache key.
            TL_API static std::string getThumbnailKey(
                intptr_t id,
                const ftk::Path&,
                int height,
                const OTIO_NS::RationalTime&,
                const IOOptions&);

            //! Add a thumbnail to the cache.
            TL_API void addThumbnail(
                const std::string& key,
                const std::shared_ptr<ftk::Image>&);

            //! Get whether the cache contains a thumbnail.
            TL_API bool containsThumbnail(const std::string& key);

            //! Get a thumbnail from the cache.
            TL_API bool getThumbnail(
                const std::string& key,
                std::shared_ptr<ftk::Image>&) const;

            //! Get a waveform cache key.
            TL_API static std::string getWaveformKey(
                intptr_t id,
                const ftk::Path&,
                const ftk::Size2I&,
                const OTIO_NS::TimeRange&,
                const IOOptions&);

            //! Add a waveform to the cache.
            TL_API void addWaveform(
                const std::string& key,
                const std::shared_ptr<ftk::TriMesh2F>&);

            //! Get whether the cache contains a waveform.
            TL_API bool containsWaveform(const std::string& key);

            //! Get a waveform from the cache.
            TL_API bool getWaveform(
                const std::string& key,
                std::shared_ptr<ftk::TriMesh2F>&) const;

            //! Clear the cache.
            TL_API void clear();

        private:
            void _maxUpdate();

            FTK_PRIVATE();
        };

        //! Thumbnail generator.
        class TL_API_TYPE ThumbnailGenerator : public std::enable_shared_from_this<ThumbnailGenerator>
        {
        protected:
            void _init(
                const std::shared_ptr<ThumbnailCache>&,
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<ftk::gl::Window>&);

            ThumbnailGenerator();

        public:
            TL_API ~ThumbnailGenerator();

            //! Create a new thumbnail generator.
            TL_API static std::shared_ptr<ThumbnailGenerator> create(
                const std::shared_ptr<ThumbnailCache>&,
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<ftk::gl::Window>& = nullptr);

            //! Get information.
            TL_API InfoRequest getInfo(
                intptr_t id,
                const ftk::Path&,
                const IOOptions& = IOOptions());

            //! Get information.
            TL_API InfoRequest getInfo(
                intptr_t id,
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const IOOptions& = IOOptions());

            //! Get a video thumbnail.
            TL_API ThumbnailRequest getThumbnail(
                intptr_t id,
                const ftk::Path&,
                int height,
                const OTIO_NS::RationalTime& = invalidTime,
                const IOOptions& = IOOptions());

            //! Get a video thumbnail.
            TL_API ThumbnailRequest getThumbnail(
                intptr_t id,
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                int height,
                const OTIO_NS::RationalTime& = invalidTime,
                const IOOptions& = IOOptions());

            //! Get an audio waveform.
            TL_API WaveformRequest getWaveform(
                intptr_t id,
                const ftk::Path&,
                const ftk::Size2I&,
                const OTIO_NS::TimeRange& = invalidTimeRange,
                const IOOptions& = IOOptions());

            //! Get an audio waveform.
            TL_API WaveformRequest getWaveform(
                intptr_t id,
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const ftk::Size2I&,
                const OTIO_NS::TimeRange& = invalidTimeRange,
                const IOOptions& = IOOptions());

            //! Cancel pending requests.
            TL_API void cancelRequests(const std::vector<uint64_t>&);

        private:
            void _infoRun();
            void _thumbnailRun();
            void _waveformRun();
            void _infoCancel();
            void _thumbnailCancel();
            void _waveformCancel();

            FTK_PRIVATE();
        };

        //! Thumbnail system.
        class TL_API_TYPE ThumbnailSystem : public ftk::ISystem
        {
        protected:
            ThumbnailSystem(const std::shared_ptr<ftk::Context>&);

        public:
            TL_API ~ThumbnailSystem();

            //! Create a new system.
            TL_API static std::shared_ptr<ThumbnailSystem> create(
                const std::shared_ptr<ftk::Context>&);

            //! Get information.
            TL_API InfoRequest getInfo(
                intptr_t id,
                const ftk::Path&,
                const IOOptions& = IOOptions());

            //! Get a video thumbnail.
            TL_API ThumbnailRequest getThumbnail(
                intptr_t id,
                const ftk::Path&,
                int height,
                const OTIO_NS::RationalTime& = invalidTime,
                const IOOptions& = IOOptions());

            //! Get an audio waveform.
            TL_API WaveformRequest getWaveform(
                intptr_t id,
                const ftk::Path&,
                const ftk::Size2I&,
                const OTIO_NS::TimeRange& = invalidTimeRange,
                const IOOptions& = IOOptions());

            //! Cancel pending requests.
            TL_API void cancelRequests(const std::vector<uint64_t>&);

            //! Get the thumbnail cache.
            TL_API const std::shared_ptr<ThumbnailCache>& getCache() const;

        private:
            FTK_PRIVATE();
        };
    }
}
