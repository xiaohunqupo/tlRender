// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/Read.h>
#include <tlRender/IO/Write.h>

namespace tl
{
    namespace io
    {
        //! Sequence I/O options.
        struct SeqOptions
        {
            double defaultSpeed = 24.0;
            size_t threadCount = 16;

            bool operator == (const SeqOptions&) const;
            bool operator != (const SeqOptions&) const;
        };

        //! Get sequence I/O options.
        Options getOptions(const SeqOptions&);

        //! Base class for image sequence readers.
        class ISeqRead : public IRead
        {
        protected:
            void _init(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            ISeqRead();

        public:
            virtual ~ISeqRead();

            std::future<Info> getInfo() override;
            std::future<VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const Options& = Options()) override;
            void cancelRequests() override;

        protected:
            virtual Info _getInfo(
                const std::string& fileName,
                const ftk::MemFile*) = 0;
            virtual VideoData _readVideo(
                const std::string& fileName,
                const ftk::MemFile*,
                const OTIO_NS::RationalTime&,
                const Options&) = 0;

            //! \bug This must be called in the sub-class destructor.
            void _finish();

            int64_t _startFrame = 0;
            int64_t _endFrame = 0;
            float _defaultSpeed = SeqOptions().defaultSpeed;

        private:
            void _thread();
            void _finishRequests();
            void _cancelRequests();

            FTK_PRIVATE();
        };

        //! Base class for image sequence writers.
        class ISeqWrite : public IWrite
        {
        protected:
            void _init(
                const ftk::Path&,
                const Info&,
                const Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            ISeqWrite();

        public:
            virtual ~ISeqWrite();

            void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<ftk::Image>&,
                const Options& = Options()) override;

        protected:
            virtual void _writeVideo(
                const std::string& fileName,
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<ftk::Image>&,
                const Options&) = 0;

        private:
            FTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const SeqOptions&);

        void from_json(const nlohmann::json&, SeqOptions&);

        ///@}
    }
}
