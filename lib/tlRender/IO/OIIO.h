// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/SeqIO.h>

namespace tl
{
    //! OpenImageIO image I/O.
    namespace oiio
    {
        //! OpenImageIO reader.
        class TL_API_TYPE Read : public io::ISeqRead
        {
        protected:
            void _init(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            Read();

        public:
            TL_API virtual ~Read();

            //! Create a new reader.
            TL_API static std::shared_ptr<Read> create(
                const ftk::Path&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            //! Create a new reader.
            TL_API static std::shared_ptr<Read> create(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

        protected:
            io::Info _getInfo(
                const std::string& fileName,
                const ftk::MemFile*) override;
            io::VideoData _readVideo(
                const std::string& fileName,
                const ftk::MemFile*,
                const OTIO_NS::RationalTime&,
                const io::Options&) override;
        };

        //! OpenImageIO writer.
        class TL_API_TYPE Write : public io::ISeqWrite
        {
        protected:
            void _init(
                const ftk::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            Write();

        public:
            TL_API virtual ~Write();

            //! Create a new writer.
            TL_API static std::shared_ptr<Write> create(
                const ftk::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

        protected:
            void _writeVideo(
                const std::string& fileName,
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<ftk::Image>&,
                const io::Options&) override;
        };

        //! OpenImageIO read plugin.
        class TL_API_TYPE ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

            ReadPlugin() = default;

        public:
            //! Create a new plugin.
            TL_API static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            TL_API std::shared_ptr<io::IRead> read(
                const ftk::Path&,
                const io::Options& = io::Options()) override;
            TL_API std::shared_ptr<io::IRead> read(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const io::Options & = io::Options()) override;
        };

        //! OpenImageIO write plugin.
        class TL_API_TYPE WritePlugin : public io::IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

            WritePlugin() = default;

        public:
            //! Create a new plugin.
            TL_API static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            TL_API ftk::ImageInfo getInfo(
                const ftk::ImageInfo&,
                const io::Options & = io::Options()) const override;
            TL_API std::shared_ptr<io::IWrite> write(
                const ftk::Path&,
                const io::Info&,
                const io::Options & = io::Options()) override;
        };
    }
}
