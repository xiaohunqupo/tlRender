// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/FFmpegPipePrivate.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <subprocess.h>

#include <regex>

namespace tl
{
    namespace ffmpeg_pipe
    {
        Options::Options(const IOOptions& value)
        {
            auto i = value.find("FFmpegPipe/FFmpegPath");
            if (i != value.end())
            {
                std::stringstream ss(i->second);
                ss >> ffmpegPath;
            }
            i = value.find("FFmpegPipe/FFprobePath");
            if (i != value.end())
            {
                std::stringstream ss(i->second);
                ss >> ffprobePath;
            }
            i = value.find("FFmpegPipe/Codec");
            if (i != value.end())
            {
                std::stringstream ss(i->second);
                ss >> codec;
            }
            for (size_t j = 0; j < value.size(); ++j)
            {
                i = value.find(ftk::Format("FFmpegPipe/ExtraArgs{0}").arg(j));
                if (i != value.end())
                {
                    extraArgs.push_back(i->second);
                }
            }
        }

        IOOptions Options::getIOOptions() const
        {
            IOOptions out;
            out["FFmpegPipe/FFmpegPath"] = ffmpegPath;
            out["FFmpegPipe/FFprobePath"] = ffprobePath;
            out["FFmpegPipe/Codec"] = codec;
            for (size_t i = 0; i < extraArgs.size(); ++i)
            {
                out[ftk::Format("FFmpegPipe/ExtraArgs{0}").arg(i)] = extraArgs[i];
            }
            return out;
        }

        bool Options::operator == (const Options& other) const
        {
            return
                ffmpegPath == other.ffmpegPath &&
                ffprobePath == other.ffprobePath &&
                codec == other.codec &&
                extraArgs == other.extraArgs;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        Rational toRational(const std::string& value)
        {
            Rational out;
            const auto s = ftk::split(value, '/');
            if (2 == s.size())
            {
                out.first = atoi(s[0].c_str());
                out.second = atoi(s[1].c_str());
            }
            return out;
        }

        double toDouble(const Rational& value)
        {
            return value.second != 0 ?
                (value.first / static_cast<double>(value.second)) :
                0.0;
        }

        ftk::ImageType toImageType(const std::string& value)
        {
            ftk::ImageType out = ftk::ImageType::None;
            if ("gray8" == value)
            {
                out = ftk::ImageType::L_U8;
            }
            else if ("rgb24" == value)
            {
                out = ftk::ImageType::RGB_U8;
            }
            else if ("rgba" == value)
            {
                out = ftk::ImageType::RGBA_U8;
            }
            return out;
        }

        std::string fromImageType(ftk::ImageType value)
        {
            std::string out;
            switch (value)
            {
            case ftk::ImageType::L_U8: out = "gray8"; break;
            case ftk::ImageType::RGB_U8: out = "rgb24"; break;
            case ftk::ImageType::RGBA_U8: out = "rgba"; break;
            default: break;
            }
            return out;
        }

        AudioType toAudioType(const std::string& value)
        {
            AudioType out = AudioType::None;
            if ("s16" == value || "s16p" == value)
            {
                out = AudioType::S16;
            }
            else if ("s32" == value || "s32p" == value)
            {
                out = AudioType::S32;
            }
            else if ("flt" == value || "fltp" == value)
            {
                out = AudioType::F32;
            }
            else if ("dbl" == value || "dblp" == value)
            {
                out = AudioType::F64;
            }
            return out;
        }

        std::string fromAudioType(AudioType value)
        {
            std::string out;
            switch (value)
            {
                case AudioType::S16: out = "s16"; break;
                case AudioType::S32: out = "s32"; break;
                case AudioType::F32: out = "f32"; break;
                case AudioType::F64: out = "f64"; break;
                default: break;
            }
            if (!out.empty())
            {
                switch (ftk::getEndian())
                {
                    case ftk::Endian::LSB: out += "le"; break;
                    case ftk::Endian::MSB: out += "be"; break;
                    default: break;
                }
            }
            return out;
        }

        struct Pipe::Private
        {
            subprocess_s subprocess;
        };

        Pipe::Pipe(const std::vector<std::string>& cmd) :
            _p(new Private)
        {
            FTK_P();
            std::vector<const char*> args;
            for (const auto& i : cmd)
            {
                args.push_back(i.c_str());
            }
            args.push_back(nullptr);
            int r = subprocess_create(
                args.data(),
                subprocess_option_inherit_environment |
                subprocess_option_search_user_path |
                subprocess_option_enable_async,
                &p.subprocess);
            if (r != 0)
            {
                throw std::runtime_error(ftk::Format("Cannot run command: \"{0}\"").
                    arg(ftk::join(cmd, ' ')));
            }
            const std::string error = readError();
            if (!error.empty())
            {
                throw std::runtime_error(ftk::Format("Cannot run command: \"{0}\": {1}").
                    arg(ftk::join(cmd, ' ')).
                    arg(error));
            }
        }

        Pipe::~Pipe()
        {
            FTK_P();
            subprocess_terminate(&p.subprocess);
            subprocess_destroy(&p.subprocess);
        }

        size_t Pipe::read(uint8_t* data, size_t size)
        {
            FTK_P();
            size_t out = 0;
            size_t r = 0;
            do
            {
                r = subprocess_read_stdout(
                    &p.subprocess,
                    reinterpret_cast<char*>(data + out),
                    size - out);
                out += r;
            } while (subprocess_alive(&p.subprocess) != 0 && out < size && r > 0);
            return out;
        }

        std::string Pipe::readAll()
        {
            FTK_P();
            std::string out;
            size_t size = 0;
            do
            {
                const size_t chunkSize = 1024;
                std::string chunk(chunkSize, 0);
                size = subprocess_read_stdout(&p.subprocess, chunk.data(), chunkSize);
                chunk.resize(size);
                out.insert(out.end(), chunk.begin(), chunk.end());
            }
            while (subprocess_alive(&p.subprocess) != 0 && size > 0);
            return out;
        }

        std::string Pipe::readError()
        {
            FTK_P();
            std::string out;
            size_t size = 0;
            do
            {
                const size_t chunkSize = 1024;
                std::string chunk(chunkSize, 0);
                size = subprocess_read_stderr(&p.subprocess, chunk.data(), chunkSize);
                chunk.resize(size);
                out.insert(out.end(), chunk.begin(), chunk.end());
            }
            while (size > 0);
            return out;
        }

        size_t Pipe::write(const uint8_t* data, size_t size)
        {
            FTK_P();
            size_t out = 0;
            if (subprocess_alive(&p.subprocess))
            {
                fwrite(data, 1, size, subprocess_stdin(&p.subprocess));
            }
            return out;
        }

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

            extensions[".aac"] = FileType::Media;
            extensions[".aiff"] = FileType::Media;
            extensions[".flac"] = FileType::Media;
            extensions[".mp3"] = FileType::Media;
            extensions[".m4a"] = FileType::Media;
            extensions[".ogg"] = FileType::Media;
            extensions[".wav"] = FileType::Media;
            IReadPlugin::_init("FFmpegPipe", extensions, logSystem);
        }

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

        void WritePlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            std::map<std::string, FileType> extensions;
            extensions[".mov"] = FileType::Media;
            extensions[".mp4"] = FileType::Media;
            extensions[".m4v"] = FileType::Media;
            extensions[".y4m"] = FileType::Media;
            IWritePlugin::_init("FFmpegPipe", extensions, logSystem);
        }

        std::shared_ptr<WritePlugin> WritePlugin::create(
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<WritePlugin>(new WritePlugin);
            out->_init(logSystem);
            return out;
        }

        ftk::ImageInfo WritePlugin::getInfo(
            const ftk::ImageInfo& info,
            const IOOptions& options) const
        {
            ftk::ImageInfo out;
            out.size = info.size;
            switch (info.type)
            {
            case ftk::ImageType::L_U8:
            case ftk::ImageType::RGB_U8:
            case ftk::ImageType::RGBA_U8:
                out.type = info.type;
                break;
            default:
                out.type = ftk::ImageType::RGB_U8;
                break;
            }
            return out;
        }

        std::shared_ptr<IWrite> WritePlugin::write(
            const ftk::Path& path,
            const IOInfo& info,
            const IOOptions& options)
        {
            if (info.video.empty() || (!info.video.empty() && !_isCompatible(info.video[0], options)))
                throw std::runtime_error(ftk::Format("Unsupported video: \"{0}\"").
                    arg(path.get()));
            return Write::create(path, info, options, _logSystem.lock());
        }
    }
}
