// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlRender/UI/TimeEdit.h>
#include <tlRender/UI/TimeLabel.h>

#include <tlRender/Timeline/Player.h>
#include <tlRender/Timeline/TimeUnits.h>

#include <ftk/UI/Action.h>
#include <ftk/UI/ComboBox.h>
#include <ftk/UI/DoubleEdit.h>
#include <ftk/UI/RowLayout.h>

namespace tl
{
    namespace play
    {
        class App;

        //! This widget provides playback controls and other time related widgets.
        class PlaybackBar : public ftk::IWidget
        {
            FTK_NON_COPYABLE(PlaybackBar);

        protected:
            void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ftk::Action> >&,
                const std::shared_ptr<IWidget>& parent);

            PlaybackBar() = default;

        public:
            ~PlaybackBar();

            static std::shared_ptr<PlaybackBar> create(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<App>&,
                const std::map<std::string, std::shared_ptr<ftk::Action> >&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const ftk::Box2I&) override;
            void sizeHintEvent(const ftk::SizeHintEvent&) override;

        private:
            std::shared_ptr<timeline::Player> _player;
            std::shared_ptr<ftk::HorizontalLayout> _layout;
            std::shared_ptr<timelineui::TimeEdit> _currentTimeEdit;
            std::shared_ptr<timelineui::TimeLabel> _durationLabel;
            std::shared_ptr<ftk::DoubleEdit> _speedEdit;
            std::shared_ptr<ftk::ComboBox> _timeUnitsComboBox;
            std::shared_ptr<ftk::Observer<std::shared_ptr<timeline::Player> > > _playerObserver;
            std::shared_ptr<ftk::Observer<OTIO_NS::RationalTime> > _currentTimeObserver;
            std::shared_ptr<ftk::Observer<double> > _speedObserver;
            std::shared_ptr<ftk::Observer<timeline::TimeUnits> > _timeUnitsObserver;
        };
    }
}
