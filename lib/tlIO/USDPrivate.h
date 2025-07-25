// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/USD.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

namespace tl
{
    namespace usd
    {
        //! USD renderer.
        class Render : public std::enable_shared_from_this<Render>
        {
        protected:
            void _init(const std::shared_ptr<feather_tk::LogSystem>&);

            Render();

        public:
            ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                const std::shared_ptr<feather_tk::LogSystem>&);
            
            //! Get information.
            std::future<io::Info> getInfo(
                int64_t id,
                const file::Path& path,
                const io::Options&);
            
            //! Render an image.
            std::future<io::VideoData> render(
                int64_t id,
                const file::Path& path,
                const OTIO_NS::RationalTime& time,
                const io::Options&);

            //! Cancel requests.
            void cancelRequests(int64_t id);

        private:
            void _open(
                const std::string&,
                PXR_NS::UsdStageRefPtr&,
                std::shared_ptr<PXR_NS::UsdImagingGLEngine>&);
            void _run();
            void _finish();

            FEATHER_TK_PRIVATE();
        };
    }
}

