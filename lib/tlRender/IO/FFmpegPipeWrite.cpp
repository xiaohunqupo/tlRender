// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/FFmpegPipePrivate.h>

#include <ftk/Core/Format.h>

namespace tl
{
    namespace ffmpeg_pipe
    {
        struct Write::Private
        {
            std::string fileName;
            //std::shared_ptr<POpen> pipe;
        };

        void Write::_init(
            const ftk::Path& path,
            const IOInfo& info,
            const IOOptions& ioOptions,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IWrite::_init(path, ioOptions, info, logSystem);

            FTK_P();

            p.fileName = path.get();
            if (info.video.empty())
            {
                throw std::runtime_error(ftk::Format("No video: \"{0}\"").arg(p.fileName));
            }

            const Options options(ioOptions);
            const auto& imageInfo = info.video.front();
            const std::string cmd = ftk::Format("{0} -v quiet -f rawvideo -video_size {1}x{2} -pixel_format {3} -framerate {4} -i pipe:0 -c:v {5} {6} -y {7}").
                arg(options.ffmpegPath).
                arg(imageInfo.size.w).
                arg(imageInfo.size.h).
                arg(fromImageType(imageInfo.type)).
                arg(info.videoTime.duration().rate()).
                arg(options.codec).
                arg(ftk::join(options.extraArgs, ' ')).
                arg(p.fileName);
            //std::cout << cmd << std::endl;
            //p.pipe = std::make_shared<POpen>(cmd, "w");
        }

        Write::Write() :
            _p(new Private)
        {}

        Write::~Write()
        {
            FTK_P();
        }

        std::shared_ptr<Write> Write::create(
            const ftk::Path& path,
            const IOInfo& info,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Write>(new Write);
            out->_init(path, info, options, logSystem);
            return out;
        }

        void Write::writeVideo(
            const OTIO_NS::RationalTime& time,
            const std::shared_ptr<ftk::Image>& image,
            const IOOptions&)
        {
            FTK_P();
            const ftk::Size2I& size = image->getSize();
            const size_t scanlineByteCount =
                size.w *
                ftk::getChannelCount(image->getType()) *
                ftk::getBitDepth(image->getType()) / 8;
            const uint8_t* data = image->getData();
            for (int y = 0; y < size.h; ++y)
            {
                /*if (fwrite(
                    data + (size.h - 1 - y) * scanlineByteCount,
                    1,
                    scanlineByteCount,
                    p.pipe->f()) < scanlineByteCount)
                {
                    throw std::runtime_error(ftk::Format("Cannot write: \"{0}\"").arg(p.fileName));
                }*/
            }
        }
    }
}
