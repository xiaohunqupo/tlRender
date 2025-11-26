// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/Timeline/Player.h>

#include <ftk/UI/IWidget.h>

namespace tl
{
    namespace timelineui
    {
        //! Playback buttons.
        class PlaybackButtons : public ftk::IWidget
        {
            FTK_NON_COPYABLE(PlaybackButtons);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            PlaybackButtons();

        public:
            virtual ~PlaybackButtons();

            //! Create a new widget.
            static std::shared_ptr<PlaybackButtons> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the playback.
            void setPlayback(timeline::Playback);

            //! Set the callback.
            void setCallback(const std::function<void(timeline::Playback)>&);

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            void _widgetUpdate();

            FTK_PRIVATE();
        };
    }
}
