// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/AVF.h>

//#include <ftk/Core/Assert.h>
//#include <ftk/Core/Format.h>
//#include <ftk/Core/LogSystem.h>

namespace tl
{
    namespace avf
    {
        bool Options::operator == (const Options&) const
        {
            return true;
        }

        bool Options::operator != (const Options& other) const
        {
            return !(*this == other);
        }

        IOOptions getOptions(const Options&)
        {
            return IOOptions();
        }

        struct ReadPlugin::Private {};

        void ReadPlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            std::map<std::string, FileType> extensions;
            extensions[".mov"] = FileType::Media;
            extensions[".mp4"] = FileType::Media;
            extensions[".m4v"] = FileType::Media;
            extensions[".m4a"] = FileType::Media;
            extensions[".mp3"] = FileType::Media;
            extensions[".aac"] = FileType::Media;
            extensions[".wav"] = FileType::Media;
            extensions[".aif"] = FileType::Media;
            extensions[".aiff"] = FileType::Media;
            IReadPlugin::_init("AVFoundation", extensions, logSystem);
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

        void to_json(nlohmann::json&, const Options&) {}
        void from_json(const nlohmann::json&, Options&) {}
    }
}
