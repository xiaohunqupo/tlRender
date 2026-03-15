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
    }
}
