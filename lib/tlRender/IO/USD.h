// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/IO/Read.h>

namespace tl
{
    //! USD image I/O.
    namespace usd
    {
        class Render;

        //! USD draw modes.
        enum class TL_API_TYPE DrawMode
        {
            Points,
            Wireframe,
            WireframeOnSurface,
            ShadedFlat,
            ShadedSmooth,
            GeomOnly,
            GeomFlat,
            GeomSmooth,

            Count,
            First = Points
        };
        TL_ENUM(DrawMode);

        //! USD options.
        struct TL_API_TYPE Options
        {
            int renderWidth = 1920;
            float complexity = 1.F;
            usd::DrawMode drawMode = usd::DrawMode::ShadedSmooth;
            bool enableLighting = true;
            bool sRGB = true;
            size_t stageCache = 10;
            size_t diskCache = 0;

            TL_API bool operator == (const Options&) const;
            TL_API bool operator != (const Options&) const;
        };

        //! Get USD options.
        TL_API io::Options getOptions(const Options&);
        
        //! USD reader.
        class TL_API_TYPE Read : public io::IRead
        {
        protected:
            void _init(
                int64_t id,
                const std::shared_ptr<Render>&,
                const ftk::Path&,
                const std::vector<ftk::MemFile>&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            Read();

        public:
            TL_API ~Read() override;

            //! Create a new reader.
            TL_API static std::shared_ptr<Read> create(
                int64_t id,
                const std::shared_ptr<Render>&,
                const ftk::Path&,
                const io::Options&,
                const std::shared_ptr<ftk::LogSystem>&);

            TL_API std::future<io::Info> getInfo() override;
            TL_API std::future<io::VideoData> readVideo(
                const OTIO_NS::RationalTime&,
                const io::Options&) override;
            TL_API void cancelRequests() override;

        private:
            FTK_PRIVATE();
        };

        //! USD read plugin.
        class TL_API_TYPE ReadPlugin : public io::IReadPlugin
        {
        protected:
            void _init(const std::shared_ptr<ftk::LogSystem>&);
            
            ReadPlugin();

        public:
            TL_API virtual ~ReadPlugin();

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

