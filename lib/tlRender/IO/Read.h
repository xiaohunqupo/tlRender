// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/Plugin.h>

namespace tl
{
    namespace io
    {
        //! Base class for readers.
        class TL_API_TYPE IRead : public IIO
        {
        protected:
            void _init(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            IRead();

        public:
            TL_API virtual ~IRead();

            //! Get the information.
            TL_API virtual std::future<Info> getInfo() = 0;

            //! Read video data.
            TL_API virtual std::future<VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const Options& = Options());

            //! Read audio data.
            TL_API virtual std::future<AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const Options& = Options());

            //! Cancel pending requests.
            TL_API virtual void cancelRequests() = 0;

        protected:
            std::vector<ftk::MemFile> _mem;
        };

        //! Base class for read plugins.
        class TL_API_TYPE IReadPlugin : public IPlugin
        {
            FTK_NON_COPYABLE(IReadPlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<ftk::LogSystem>&);

            IReadPlugin();

        public:
            TL_API virtual ~IReadPlugin() = 0;

            //! Create a reader for the given path.
            TL_API virtual std::shared_ptr<IRead> read(
                const ftk::Path&,
                const Options& = Options()) = 0;

            //! Create a reader for the given path and memory locations.
            TL_API virtual std::shared_ptr<IRead> read(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const Options& = Options()) = 0;

        private:
            FTK_PRIVATE();
        };
    }
}
