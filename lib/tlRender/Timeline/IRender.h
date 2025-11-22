// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/BackgroundOptions.h>
#include <tlRender/Timeline/ColorOptions.h>
#include <tlRender/Timeline/CompareOptions.h>
#include <tlRender/Timeline/DisplayOptions.h>
#include <tlRender/Timeline/ForegroundOptions.h>
#include <tlRender/Timeline/Video.h>

#include <ftk/Core/IRender.h>

namespace tl
{
    namespace timeline
    {
        //! Base class for renderers.
        class IRender : public ftk::IRender
        {
        public:
            virtual ~IRender() = 0;

            //! Set the OpenColorIO options.
            virtual void setOCIOOptions(const OCIOOptions&) = 0;

            //! Set the LUT options.
            virtual void setLUTOptions(const LUTOptions&) = 0;

            //! Draw the background.
            virtual void drawBackground(
                const std::vector<ftk::Box2I>&,
                const ftk::M44F&,
                const BackgroundOptions&) = 0;

            //! Draw timeline video data.
            virtual void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<ftk::Box2I>&,
                const std::vector<ftk::ImageOptions>& = {},
                const std::vector<DisplayOptions>& = {},
                const CompareOptions& = CompareOptions(),
                ftk::ImageType colorBuffer = ftk::ImageType::RGBA_U8) = 0;

            //! Draw the foreground.
            virtual void drawForeground(
                const std::vector<ftk::Box2I>&,
                const ftk::M44F&,
                const ForegroundOptions&) = 0;
        };
    }
}
