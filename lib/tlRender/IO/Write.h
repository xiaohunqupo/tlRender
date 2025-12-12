// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/Plugin.h>

namespace tl
{
    namespace io
    {
        //! Base class for writers.
        class TL_API_TYPE IWrite : public IIO
        {
        protected:
            void _init(
                const ftk::Path&,
                const Options&,
                const Info&,
                const std::shared_ptr<ftk::LogSystem>&);

            IWrite();

        public:
            TL_API virtual ~IWrite();

            //! Write video data.
            TL_API virtual void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<ftk::Image>&,
                const Options& = Options()) = 0;

        protected:
            Info _info;
        };

        //! Base class for write plugins.
        class TL_API_TYPE IWritePlugin : public IPlugin
        {
            FTK_NON_COPYABLE(IWritePlugin);

        protected:
            void _init(
                const std::string& name,
                const std::map<std::string, FileType>& extensions,
                const std::shared_ptr<ftk::LogSystem>&);

            IWritePlugin();

        public:
            TL_API virtual ~IWritePlugin() = 0;

            //! Get information for writing.
            TL_API virtual ftk::ImageInfo getInfo(
                const ftk::ImageInfo&,
                const Options& = Options()) const = 0;

            //! Create a writer for the given path.
            TL_API virtual std::shared_ptr<IWrite> write(
                const ftk::Path&,
                const Info&,
                const Options& = Options()) = 0;

        protected:
            bool _isCompatible(const ftk::ImageInfo&, const Options&) const;

        private:
            FTK_PRIVATE();
        };
    }
}
