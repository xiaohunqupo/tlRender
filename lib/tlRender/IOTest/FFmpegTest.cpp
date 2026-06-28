// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IOTest/FFmpegTest.h>

#include <tlRender/IO/FFmpeg.h>
#include <tlRender/IO/System.h>

#include <ftk/Core/Assert.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/FileIO.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace tl
{
    namespace io_tests
    {
        FFmpegTest::FFmpegTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "io_tests::FFmpegTest")
        {}

        std::shared_ptr<FFmpegTest> FFmpegTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<FFmpegTest>(new FFmpegTest(context));
        }

        void FFmpegTest::run()
        {
            _convert();
            _io();
        }

        namespace
        {
            void write(
                const std::shared_ptr<IWritePlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const ftk::Path& path,
                const ftk::ImageInfo& imageInfo,
                const ftk::ImageTags& tags,
                const OTIO_NS::RationalTime& duration,
                const IOOptions& options)
            {
                IOInfo info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(OTIO_NS::RationalTime(0.0, 24.0), duration);
                info.tags = tags;
                auto write = plugin->write(path, info, options);
                for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                {
                    write->writeVideo(OTIO_NS::RationalTime(i, 24.0), image);
                }
            }

            void read(
                const std::shared_ptr<IReadPlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const ftk::Path& path,
                bool memoryIO,
                const ftk::ImageTags& tags,
                const OTIO_NS::RationalTime& duration,
                const IOOptions& options)
            {
                std::vector<uint8_t> memoryData;
                std::vector<ftk::MemFile> memory;
                std::shared_ptr<IRead> read;
                if (memoryIO)
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(ftk::MemFile(nullptr, memoryData.data(), memoryData.size()));
                    read = plugin->read(path, memory, options);
                }
                else
                {
                    read = plugin->read(path, options);
                }
                const auto ioInfo = read->getInfo().get();
                FTK_ASSERT(!ioInfo.video.empty());
                for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                {
                    const auto videoData = read->readVideo(OTIO_NS::RationalTime(i, 24.0)).get();
                    FTK_ASSERT(videoData.image);
                    FTK_ASSERT(videoData.image->getSize() == image->getSize());
                    const auto frameTags = videoData.image->getTags();
                    for (const auto& j : tags)
                    {
                        const auto k = frameTags.find(j.first);
                        FTK_ASSERT(k != frameTags.end());
                        FTK_ASSERT(k->second == j.second);
                    }
                }
                for (size_t i = 0; i < static_cast<size_t>(duration.value()); ++i)
                {
                    const auto videoData = read->readVideo(OTIO_NS::RationalTime(i, 24.0)).get();
                }
            }

            void readError(
                const std::shared_ptr<IReadPlugin>& plugin,
                const std::shared_ptr<ftk::Image>& image,
                const ftk::Path& path,
                bool memoryIO,
                const IOOptions& options)
            {
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    const size_t size = fileIO->getSize();
                    fileIO.reset();
                    ftk::truncateFile(path.get(), size / 2);
                }
                std::vector<uint8_t> memoryData;
                std::vector<ftk::MemFile> memory;
                if (memoryIO)
                {
                    auto fileIO = ftk::FileIO::create(path.get(), ftk::FileMode::Read);
                    memoryData.resize(fileIO->getSize());
                    fileIO->read(memoryData.data(), memoryData.size());
                    memory.push_back(ftk::MemFile(nullptr, memoryData.data(), memoryData.size()));
                }
                auto read = plugin->read(path, memory, options);
                //! \bug This causes the test to hang.
                //const auto videoData = read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
            }
        }

        void FFmpegTest::_io()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<ffmpeg::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<ffmpeg::WritePlugin>();

            const ftk::ImageTags tags =
            {
                { "artist", "artist" },
                { "comment", "comment" },
                { "title", "title" }
            };
            const std::vector<std::string> fileNames =
            {
                "FFmpegTest",
                "大平原"
            };
            const std::vector<bool> memoryIOList =
            {
                false,
                true
            };
            const std::vector<ftk::Size2I> sizes =
            {
                ftk::Size2I(640, 480),
                ftk::Size2I(80, 60),
                ftk::Size2I(0, 0)
            };
            const std::vector<std::pair<std::string, std::string> > options =
            {
                { "FFmpeg/YUVToRGB", "1" },
                { "FFmpeg/ThreadCount", "1" },
                { "FFmpeg/RequestTimeout", "1" },
                { "FFmpeg/VideoBufferSize", "1" },
                { "FFmpeg/AudioBufferSize", "1/1" },
                { "FFmpeg/Codec", "mjpeg" },
                { "FFmpeg/Codec", "v210" },
                { "FFmpeg/Codec", "v410" }
            };

            for (const auto& fileName : fileNames)
            {
                for (const bool memoryIO : memoryIOList)
                {
                    for (const auto& size : sizes)
                    {
                        for (const auto pixelType : ftk::getImageTypeEnums())
                        {
                            for (const auto& option : options)
                            {
                                IOOptions options;
                                options[option.first] = option.second;
                                const auto imageInfo = writePlugin->getInfo(ftk::ImageInfo(size, pixelType));
                                if (imageInfo.isValid())
                                {
                                    ftk::Path path;
                                    {
                                        std::stringstream ss;
                                        ss << fileName << ' ' << size << ' ' << pixelType << ".mp4";
                                        _print(ss.str());
                                        path = ftk::Path(ss.str());
                                    }
                                    auto image = ftk::Image::create(imageInfo);
                                    image->zero();
                                    image->setTags(tags);
                                    const OTIO_NS::RationalTime duration(24.0, 24.0);
                                    try
                                    {
                                        write(writePlugin, image, path, imageInfo, tags, duration, options);
                                        read(readPlugin, image, path, memoryIO, tags, duration, options);
                                        readError(readPlugin, image, path, memoryIO, options);
                                    }
                                    catch (const std::exception& e)
                                    {
                                        _error(e.what());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void FFmpegTest::_convert()
        {
            // Round-trip a known color image through the writer and back through
            // the reader with YUV->RGB conversion forced ON, then check pixels.
            //
            // The pre-existing _io() test writes a *zeroed* (black) image and
            // only asserts size and tags -- black survives every color bug, so
            // it cannot catch a broken conversion. This test paints vertical
            // color stripes (invariant under the reader's y-mirror) and verifies
            // each stripe's center survives the round-trip. The tolerance is
            // deliberately loose: it sits far above codec + colorspace rounding
            // on a flat stripe interior, but far below a gross failure such as
            // black output or a saturated channel. It is a regression guard for
            // the conversion path, not a colorimetric assertion.
            auto readSystem = _context->getSystem<ReadSystem>();
            auto readPlugin = readSystem->getPlugin<ffmpeg::ReadPlugin>();
            auto writeSystem = _context->getSystem<WriteSystem>();
            auto writePlugin = writeSystem->getPlugin<ffmpeg::WritePlugin>();

            // Expected stripe colors, full-range RGB (16-bit). Primaries near
            // saturation plus near-black and near-white exercise the matrix and
            // the limited/full range handling.
            const std::array<std::array<uint16_t, 3>, 6> stripes =
            {{
                {{ 60000,  2000,  2000 }},  // red
                {{  2000, 60000,  2000 }},  // green
                {{  2000,  2000, 60000 }},  // blue
                {{ 32768, 32768, 32768 }},  // mid gray
                {{  4000,  4000,  4000 }},  // near black
                {{ 61000, 61000, 61000 }}   // near white
            }};

            const ftk::Size2I size(360, 240);
            const int stripeW = size.w / static_cast<int>(stripes.size());

            const auto imageInfo = ftk::ImageInfo(size, ftk::ImageType::RGB_U16);
            if (!writePlugin->getInfo(imageInfo).isValid())
            {
                _print("_convert: writer does not support RGB_U16; skipping");
                return;
            }
            auto image = ftk::Image::create(imageInfo);
            {
                uint16_t* p = reinterpret_cast<uint16_t*>(image->getData());
                for (int y = 0; y < size.h; ++y)
                {
                    for (int x = 0; x < size.w; ++x)
                    {
                        const size_t s = std::min<size_t>(
                            x / stripeW, stripes.size() - 1);
                        *p++ = stripes[s][0];
                        *p++ = stripes[s][1];
                        *p++ = stripes[s][2];
                    }
                }
            }

            // Sample a pixel as normalized [0,1] RGB, tolerating whatever bit
            // depth the conversion produced (8- or 16-bit RGB).
            auto sample = [](
                const std::shared_ptr<ftk::Image>& img,
                int x, int y) -> std::array<float, 3>
            {
                const int w = img->getSize().w;
                const uint8_t* const d = img->getData();
                std::array<float, 3> out = {{ 0.f, 0.f, 0.f }};
                switch (img->getInfo().type)
                {
                case ftk::ImageType::RGB_U16:
                {
                    const uint16_t* const px =
                        reinterpret_cast<const uint16_t*>(d) +
                        (static_cast<size_t>(y) * w + x) * 3;
                    for (int c = 0; c < 3; ++c) out[c] = px[c] / 65535.f;
                    break;
                }
                case ftk::ImageType::RGB_U8:
                {
                    const uint8_t* const px =
                        d + (static_cast<size_t>(y) * w + x) * 3;
                    for (int c = 0; c < 3; ++c) out[c] = px[c] / 255.f;
                    break;
                }
                default: break;
                }
                return out;
            };

            const ftk::Path path("FFmpegConvertTest.mov");
            try
            {
                IOInfo info;
                info.video.push_back(imageInfo);
                info.videoTime = OTIO_NS::TimeRange(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::RationalTime(1.0, 24.0));
                {
                    auto write = writePlugin->write(path, info, IOOptions());
                    write->writeVideo(OTIO_NS::RationalTime(0.0, 24.0), image);
                }

                IOOptions readOptions;
                readOptions["FFmpeg/YUVToRGB"] = "1";
                auto read = readPlugin->read(path, readOptions);
                const auto ioInfo = read->getInfo().get();
                FTK_ASSERT(!ioInfo.video.empty());
                const auto videoData =
                    read->readVideo(OTIO_NS::RationalTime(0.0, 24.0)).get();
                FTK_ASSERT(videoData.image);

                const ftk::ImageType type = videoData.image->getInfo().type;
                if (type != ftk::ImageType::RGB_U16 &&
                    type != ftk::ImageType::RGB_U8)
                {
                    _print("_convert: unexpected output type; skipping pixel check");
                    return;
                }

                const int yMid = videoData.image->getSize().h / 2;
                const float tolerance = 0.12f; // ~8000/65535
                for (size_t i = 0; i < stripes.size(); ++i)
                {
                    const int xMid =
                        static_cast<int>(i) * stripeW + stripeW / 2;
                    const auto got = sample(videoData.image, xMid, yMid);
                    const std::array<float, 3> want =
                    {{
                        stripes[i][0] / 65535.f,
                        stripes[i][1] / 65535.f,
                        stripes[i][2] / 65535.f
                    }};
                    for (int c = 0; c < 3; ++c)
                    {
                        if (std::fabs(got[c] - want[c]) > tolerance)
                        {
                            std::stringstream ss;
                            ss << "Color conversion mismatch, stripe " << i <<
                                " channel " << c << ": expected " << want[c] <<
                                " got " << got[c];
                            _error(ss.str());
                        }
                        FTK_ASSERT(std::fabs(got[c] - want[c]) <= tolerance);
                    }
                }
            }
            catch (const std::exception& e)
            {
                _error(e.what());
            }
        }
    }
}
