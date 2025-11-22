// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlIO/IO.h>

#include <ftk/Core/Path.h>

namespace tl
{
    //! Timelines.
    namespace timeline
    {
        //! Image sequence audio options.
        enum class ImageSeqAudio
        {
            None,     //!< No audio
            Ext,      //!< Search for an audio file by extension
            FileName, //!< Use the given audio file name

            Count,
            First = None
        };
        FTK_ENUM(ImageSeqAudio);

        //! Timeline options.
        struct Options
        {
            //! Image sequence audio.
            ImageSeqAudio imageSeqAudio = ImageSeqAudio::Ext;

            //! Image sequence audio extensions.
            std::vector<std::string> imageSeqAudioExts = { ".mp3", ".wav" };

            //! Image sequence audio file name.
            std::string imageSeqAudioFileName;

            //! Enable workarounds for timelines that may not conform exactly
            //! to specification.
            bool compat = true;

            //! Maximum number of video requests.
            size_t videoRequestMax = 16;

            //! Maximum number of audio requests.
            size_t audioRequestMax = 16;

            //! Request timeout.
            std::chrono::milliseconds requestTimeout = std::chrono::milliseconds(5);

            //! I/O options.
            io::Options ioOptions;

            //! Path options.
            ftk::PathOptions pathOptions;

            bool operator == (const Options&) const;
            bool operator != (const Options&) const;
        };
    }
}
