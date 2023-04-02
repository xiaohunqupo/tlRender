// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <opentimelineio/clip.h>
#include <opentimelineio/track.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
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

            public:
                static std::shared_ptr<AudioClipItem> create(
                    const otio::Clip*,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~AudioClipItem() override;

                void setScale(float) override;
                void setThumbnailHeight(int) override;
                void setViewport(const math::BBox2i&) override;

                void tickEvent(const ui::TickEvent&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;

            private:
                void _cancelAudioRequests();

                const otio::Clip* _clip = nullptr;
                const otio::Track* _track = nullptr;
                file::Path _path;
                std::vector<file::MemoryRead> _memoryRead;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::string _label;
                std::string _durationLabel;
                imaging::FontInfo _fontInfo;
                int _margin = 0;
                imaging::FontMetrics _fontMetrics;
            };
        }
    }
}