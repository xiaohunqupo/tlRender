// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Device/BMDData.h>

#include <ftk/GL/GL.h>
#include <ftk/GL/Texture.h>
#include <ftk/Core/Image.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include "DeckLinkAPI.h"

#include <string>

namespace tl
{
    namespace bmd
    {
        //! Convert to BMD.
        TL_API BMDPixelFormat toBMD(PixelType);

        //! Convert from BMD.
        TL_API PixelType fromBMD(BMDPixelFormat);

        //! Get a label.
        TL_API std::string getVideoConnectionLabel(BMDVideoConnection);

        //! Get a label.
        TL_API std::string getAudioConnectionLabel(BMDAudioConnection);

        //! Get a label.
        TL_API std::string getDisplayModeLabel(BMDDisplayMode);

        //! Get a label.
        TL_API std::string getPixelFormatLabel(BMDPixelFormat);

        //! Get a label.
        TL_API std::string getOutputFrameCompletionResultLabel(BMDOutputFrameCompletionResult);

        //! Get the output pixel type.
        TL_API PixelType getOutputType(PixelType);

        //! Get the color buffer type.
        TL_API ftk::gl::TextureType getColorBuffer(PixelType);

        //! Get the pack pixels buffer size.
        TL_API size_t getPackPixelsSize(const ftk::Size2I&, PixelType);

        //! Get the pack pixels format.
        TL_API GLenum getPackPixelsFormat(PixelType);

        //! Get the pack pixels type.
        TL_API GLenum getPackPixelsType(PixelType);

        //! Get the pack pixels alignment.
        TL_API GLint getPackPixelsAlign(PixelType);

        //! Get the pack pixels endian byte swap.
        TL_API GLint getPackPixelsSwap(PixelType);

        //! Copy the pack pixels.
        TL_API void copyPackPixels(
            const void*,
            void*,
            const ftk::Size2I&,
            PixelType);
    }
}
