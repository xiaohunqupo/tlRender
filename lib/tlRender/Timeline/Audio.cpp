// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Timeline/Audio.h>

namespace tl
{
    bool AudioLayer::operator == (const AudioLayer& other) const
    {
        return audio == other.audio;
    }

    bool AudioLayer::operator != (const AudioLayer& other) const
    {
        return !(*this == other);
    }

    bool AudioFrame::operator == (const AudioFrame& other) const
    {
        return
            seconds == other.seconds &&
            layers == other.layers;
    }

    bool AudioFrame::operator != (const AudioFrame& other) const
    {
        return !(*this == other);
    }

    bool isTimeEqual(const AudioFrame& a, const AudioFrame& b)
    {
        return a.seconds == b.seconds;
    }
}
