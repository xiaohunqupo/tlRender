// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Audio.h>
#include <tlRender/Timeline/TimelineOptions.h>
#include <tlRender/Timeline/Video.h>

#include <tlRender/IO/Read.h>

#include <ftk/Core/FileIO.h>
#include <ftk/Core/Path.h>

#include <opentimelineio/timeline.h>
#include <opentimelineio/mediaReference.h>

#include <future>

namespace ftk
{
    class Context;
}

namespace tl
{
    //! Video request.
    struct TL_API_TYPE VideoRequest
    {
        uint64_t id = 0;
        std::future<VideoFrame> future;
    };

    //! Audio request.
    struct TL_API_TYPE AudioRequest
    {
        uint64_t id = 0;
        std::future<AudioFrame> future;
    };

    //! Timeline.
    class TL_API_TYPE Timeline : public std::enable_shared_from_this<Timeline>
    {
        FTK_NON_COPYABLE(Timeline);

    protected:
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const ftk::Path& inputPath,
            const ftk::Path& inputAudioPath,
            const Options&);
        void _init(
            const std::shared_ptr<ftk::Context>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            const Options&);

        Timeline();

    public:
        TL_API ~Timeline();

        //! Create a new timeline.
        TL_API static std::shared_ptr<Timeline> create(
            const std::shared_ptr<ftk::Context>&,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&,
            const Options& = Options());

        //! Create a new timeline from a path. The path can point to an
        //! .otio file, movie file, or image sequence.
        TL_API static std::shared_ptr<Timeline> create(
            const std::shared_ptr<ftk::Context>&,
            const ftk::Path&,
            const Options& = Options());

        //! Create a new timeline from a path and audio path. The path can
        //! point to an .otio file, movie file, or image sequence.
        TL_API static std::shared_ptr<Timeline> create(
            const std::shared_ptr<ftk::Context>&,
            const ftk::Path& path,
            const ftk::Path& audioPath,
            const Options& = Options());

        //! Create a new timeline from a file name. The file name can point
        //! to an .otio file, movie file, or image sequence.
        TL_API static std::shared_ptr<Timeline> create(
            const std::shared_ptr<ftk::Context>&,
            const std::string&,
            const Options& = Options());

        //! Create a new timeline from a file name and audio file name.
        //! The file name can point to an .otio file, movie file, or
        //! image sequence.
        TL_API static std::shared_ptr<Timeline> create(
            const std::shared_ptr<ftk::Context>&,
            const std::string& fileName,
            const std::string& audioFilename,
            const Options& = Options());

        //! Get the context.
        TL_API std::shared_ptr<ftk::Context> getContext() const;

        //! Get the timeline.
        TL_API const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& getTimeline() const;

        //! Get the file path.
        TL_API const ftk::Path& getPath() const;

        //! Get the audio file path.
        TL_API const ftk::Path& getAudioPath() const;

        //! Get the timeline options.
        TL_API const Options& getOptions() const;

        //! Get the memory for the given media reference.
        TL_API std::vector<ftk::MemFile> getMem(
            const OTIO_NS::MediaReference*);

        //! \name Information
        ///@{

        //! Get the time range.
        TL_API const OTIO_NS::TimeRange& getTimeRange() const;

        //! Get the duration.
        TL_API OTIO_NS::RationalTime getDuration() const;

        //! Get the I/O information. This information is retrieved from
        //! the first clip in the timeline.
        TL_API const IOInfo& getIOInfo() const;

        //! Get the first error encountered while reading, or an empty
        //! string. Errors are also sent to the log.
        TL_API std::string getReadError() const;

        //! Get the number of errors encountered while reading. The count
        //! is a lower bound; errors from readers that have been evicted
        //! from the internal cache may not be included.
        TL_API size_t getReadErrorCount() const;

        ///@}

        //! \name Video and Audio
        ///@{

        //! Get video.
        TL_API VideoRequest getVideo(
            const OTIO_NS::RationalTime&,
            const IOOptions& = IOOptions());

        //! Get audio.
        TL_API AudioRequest getAudio(
            double seconds,
            const IOOptions& = IOOptions());

        //! Cancel requests.
        TL_API void cancelRequests(const std::vector<uint64_t>&);

        ///@}

        //! Get the number of objects currenty instantiated.
        TL_API static size_t getObjectCount();

    private:
        std::shared_ptr<IRead> _getRead(
            const OTIO_NS::Clip*,
            const IOOptions&);
        std::future<VideoData> _readVideo(
            const OTIO_NS::Clip*,
            const OTIO_NS::RationalTime&,
            const IOOptions&);
        std::future<AudioData> _readAudio(
            const OTIO_NS::Clip*,
            const OTIO_NS::TimeRange&,
            const IOOptions&);

        bool _getVideoInfo(const OTIO_NS::Composable*);
        bool _getAudioInfo(const OTIO_NS::Composable*);
        void _getCanvas();

        float _transitionValue(double frame, double in, double out) const;

        void _tick();
        void _requests();
        void _finishRequests();

        FTK_PRIVATE();
    };
}
