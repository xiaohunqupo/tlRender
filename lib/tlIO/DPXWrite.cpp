// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/DPX.h>

#include <sstream>

namespace tl
{
    namespace dpx
    {
        void Write::_init(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            ISequenceWrite::_init(path, info, options, logSystem);
        }

        Write::Write()
        {}

        Write::~Write()
        {}

        std::shared_ptr<Write> Write::create(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options,
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::_writeVideo(
            const std::string& fileName,
            const OTIO_NS::RationalTime&,
            const std::shared_ptr<feather_tk::Image>& image,
            const io::Options&)
        {
            auto io = feather_tk::FileIO::create(fileName, feather_tk::FileMode::Write);

            io::Info info;
            const auto& imageInfo = image->getInfo();
            info.video.push_back(imageInfo);
            info.tags = image->getTags();

            Version version = Version::_2_0;
            auto i = _options.find("DPX/Version");
            if (i != _options.end())
            {
                std::stringstream ss(i->second);
                ss >> version;
            }
            Endian endian = Endian::Auto;
            i = _options.find("DPX/Endian");
            if (i != _options.end())
            {
                std::stringstream ss(i->second);
                ss >> endian;
            }
            Transfer transfer = Transfer::FilmPrint;
            write(io, info, version, endian, transfer);

            const size_t scanlineByteCount = feather_tk::getAlignedByteCount(
                static_cast<size_t>(imageInfo.size.w) * 4,
                imageInfo.layout.alignment);
            const uint8_t* imageP = image->getData() + (imageInfo.size.h - 1) * scanlineByteCount;
            for (uint16_t y = 0; y < imageInfo.size.h; ++y, imageP -= scanlineByteCount)
            {
                io->write(imageP, scanlineByteCount);
            }

            finishWrite(io);
        }
    }
}
