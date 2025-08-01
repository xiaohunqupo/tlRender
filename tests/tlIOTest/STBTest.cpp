// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlIOTest/STBTest.h>

#include <tlIO/STB.h>
#include <tlIO/System.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        STBTest::STBTest(const std::shared_ptr<feather_tk::Context>& context) :
            ITest(context, "io_test::STBTest")
        {}

        std::shared_ptr<STBTest> STBTest::create(const std::shared_ptr<feather_tk::Context>& context)
        {
            return std::shared_ptr<STBTest>(new STBTest(context));
        }

        void STBTest::run()
        {
            _io();
        }

        namespace
        {
            void write(
                const std::shared_ptr<io::IWritePlugin>& plugin,
                const std::shared_ptr<feather_tk::Image>& image,
                const file::Path& path,
                const feather_tk::ImageInfo& imageInfo)
            {
                Info info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), OTIO_NS::RationalTime(1.0, 24.0));
                auto write = plugin->write(path, info);
                write->writeVideo(OTIO_NS::RationalTime(0.0, 24.0), image);
            }

            void read(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<feather_tk::Image>& image,
                const file::Path& path,
                bool memoryIO)
            {
                std::vector<uint8_t> memoryData;
                std::vector<feather_tk::InMemoryFile> memory;
                std::shared_ptr<io::IRead> read;
                if (memoryIO)
                {
                    auto fileIO = feather_tk::FileIO::create(path.get(), feather_tk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(feather_tk::InMemoryFile(memoryData.data(), memoryData.size()));
                    read = plugin->read(path, memory);
                }
                else
                {
                    read = plugin->read(path);
                }
                const auto ioInfo = read->getInfo().get();
                FEATHER_TK_ASSERT(!ioInfo.video.empty());
                const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
                FEATHER_TK_ASSERT(videoData.image);
                FEATHER_TK_ASSERT(videoData.image->getSize() == image->getSize());
                //! \todo Compare image data.
                //FEATHER_TK_ASSERT(0 == memcmp(
                //    videoData.image->getData(),
                //    image->getData(),
                //    image->getDataByteCount()));
            }

            void readError(
                const std::shared_ptr<io::IReadPlugin>& plugin,
                const std::shared_ptr<feather_tk::Image>& image,
                const file::Path& path,
                bool memoryIO)
            {
                {
                    auto io = feather_tk::FileIO::create(path.get(), feather_tk::FileMode::Read);
                    const size_t size = io->getSize();
                    io.reset();
                    feather_tk::truncateFile(path.get(), size / 2);
                }
                std::vector<uint8_t> memoryData;
                std::vector<feather_tk::InMemoryFile> memory;
                if (memoryIO)
                {
                    auto fileIO = feather_tk::FileIO::create(path.get(), feather_tk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(feather_tk::InMemoryFile(memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory);
                const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
            }
        }

        void STBTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<stb::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<stb::WritePlugin>();

            const std::vector<std::string> fileNames =
            {
                "STBTest",
                "大平原"
            };
            const std::vector<bool> memoryIOList =
            {
                false,
                true
            };
            const std::vector<feather_tk::Size2I> sizes =
            {
                feather_tk::Size2I(16, 16),
                feather_tk::Size2I(1, 1),
                feather_tk::Size2I(0, 0)
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto pixelType : feather_tk::getImageTypeEnums())
                        {
                            const auto imageInfo = writePlugin->getInfo(feather_tk::ImageInfo(size, pixelType));
                            if (imageInfo.isValid())
                            {
                                file::Path path;
                                {
                                    std::stringstream ss;
                                    ss << fileName << '_' << size << '_' << pixelType << ".0.tga";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                auto image = feather_tk::Image::create(imageInfo);
                                image->zero();
                                try
                                {
                                    write(writePlugin, image, path, imageInfo);
                                    read(readPlugin, image, path, memoryIO);
                                    readError(readPlugin, image, path, memoryIO);
                                }
                                catch (const std::exception& e)
                                {
                                    _printError(e.what());
                                }
                                {
                                    std::stringstream ss;
                                    ss << fileName << '_' << size << '_' << pixelType << ".0.bmp";
                                    _print(ss.str());
                                    path = file::Path(ss.str());
                                }
                                try
                                {
                                    write(writePlugin, image, path, imageInfo);
                                    read(readPlugin, image, path, memoryIO);
                                    readError(readPlugin, image, path, memoryIO);
                                }
                                catch (const std::exception& e)
                                {
                                    _printError(e.what());
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
