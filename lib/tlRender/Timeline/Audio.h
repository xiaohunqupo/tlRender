// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/IO.h>

namespace tl
{
    namespace timeline
    {
        //! Audio layer.
        struct TL_API_TYPE AudioLayer
        {
            std::shared_ptr<audio::Audio> audio;

            TL_API bool operator == (const AudioLayer&) const;
            TL_API bool operator != (const AudioLayer&) const;
        };

        //! Audio data.
        struct TL_API_TYPE AudioData
        {
            double                  seconds = -1.0;
            std::vector<AudioLayer> layers;

            TL_API bool operator == (const AudioData&) const;
            TL_API bool operator != (const AudioData&) const;
        };

        //! Compare the time values of audio data.
        TL_API bool isTimeEqual(const AudioData&, const AudioData&);
    }
}
