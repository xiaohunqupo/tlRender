// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIO/DPX.h>

#include <sstream>

namespace tl
{
    namespace dpx
    {
        void Read::_init(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            ISequenceRead::_init(path, memory, options, cache, logSystem);
        }

        Read::Read()
        {}

        Read::~Read()
        {
            _finish();
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, cache, logSystem);
            return out;
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const std::vector<dtk::InMemoryFile>& memory,
            const io::Options& options,
            const std::shared_ptr<io::Cache>& cache,
            const std::shared_ptr<dtk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, memory, options, cache, logSystem);
            return out;
        }

        io::Info Read::_getInfo(
            const std::string& fileName,
            const dtk::InMemoryFile* memory)
        {
            io::Info out;
            auto io = memory ?
                dtk::FileIO::create(fileName, *memory) :
                dtk::FileIO::create(fileName, dtk::FileMode::Read);
            Transfer transfer = Transfer::User;
            const auto header = read(io, out, transfer);
            float speed = _defaultSpeed;
            auto i = out.tags.find("Film Frame Rate");
            if (i != out.tags.end())
            {
                speed = std::stof(i->second);
            }
            else
            {
                i = out.tags.find("TV Frame Rate");
                if (i != out.tags.end())
                {
                    speed = std::stof(i->second);
                }
            }
            out.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, speed),
                OTIO_NS::RationalTime(_endFrame, speed));
            return out;
        }

        io::VideoData Read::_readVideo(
            const std::string& fileName,
            const dtk::InMemoryFile* memory,
            const OTIO_NS::RationalTime& time,
            const io::Options&)
        {
            io::VideoData out;
            out.time = time;

            auto io = memory ?
                dtk::FileIO::create(fileName, *memory) :
                dtk::FileIO::create(fileName, dtk::FileMode::Read);
            io::Info info;
            Transfer transfer = Transfer::User;
            read(io, info, transfer);

            out.image = dtk::Image::create(info.video[0]);
            out.image->setTags(info.tags);
            io->read(out.image->getData(), info.video[0].getByteCount());
            return out;
        }
    }
}
