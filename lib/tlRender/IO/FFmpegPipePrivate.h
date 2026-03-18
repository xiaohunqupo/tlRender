// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/FFmpegPipe.h>

namespace tl
{
    namespace ffmpeg_pipe
    {
        class POpen
        {
        public:
            POpen(const std::string& cmd, const std::string& mode);

            ~POpen();

            std::string readAll();

            FILE* f();

        private:
            FTK_PRIVATE();
        };

        typedef std::pair<int, int> Rational;

        Rational toRational(const std::string&);
        double toDouble(const Rational&);

        ftk::ImageType toImageType(const std::string&);
        std::string fromImageType(ftk::ImageType);

        AudioType toAudioType(const std::string&);
        std::string fromAudioType(AudioType);
    }
}
