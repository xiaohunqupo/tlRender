// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/IO.h>

namespace tl
{
    ftk::ImageType getIntImageType(std::size_t channelCount, std::size_t bitDepth)
    {
        ftk::ImageType out = ftk::ImageType::None;
        switch (channelCount)
        {
        case 1:
            switch (bitDepth)
            {
            case 8: out = ftk::ImageType::L_U8; break;
            case 16: out = ftk::ImageType::L_U16; break;
            case 32: out = ftk::ImageType::L_U32; break;
            }
            break;
        case 2:
            switch (bitDepth)
            {
            case 8: out = ftk::ImageType::LA_U8; break;
            case 16: out = ftk::ImageType::LA_U16; break;
            case 32: out = ftk::ImageType::LA_U32; break;
            }
            break;
        case 3:
            switch (bitDepth)
            {
            case 8: out = ftk::ImageType::RGB_U8; break;
            case 10: out = ftk::ImageType::RGB_U10; break;
            case 16: out = ftk::ImageType::RGB_U16; break;
            case 32: out = ftk::ImageType::RGB_U32; break;
            }
            break;
        case 4:
            switch (bitDepth)
            {
            case 8: out = ftk::ImageType::RGBA_U8; break;
            case 16: out = ftk::ImageType::RGBA_U16; break;
            case 32: out = ftk::ImageType::RGBA_U32; break;
            }
            break;
        }
        return out;
    }

    ftk::ImageType getFloatImageType(std::size_t channelCount, std::size_t bitDepth)
    {
        ftk::ImageType out = ftk::ImageType::None;
        switch (channelCount)
        {
        case 1:
            switch (bitDepth)
            {
            case 16: out = ftk::ImageType::L_F16; break;
            case 32: out = ftk::ImageType::L_F32; break;
            }
            break;
        case 2:
            switch (bitDepth)
            {
            case 16: out = ftk::ImageType::LA_F16; break;
            case 32: out = ftk::ImageType::LA_F32; break;
            }
            break;
        case 3:
            switch (bitDepth)
            {
            case 16: out = ftk::ImageType::RGB_F16; break;
            case 32: out = ftk::ImageType::RGB_F32; break;
            }
            break;
        case 4:
            switch (bitDepth)
            {
            case 16: out = ftk::ImageType::RGBA_F16; break;
            case 32: out = ftk::ImageType::RGBA_F32; break;
            }
            break;
        }
        return out;
    }

    IOOptions merge(const IOOptions& a, const IOOptions& b)
    {
        IOOptions out = b;
        for (const auto& i : a)
        {
            out[i.first] = i.second;
        }
        return out;
    }
}