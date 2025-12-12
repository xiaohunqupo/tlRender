// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Transition.h>

#include <tlRender/Core/Time.h>

#include <ftk/Core/Image.h>
#include <ftk/Core/RenderOptions.h>

namespace tl
{
    namespace timeline
    {
        //! Video layer.
        struct TL_API_TYPE VideoLayer
        {
            std::shared_ptr<ftk::Image> image;
            ftk::ImageOptions           imageOptions;

            std::shared_ptr<ftk::Image> imageB;
            ftk::ImageOptions           imageOptionsB;

            Transition transition = Transition::None;
            float      transitionValue = 0.F;

            TL_API bool operator == (const VideoLayer&) const;
            TL_API bool operator != (const VideoLayer&) const;
        };

        //! Video data.
        struct TL_API_TYPE VideoData
        {
            ftk::Size2I             size;
            OTIO_NS::RationalTime   time   = time::invalidTime;
            std::vector<VideoLayer> layers;

            TL_API bool operator == (const VideoData&) const;
            TL_API bool operator != (const VideoData&) const;
        };

        //! Compare the time values of video data.
        TL_API bool isTimeEqual(const VideoData&, const VideoData&);
    }
}
