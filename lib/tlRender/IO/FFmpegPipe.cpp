// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/FFmpegPipePrivate.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <regex>

namespace tl
{
    namespace ffmpeg_pipe
    {
        struct POpen::Private
        {
            FILE* f = nullptr;
        };

        POpen::POpen(const std::string& cmd, const std::string& mode) :
            _p(new Private)
        {
            _p->f = popen(cmd.c_str(), mode.c_str());
        }

        POpen::~POpen()
        {
            pclose(_p->f);
        }

        std::string POpen::readAll()
        {
            FTK_P();
            std::string out;
            while (!feof(p.f))
            {
                const size_t chunkSize = 1024;
                std::string chunk(chunkSize, 0);
                size_t size = fread(chunk.data(), 1, chunkSize, p.f);
                chunk.resize(size);
                out.insert(out.end(), chunk.begin(), chunk.end());
            }
            return out;
        }

        FILE* POpen::f()
        {
            return _p->f;
        }

        IOOptions getOptions(const Options& value)
        {
            IOOptions out;
            out["FFmpegPipe/FFmpegPath"] = value.ffmpegPath;
            out["FFmpegPipe/FFprobePath"] = value.ffprobePath;
            return out;
        }

        Options getOptions(const IOOptions& value)
        {
            Options out;
            auto i = value.find("FFmpegPipe/FFmpegPath");
            if (i != value.end())
            {
                std::stringstream ss(i->second);
                ss >> out.ffmpegPath;
            }
            i = value.find("FFmpegPipe/FFprobePath");
            if (i != value.end())
            {
                std::stringstream ss(i->second);
                ss >> out.ffprobePath;
            }
            return out;
        }

        struct ReadPlugin::Private
        {
        };

        void ReadPlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            std::map<std::string, FileType> extensions;
            extensions[".avi"] = FileType::Media;
            extensions[".gif"] = FileType::Media;
            extensions[".mkv"] = FileType::Media;
            extensions[".mov"] = FileType::Media;
            extensions[".mp4"] = FileType::Media;
            extensions[".m4v"] = FileType::Media;
            extensions[".y4m"] = FileType::Media;
            extensions[".webm"] = FileType::Media;

            extensions[".aac"] = FileType::Media;
            extensions[".aiff"] = FileType::Media;
            extensions[".flac"] = FileType::Media;
            extensions[".mp3"] = FileType::Media;
            extensions[".m4a"] = FileType::Media;
            extensions[".ogg"] = FileType::Media;
            extensions[".wav"] = FileType::Media;
            IReadPlugin::_init("FFmpegPipe", extensions, logSystem);
            FTK_P();
        }

        ReadPlugin::ReadPlugin() :
            _p(new Private)
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<IRead> ReadPlugin::read(
            const ftk::Path& path,
            const IOOptions& options)
        {
            return Read::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<IRead> ReadPlugin::read(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& memory,
            const IOOptions& options)
        {
            return Read::create(path, memory, options, _logSystem.lock());
        }
    }
}
