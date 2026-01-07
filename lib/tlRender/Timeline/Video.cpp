// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/Video.h>

namespace tl
{
    namespace timeline
    {
        bool VideoLayer::operator == (const VideoLayer& other) const
        {
            return
                image == other.image &&
                imageOptions == other.imageOptions &&
                imageB == other.imageB &&
                imageOptionsB == other.imageOptionsB &&
                transition == other.transition &&
                transitionValue == other.transitionValue;
        }

        bool VideoLayer::operator != (const VideoLayer& other) const
        {
            return !(*this == other);
        }

        bool VideoFrame::operator == (const VideoFrame& other) const
        {
            return
                size == other.size &&
                time.strictly_equal(other.time) &&
                layers == other.layers;
        }

        bool VideoFrame::operator != (const VideoFrame& other) const
        {
            return !(*this == other);
        }

        bool isTimeEqual(const VideoFrame& a, const VideoFrame& b)
        {
            return a.time.strictly_equal(b.time);
        }
    }
}
