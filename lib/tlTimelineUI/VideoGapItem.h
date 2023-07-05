// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/gap.h>

namespace tl
{
    namespace timelineui
    {
        //! Video gap item.
        class VideoGapItem : public IBasicItem
        {
        protected:
            void _init(
                const otio::Gap*,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            VideoGapItem();

        public:
            ~VideoGapItem() override;

            //! Create a new item.
            static std::shared_ptr<VideoGapItem> create(
                const otio::Gap*,
                const ItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);
        };
    }
}