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
                const ftk::Path&,
                const IOOptions& = IOOptions());

            //! Get information.
            TL_API InfoRequest getInfo(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const IOOptions& = IOOptions());

            //! Get a video thumbnail.
            TL_API ThumbnailRequest getThumbnail(
                const ftk::Path&,
                int height,
                const OTIO_NS::RationalTime& = invalidTime,
                const IOOptions& = IOOptions());

            //! Get a video thumbnail.
            TL_API ThumbnailRequest getThumbnail(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                int height,
                const OTIO_NS::RationalTime& = invalidTime,
                const IOOptions& = IOOptions());

            //! Get an audio waveform.
            TL_API WaveformRequest getWaveform(
                const ftk::Path&,
                const ftk::Size2I&,
                const OTIO_NS::TimeRange& = invalidTimeRange,
                const IOOptions& = IOOptions());

            //! Get an audio waveform.
            TL_API WaveformRequest getWaveform(
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
    }
}
