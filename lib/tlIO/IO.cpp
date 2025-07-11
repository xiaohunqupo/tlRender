// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/IO.h>

namespace tl
{
    namespace io
    {
        feather_tk::ImageType getIntType(std::size_t channelCount, std::size_t bitDepth)
        {
            feather_tk::ImageType out = feather_tk::ImageType::None;
            switch (channelCount)
            {
            case 1:
                switch (bitDepth)
                {
                case 8: out = feather_tk::ImageType::L_U8; break;
                case 16: out = feather_tk::ImageType::L_U16; break;
                case 32: out = feather_tk::ImageType::L_U32; break;
                }
                break;
            case 2:
                switch (bitDepth)
                {
                case 8: out = feather_tk::ImageType::LA_U8; break;
                case 16: out = feather_tk::ImageType::LA_U16; break;
                case 32: out = feather_tk::ImageType::LA_U32; break;
                }
                break;
            case 3:
                switch (bitDepth)
                {
                case 8: out = feather_tk::ImageType::RGB_U8; break;
                case 10: out = feather_tk::ImageType::RGB_U10; break;
                case 16: out = feather_tk::ImageType::RGB_U16; break;
                case 32: out = feather_tk::ImageType::RGB_U32; break;
                }
                break;
            case 4:
                switch (bitDepth)
                {
                case 8: out = feather_tk::ImageType::RGBA_U8; break;
                case 16: out = feather_tk::ImageType::RGBA_U16; break;
                case 32: out = feather_tk::ImageType::RGBA_U32; break;
                }
                break;
            }
            return out;
        }

        feather_tk::ImageType getFloatType(std::size_t channelCount, std::size_t bitDepth)
        {
            feather_tk::ImageType out = feather_tk::ImageType::None;
            switch (channelCount)
            {
            case 1:
                switch (bitDepth)
                {
                case 16: out = feather_tk::ImageType::L_F16; break;
                case 32: out = feather_tk::ImageType::L_F32; break;
                }
                break;
            case 2:
                switch (bitDepth)
                {
                case 16: out = feather_tk::ImageType::LA_F16; break;
                case 32: out = feather_tk::ImageType::LA_F32; break;
                }
                break;
            case 3:
                switch (bitDepth)
                {
                case 16: out = feather_tk::ImageType::RGB_F16; break;
                case 32: out = feather_tk::ImageType::RGB_F32; break;
                }
                break;
            case 4:
                switch (bitDepth)
                {
                case 16: out = feather_tk::ImageType::RGBA_F16; break;
                case 32: out = feather_tk::ImageType::RGBA_F32; break;
                }
                break;
            }
            return out;
        }

        Options merge(const Options& a, const Options& b)
        {
            Options out = b;
            for (const auto& i : a)
            {
                out[i.first] = i.second;
            }
            return out;
        }
    }
}
