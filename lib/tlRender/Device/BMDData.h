// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Video.h>

#include <tlRender/Core/HDR.h>
#include <tlRender/Core/Time.h>

#include <ftk/Core/Size.h>

namespace tl
{
    namespace bmd
    {
        //! Display mode.
        struct TL_API_TYPE DisplayMode
        {
            std::string           name;
            ftk::Size2I           size;
            OTIO_NS::RationalTime frameRate;

            TL_API bool operator == (const DisplayMode&) const;
        };

        //! Pixel types.
        //!
        //! \bug Disable 10-bit YUV since the BMD conversion function
        //! shows artifacts.
        enum class TL_API_TYPE PixelType
        {
            None,
            _8BitBGRA,
            _8BitYUV,
            _10BitRGB,
            _10BitRGBX,
            _10BitRGBXLE,
            //_10BitYUV,
            _12BitRGB,
            _12BitRGBLE,

            Count,
            First = None
        };
        TL_ENUM(PixelType);

        //! Get the number of bytes used to store a row of pixel data.
        TL_API size_t getRowByteCount(int, PixelType);

        //! Get the number of bytes used to storepixel data.
        TL_API size_t getDataByteCount(const ftk::Size2I&, PixelType);

        //! Device information.
        struct TL_API_TYPE DeviceInfo
        {
            std::string              name;
            std::vector<DisplayMode> displayModes;
            std::vector<PixelType>   pixelTypes;
            size_t                   minVideoPreroll  = 0;
            bool                     hdrMetaData      = false;
            size_t                   maxAudioChannels = 0;

            TL_API bool operator == (const DeviceInfo&) const;
            TL_API bool operator != (const DeviceInfo&) const;
        };

        //! Device options.
        enum class TL_API_TYPE Option
        {
            None,
            _444SDIVideoOutput,

            Count,
            First = None
        };
        TL_ENUM(Option);

        //! Device boolean options.
        typedef std::map<Option, bool> BoolOptions;

        //! Device configuration.
        struct TL_API_TYPE DeviceConfig
        {
            int         deviceIndex      = -1;
            int         displayModeIndex = -1;
            PixelType   pixelType        = PixelType::None;
            BoolOptions boolOptions;

            TL_API bool operator == (const DeviceConfig&) const;
            TL_API bool operator != (const DeviceConfig&) const;
        };

        //! HDR mode.
        enum class TL_API_TYPE HDRMode
        {
            None,
            FromFile,
            Custom,

            Count,
            First = None
        };
        TL_ENUM(HDRMode);

        //! Get HDR data from a video frame.
        TL_API std::shared_ptr<HDRData> getHDRData(const VideoFrame&);
    }
}

