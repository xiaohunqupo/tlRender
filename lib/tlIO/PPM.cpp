// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/PPM.h>

#include <feather-tk/core/Error.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/String.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace ppm
    {
        FEATHER_TK_ENUM_IMPL(
            Data,
            "ASCII",
            "Binary");

        size_t getFileScanlineByteCount(
            int    width,
            size_t channelCount,
            size_t bitDepth)
        {
            size_t chars = 0;
            switch (bitDepth)
            {
            case  8: chars = 3; break;
            case 16: chars = 5; break;
            default: break;
            }
            return (chars + 1) * width * channelCount + 1;
        }

        namespace
        {
            template<typename T>
            void _readASCII(
                const std::shared_ptr<feather_tk::FileIO>& io,
                uint8_t* out,
                size_t                               size)
            {
                char tmp[feather_tk::cStringSize] = "";
                T* outP = reinterpret_cast<T*>(out);
                for (int i = 0; i < size; ++i)
                {
                    feather_tk::readWord(io, tmp, feather_tk::cStringSize);
                    const int value = std::atoi(tmp);
                    outP[i] = value;
                }
            }

        } // namespace

        void readASCII(
            const std::shared_ptr<feather_tk::FileIO>& io,
            uint8_t* out,
            size_t                               size,
            size_t                               bitDepth)
        {
            switch (bitDepth)
            {
            case  8: _readASCII<uint8_t>(io, out, size); break;
            case 16: _readASCII<uint16_t>(io, out, size); break;
            default: break;
            }
        }

        namespace
        {
            template<typename T>
            size_t _writeASCII(
                const uint8_t* in,
                char* out,
                size_t         size)
            {
                char* outP = out;
                const T* inP = reinterpret_cast<const T*>(in);
                for (size_t i = 0; i < size; ++i)
                {
                    const std::string s = std::to_string(static_cast<unsigned int>(inP[i]));
                    const char* c = s.c_str();
                    for (size_t j = 0; j < s.size(); ++j)
                    {
                        *outP++ = c[j];
                    }
                    *outP++ = ' ';
                }
                *outP++ = '\n';
                return outP - out;
            }

        } // namespace

        size_t writeASCII(
            const uint8_t* in,
            char* out,
            size_t         size,
            size_t         bitDepth)
        {
            switch (bitDepth)
            {
            case  8: return _writeASCII<uint8_t>(in, out, size);
            case 16: return _writeASCII<uint16_t>(in, out, size);
            default: break;
            }
            return 0;
        }

        ReadPlugin::ReadPlugin()
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(
                "PPM",
                { { ".ppm", io::FileType::Sequence } },
                logSystem);
            return out;
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const io::Options& options)
        {
            return Read::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<io::IRead> ReadPlugin::read(
            const file::Path& path,
            const std::vector<feather_tk::InMemoryFile>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _logSystem.lock());
        }

        WritePlugin::WritePlugin()
        {}

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(
                "PPM",
                { { ".ppm", io::FileType::Sequence } },
                logSystem);
            return out;
        }

        feather_tk::ImageInfo WritePlugin::getInfo(
            const feather_tk::ImageInfo& info,
            const io::Options& options) const
        {
            feather_tk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case feather_tk::ImageType::L_U8:
            case feather_tk::ImageType::L_U16:
            case feather_tk::ImageType::RGB_U8:
            case feather_tk::ImageType::RGB_U16:
                out.type = info.type;
                break;
            default: break;
            }
            Data data = Data::Binary;
            auto option = options.find("PPM/Data");
            if (option != options.end())
            {
                std::stringstream ss(option->second);
                ss >> data;
            }
            out.layout.endian = Data::Binary == data ? feather_tk::Endian::MSB : feather_tk::getEndian();
            return out;
        }

        std::shared_ptr<io::IWrite> WritePlugin::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(feather_tk::Format("Unsupported video: \"{0}\"").
                    arg(path.get()));
            return Write::create(path, info, options, _logSystem.lock());
        }
    }
}
