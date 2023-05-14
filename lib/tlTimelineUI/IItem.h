// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IOManager.h>

#include <tlUI/IWidget.h>

namespace tl
{
    namespace timelineui
    {
        //! Time units.
        enum class TimeUnits
        {
            Seconds,
            Frames,
            Timecode,

            Count,
            First = Seconds
        };
        TLRENDER_ENUM(TimeUnits);
        TLRENDER_ENUM_SERIALIZE(TimeUnits);

        //! Item data.
        struct ItemData
        {
            std::string directory;
            file::PathOptions pathOptions;
            std::shared_ptr<IOManager> ioManager;
        };

        //! Item options.
        struct ItemOptions
        {
            TimeUnits timeUnits = TimeUnits::Timecode;
            float scale = 500.F;
            float clipRectScale = 2.F;
            bool thumbnails = true;
            int thumbnailHeight = 100;
            int waveformHeight = 50;
            float thumbnailFade = .5F;

            bool operator == (const ItemOptions&) const;
            bool operator != (const ItemOptions&) const;
        };

        //! Base class for items.
        class IItem : public ui::IWidget
        {
        protected:
            void _init(
                const std::string& name,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IItem();

        public:
            ~IItem() override;

            virtual void setOptions(const ItemOptions&);

        protected:
            static math::BBox2i _getClipRect(
                const math::BBox2i&,
                float scale);

            static std::string _durationLabel(
                const otime::RationalTime&,
                TimeUnits);
            static std::string _timeLabel(
                const otime::RationalTime&,
                TimeUnits);

            ItemData _data;
            ItemOptions _options;
        };
    }
}