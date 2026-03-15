// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <IOTest/IOTest.h>

#include <tlRender/IO/System.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <sstream>

namespace tl
{
    namespace io_tests
    {
        IOTest::IOTest(const std::shared_ptr<ftk::Context>& context) :
            ITest(context, "IOTest::IOTest")
        {}

        std::shared_ptr<IOTest> IOTest::create(const std::shared_ptr<ftk::Context>& context)
        {
            return std::shared_ptr<IOTest>(new IOTest(context));
        }

        void IOTest::run()
        {
            _videoData();
            _ioSystem();
        }

        void IOTest::_videoData()
        {
            {
                const VideoData v;
                FTK_ASSERT(!isValid(v.time));
                FTK_ASSERT(!v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = ftk::Image::create(160, 80, ftk::ImageType::L_U8);
                const VideoData v(time, layer, image);
                FTK_ASSERT(time.strictly_equal(v.time));
                FTK_ASSERT(layer == v.layer);
                FTK_ASSERT(image == v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = ftk::Image::create(16, 16, ftk::ImageType::L_U8);
                const VideoData a(time, layer, image);
                VideoData b(time, layer, image);
                FTK_ASSERT(a == b);
                b.time = OTIO_NS::RationalTime(2.0, 24.0);
                FTK_ASSERT(a != b);
                FTK_ASSERT(a < b);
            }
        }

        namespace
        {
            class DummyReadPlugin : public IReadPlugin
            {
            public:
                std::shared_ptr<IRead> read(
                    const ftk::Path&,
                    const IOOptions& = IOOptions()) override
                {
                    return nullptr;
                }
            };

            class DummyWritePlugin : public IWritePlugin
            {
            public:
                ftk::ImageInfo getInfo(
                    const ftk::ImageInfo&,
                    const IOOptions& = IOOptions()) const override
                {
                    return ftk::ImageInfo();
                }

                std::shared_ptr<IWrite> write(
                    const ftk::Path&,
                    const IOInfo&,
                    const IOOptions& = IOOptions()) override
                {
                    return nullptr;
                }
            };
        }

        void IOTest::_ioSystem()
        {
            auto readSystem = _context->getSystem<ReadSystem>();
            {
                for (const auto& plugin : readSystem->getPlugins())
                {
                    const auto& exts = plugin->getExts();
                    std::stringstream ss;
                    ss << plugin->getName() << ": " <<
                        ftk::join(std::vector<std::string>(exts.begin(), exts.end()), ", ");
                    _print(ss.str());
                }
            }
            FTK_ASSERT(!readSystem->read(ftk::Path()));
            auto writeSystem = _context->getSystem<WriteSystem>();
            FTK_ASSERT(!writeSystem->write(ftk::Path(), IOInfo()));
        }
    }
}
