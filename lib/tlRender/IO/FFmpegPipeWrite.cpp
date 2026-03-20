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
            std::shared_ptr<Pipe> pipe;
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
            std::vector<std::string> cmd;
            cmd.push_back(options.ffmpegPath);
            cmd.push_back("-v");
            cmd.push_back("quiet");
            cmd.push_back("-f");
            cmd.push_back("rawvideo");
            cmd.push_back("-video_size");
            cmd.push_back(ftk::Format("{0}x{1}").
                arg(imageInfo.size.w).
                arg(imageInfo.size.h));
            cmd.push_back("-pixel_format");
            cmd.push_back(fromImageType(imageInfo.type));
            cmd.push_back("-framerate");
            cmd.push_back(ftk::Format("{0}").
                arg(info.videoTime.duration().rate()));
            cmd.push_back("-i");
            cmd.push_back("pipe:0");
            cmd.push_back("-c:v");
            cmd.push_back(options.codec);
            cmd.insert(cmd.end(), options.extraArgs.begin(), options.extraArgs.end());
            cmd.push_back("-y");
            cmd.push_back(p.fileName);
            //std::cout << ftk::join(cmd, ' ') << std::endl;
            p.pipe = std::make_shared<Pipe>(cmd);
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
                if (p.pipe->write(
                    data + (size.h - 1 - y) * scanlineByteCount,
                    scanlineByteCount) < scanlineByteCount)
                {
                    throw std::runtime_error(ftk::Format("Cannot write: \"{0}\"").arg(p.fileName));
                }
            }
        }
    }
}
