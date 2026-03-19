// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/FFmpegPipe.h>

namespace tl
{
    namespace ffmpeg_pipe
    {
        class PipeRead
        {
        public:
            PipeRead(const std::vector<std::string>& cmd);

            ~PipeRead();

            size_t read(uint8_t*, size_t);

            std::string readAll();

            std::string readError();

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
