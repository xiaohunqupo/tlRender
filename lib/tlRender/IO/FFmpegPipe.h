// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/Read.h>
#include <tlRender/IO/Write.h>

namespace tl
{
    //! FFmpeg video and audio I/O via pipe
    namespace ffmpeg_pipe
    {
        //! FFmpeg pipe options.
        //!
        //! References:
        //! * https://academysoftwarefoundation.github.io/EncodingGuidelines/EncodeVP9.html
        struct TL_API_TYPE Options
        {
            Options() = default;
            Options(const IOOptions&);

            std::string ffmpegPath  = "ffmpeg";
            std::string ffprobePath = "ffprobe";
            std::string codec = "vp9";
            std::vector<std::string> extraArgs =
            {
                "-crf", "22",
                "-b:v", "0",
                "-quality", "good",
                "-pix_fmt", "yuv420p10le",
                "-row-mt", "1",
                "-sws_flags", "spline+accurate_rnd+full_chroma_int",
                "-vf", "'scale=in_range=full:in_color_matrix=bt709:out_range=tv:out_color_matrix=bt709'",
                "-color_range", "tv",
                "-colorspace", "bt709",
                "-color_primaries",  "bt709",
                "-color_trc", "iec61966-2-1"
            };

            TL_API IOOptions getIOOptions() const;

            TL_API bool operator == (const Options&) const;
            TL_API bool operator != (const Options&) const;
        };

        //! FFmpeg pipe reader.
        class TL_API_TYPE Read : public IRead
        {
        protected:
            void _init(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const IOOptions&,
                const std::shared_ptr<ftk::LogSystem>&);

            Read();

        public:
            TL_API virtual ~Read();

            //! Create a new reader.
            TL_API static std::shared_ptr<Read> create(
                const ftk::Path&,
                const IOOptions&,
                const std::shared_ptr<ftk::LogSystem>&);

            //! Create a new reader.
            TL_API static std::shared_ptr<Read> create(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const IOOptions&,
                const std::shared_ptr<ftk::LogSystem>&);

            TL_API std::future<IOInfo> getInfo() override;
            TL_API std::future<VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const IOOptions& = IOOptions()) override;
            TL_API std::future<AudioData> readAudio(
                const OTIO_NS::TimeRange&,
                const IOOptions& = IOOptions()) override;
            TL_API void cancelRequests() override;

        private:
            void _info(const IOOptions&);
            void _thread();
            void _audioThread();

            FTK_PRIVATE();
        };

        //! FFmpeg pipe writer.
        class TL_API_TYPE Write : public IWrite
        {
        protected:
            void _init(
                const ftk::Path&,
                const IOInfo&,
                const IOOptions&,
                const std::shared_ptr<ftk::LogSystem>&);

            Write();

        public:
            TL_API virtual ~Write();

            //! Create a new writer.
            TL_API static std::shared_ptr<Write> create(
                const ftk::Path&,
                const IOInfo&,
                const IOOptions&,
                const std::shared_ptr<ftk::LogSystem>&);

            TL_API void writeVideo(
                const OTIO_NS::RationalTime&,
                const std::shared_ptr<ftk::Image>&,
                const IOOptions& = IOOptions()) override;

        private:
            FTK_PRIVATE();
        };

        //! FFmpeg pipe read plugin.
        class TL_API_TYPE ReadPlugin : public IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

            ReadPlugin() = default;

        public:
            //! Create a new plugin.
            TL_API static std::shared_ptr<ReadPlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            TL_API std::shared_ptr<IRead> read(
                const ftk::Path&,
                const IOOptions& = IOOptions()) override;
            TL_API std::shared_ptr<IRead> read(
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const IOOptions & = IOOptions()) override;
        };

        //! FFmpeg pipe write plugin.
        class TL_API_TYPE WritePlugin : public IWritePlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);

            WritePlugin() = default;

        public:
            //! Create a new write plugin.
            TL_API static std::shared_ptr<WritePlugin> create(
                const std::shared_ptr<ftk::LogSystem>&);

            TL_API ftk::ImageInfo getInfo(
                const ftk::ImageInfo&,
                const IOOptions & = IOOptions()) const override;
            TL_API std::shared_ptr<IWrite> write(
                const ftk::Path&,
                const IOInfo&,
                const IOOptions & = IOOptions()) override;
        };
    }
}
