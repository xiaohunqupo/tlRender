// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace ui
    {
        class ThumbnailGenerator;
    }
    
    namespace timelineui
    {
        //! Video clip item.
        class VideoClipItem : public IBasicItem
        {
        protected:
            void _init(
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ui::ThumbnailGenerator>,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            VideoClipItem();

        public:
            virtual ~VideoClipItem();

            //! Create a new item.
            static std::shared_ptr<VideoClipItem> create(
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ui::ThumbnailGenerator>,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setScale(double) override;
            void setDisplayOptions(const DisplayOptions&) override;

            void tickEvent(
                bool,
                bool,
                const ui::TickEvent&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const dtk::Box2I&, bool) override;
            void drawEvent(const dtk::Box2I&, const ui::DrawEvent&) override;

        private:
            void _drawThumbnails(
                const dtk::Box2I&,
                const ui::DrawEvent&);

            void _cancelRequests();

            TLRENDER_PRIVATE();
        };
    }
}
