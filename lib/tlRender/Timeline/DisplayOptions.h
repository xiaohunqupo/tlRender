// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Core/Export.h>

#include <ftk/Core/Image.h>
#include <ftk/Core/Matrix.h>
#include <ftk/Core/RenderOptions.h>

namespace tl
{
    namespace timeline
    {
        //! Color values.
        struct TL_API_TYPE Color
        {
        public:
            bool     enabled    = false;
            ftk::V3F add        = ftk::V3F(0.F, 0.F, 0.F);
            ftk::V3F brightness = ftk::V3F(1.F, 1.F, 1.F);
            ftk::V3F contrast   = ftk::V3F(1.F, 1.F, 1.F);
            ftk::V3F saturation = ftk::V3F(1.F, 1.F, 1.F);
            float    tint       = 0.F;
            bool     invert     = false;

            TL_API bool operator == (const Color&) const;
            TL_API bool operator != (const Color&) const;
        };

        //! Get a brightness color matrix.
        TL_API ftk::M44F brightness(const ftk::V3F&);

        //! Get a contrast color matrix.
        TL_API ftk::M44F contrast(const ftk::V3F&);

        //! Get a saturation color matrix.
        TL_API ftk::M44F saturation(const ftk::V3F&);

        //! Get a tint color matrix.
        TL_API ftk::M44F tint(float);

        //! Get a color matrix.
        TL_API ftk::M44F color(const Color&);

        //! Levels values.
        struct TL_API_TYPE Levels
        {
            bool  enabled = false;
            float inLow   = 0.F;
            float inHigh  = 1.F;
            float gamma   = 1.F;
            float outLow  = 0.F;
            float outHigh = 1.F;

            TL_API bool operator == (const Levels&) const;
            TL_API bool operator != (const Levels&) const;
        };

        //! These values match the ones in exrdisplay for comparison and
        //! testing.
        struct TL_API_TYPE EXRDisplay
        {
            bool  enabled  = false;
            float exposure = 0.F;
            float defog    = 0.F;
            float kneeLow  = 0.F;
            float kneeHigh = 5.F;

            TL_API bool operator == (const EXRDisplay&) const;
            TL_API bool operator != (const EXRDisplay&) const;
        };

        //! Soft clip.
        struct TL_API_TYPE SoftClip
        {
            bool  enabled = false;
            float value   = 0.F;

            TL_API bool operator == (const SoftClip&) const;
            TL_API bool operator != (const SoftClip&) const;
        };

        //! Display options.
        struct TL_API_TYPE DisplayOptions
        {
            ftk::ChannelDisplay channels     = ftk::ChannelDisplay::Color;
            ftk::ImageMirror    mirror;
            Color               color;
            Levels              levels;
            EXRDisplay          exrDisplay;
            SoftClip            softClip;
            ftk::ImageFilters   imageFilters;
            ftk::VideoLevels    videoLevels  = ftk::VideoLevels::FullRange;

            TL_API bool operator == (const DisplayOptions&) const;
            TL_API bool operator != (const DisplayOptions&) const;
        };

        //! \name Serialize
        ///@{

        TL_API void to_json(nlohmann::json&, const Color&);
        TL_API void to_json(nlohmann::json&, const Levels&);
        TL_API void to_json(nlohmann::json&, const EXRDisplay&);
        TL_API void to_json(nlohmann::json&, const SoftClip&);
        TL_API void to_json(nlohmann::json&, const DisplayOptions&);

        TL_API void from_json(const nlohmann::json&, Color&);
        TL_API void from_json(const nlohmann::json&, Levels&);
        TL_API void from_json(const nlohmann::json&, EXRDisplay&);
        TL_API void from_json(const nlohmann::json&, SoftClip&);
        TL_API void from_json(const nlohmann::json&, DisplayOptions&);

        ///@}
    }
}
