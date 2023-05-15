// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace timelineui
    {
        //! Audio clip item.
        class AudioClipItem : public IItem
        {
        protected:
            void _init(
                const otio::Clip*,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            AudioClipItem();

        public:
            ~AudioClipItem() override;

            //! Create a new item.
            static std::shared_ptr<AudioClipItem> create(
                const otio::Clip*,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setScale(float) override;
            void setOptions(const ItemOptions&) override;

            void tickEvent(
                bool,
                bool,
                const ui::TickEvent&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ui::ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const ui::DrawEvent&) override;

        private:
            void _textUpdate();

            void _drawInfo(
                const math::BBox2i&,
                const ui::DrawEvent&);
            void _drawWaveforms(
                const math::BBox2i&,
                const ui::DrawEvent&);

            TLRENDER_PRIVATE();
        };
    }
}
