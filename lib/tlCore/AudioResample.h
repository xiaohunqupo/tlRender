// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>

namespace tl
{
    namespace audio
    {
        //! Resample audio data.
        class AudioResample
        {
            FEATHER_TK_NON_COPYABLE(AudioResample);

        protected:
            void _init(
                const audio::Info& input,
                const audio::Info& output);

            AudioResample();

        public:
            ~AudioResample();

            //! Create a new resampler.
            static std::shared_ptr<AudioResample> create(
                const audio::Info& input,
                const audio::Info& ouput);

            //! Get the input audio information.
            const audio::Info& getInputInfo() const;

            //! Get the output audio information.
            const audio::Info& getOutputInfo() const;

            //! Resample audio data.
            std::shared_ptr<Audio> process(const std::shared_ptr<Audio>&);

            //! Flush any remaining data.
            void flush();

        private:
            FEATHER_TK_PRIVATE();
        };
    }
}
