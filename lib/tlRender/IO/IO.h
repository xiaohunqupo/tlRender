// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Audio.h>
#include <tlRender/Core/Time.h>

#include <ftk/Core/Image.h>

namespace tl
{
    //! Audio and video I/O.
    namespace io
    {
        //! File types.
        enum class TL_API_TYPE FileType
        {
            Unknown = 0,
            Media   = 1,
            Seq     = 2,

            Count,
            First = Unknown
        };

        //! I/O information.
        struct TL_API_TYPE Info
        {
            //! Video layer information.
            std::vector<ftk::ImageInfo> video;

            //! Video time range.
            OTIO_NS::TimeRange videoTime = invalidTimeRange;

            //! Audio information.
            AudioInfo audio;

            //! Audio time range.
            OTIO_NS::TimeRange audioTime = invalidTimeRange;

            //! Metadata tags.
            ftk::ImageTags tags;

            bool operator == (const Info&) const;
            bool operator != (const Info&) const;
        };

        //! Video I/O data.
        struct TL_API_TYPE VideoData
        {
            VideoData();
            VideoData(
                const OTIO_NS::RationalTime&,
                uint16_t layer,
                const std::shared_ptr<ftk::Image>&);

            OTIO_NS::RationalTime       time = invalidTime;
            uint16_t                    layer = 0;
            std::shared_ptr<ftk::Image> image;

            bool operator == (const VideoData&) const;
            bool operator != (const VideoData&) const;
            bool operator < (const VideoData&) const;
        };

        //! Audio I/O data.
        struct AudioData
        {
            AudioData();
            AudioData(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<Audio>&);

            OTIO_NS::RationalTime  time = invalidTime;
            std::shared_ptr<Audio> audio;

            bool operator == (const AudioData&) const;
            bool operator != (const AudioData&) const;
            bool operator < (const AudioData&) const;
        };

        //! Get an integer image type for the given channel count and bit depth.
        TL_API ftk::ImageType getIntType(size_t channelCount, size_t bitDepth);

        //! Get a floating point image type for the given channel count and bit
        //! depth.
        TL_API ftk::ImageType getFloatType(size_t channelCount, size_t bitDepth);

        //! Options.
        typedef std::map<std::string, std::string> Options;

        //! Merge options.
        TL_API Options merge(const Options&, const Options&);
    }
}

#include <tlRender/IO/IOInline.h>
