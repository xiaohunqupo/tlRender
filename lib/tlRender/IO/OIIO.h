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
        class Read : public io::ISeqRead
        {
        protected:
            void _init(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            Read();

        public:
            virtual ~Read();

            //! Create a new reader.
            static std::shared_ptr<Read> create(
                const ftk::Path&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            //! Create a new reader.
            static std::shared_ptr<Read> create(
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
        class Write : public io::ISeqWrite
        {
        protected:
            void _init(
                const ftk::Path&,
                const io::Info&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            Write();

        public:
            virtual ~Write();

            //! Create a new writer.
            static std::shared_ptr<Write> create(
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
        class ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

            ReadPlugin() = default;

        public:
            //! Create a new plugin.
            static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            std::shared_ptr<io::IRead> read(
                const ftk::Path&,
                const io::Options& = io::Options()) override;
            std::shared_ptr<io::IRead> read(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const io::Options & = io::Options()) override;
        };

        //! OpenImageIO write plugin.
        class WritePlugin : public io::IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

            WritePlugin() = default;

        public:
            //! Create a new plugin.
            static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            ftk::ImageInfo getInfo(
                const ftk::ImageInfo&,
                const io::Options & = io::Options()) const override;
            std::shared_ptr<io::IWrite> write(
                const ftk::Path&,
                const io::Info&,
                const io::Options & = io::Options()) override;
        };
    }
}
