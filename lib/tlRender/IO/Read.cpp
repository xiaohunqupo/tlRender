// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/Read.h>

#include <ftk/Core/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IRead::_init(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>& mem,
            const Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IIO::_init(path, options, logSystem);
            _mem = mem;
        }

        IRead::IRead()
        {}

        IRead::~IRead()
        {}

        std::future<VideoData> IRead::readVideo(
            const OTIO_NS::RationalTime&,
            const Options&)
        {
            return std::future<VideoData>();
        }

        std::future<AudioData> IRead::readAudio(
            const OTIO_NS::TimeRange&,
            const Options&)
        {
            return std::future<AudioData>();
        }

        struct IReadPlugin::Private
        {
        };

        void IReadPlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IPlugin::_init(name, extensions, logSystem);
        }

        IReadPlugin::IReadPlugin() :
            _p(new Private)
        {}

        IReadPlugin::~IReadPlugin()
        {}
    }
}
