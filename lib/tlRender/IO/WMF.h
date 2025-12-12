// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/Read.h>

namespace tl
{
    namespace wmf
    {
        //! WMF options.
        struct TL_API_TYPE Options
        {
            TL_API bool operator == (const Options&) const;
            TL_API bool operator != (const Options&) const;
        };

        //! Get WMF options.
        TL_API io::Options getOptions(const Options&);

        //! WMF reader
        class TL_API_TYPE Read : public io::IRead
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

            TL_API std::future<io::Info> getInfo() override;
            TL_API std::future<io::VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const io::Options& = io::Options()) override;
            TL_API std::future<io::AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const io::Options& = io::Options()) override;
            TL_API void cancelRequests() override;

        private:
            void _thread(const ftk::Path&);

            FTK_PRIVATE();
        };

        //! WMF read plugin.
        class TL_API_TYPE ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

            ReadPlugin();

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
                const io::Options& = io::Options()) override;

        private:
            FTK_PRIVATE();
        };

        //! \name Serialize
        ///@{

        TL_API void to_json(nlohmann::json&, const Options&);

        TL_API void from_json(const nlohmann::json&, Options&);

        ///@}
    }
}
