// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include "PlaybackBar.h"

#include "App.h"
#include "FilesModel.h"

#include <ftk/UI/ToolButton.h>
#include <ftk/Core/Format.h>

namespace tl
{
    namespace play
    {
        void PlaybackBar::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "PlaybackBar", parent);

            _layout = ftk::HorizontalLayout::create(context, shared_from_this());
            _layout->setMarginRole(ftk::SizeRole::MarginInside);

            auto hLayout = ftk::HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);
            auto tmp = actions;
            auto reverseButton = ftk::ToolButton::create(context, tmp["Reverse"], hLayout);
            auto stopButton = ftk::ToolButton::create(context, tmp["Stop"], hLayout);
            auto forwardButton = ftk::ToolButton::create(context, tmp["Forward"], hLayout);

            hLayout = ftk::HorizontalLayout::create(context, _layout);
            hLayout->setSpacingRole(ftk::SizeRole::SpacingTool);
            auto startButton = ftk::ToolButton::create(context, tmp["Start"], hLayout);
            auto prevButton = ftk::ToolButton::create(context, tmp["Prev"], hLayout);
            prevButton->setRepeatClick(true);
            auto nextButton = ftk::ToolButton::create(context, tmp["Next"], hLayout);
            nextButton->setRepeatClick(true);
            auto endButton = ftk::ToolButton::create(context, tmp["End"], hLayout);

            _currentTimeEdit = ui::TimeEdit::create(context, app->getTimeUnitsModel(), _layout);
            _currentTimeEdit->setTooltip("The current time.");

            _durationLabel = ui::TimeLabel::create(context, app->getTimeUnitsModel(), _layout);
            _durationLabel->setTooltip("The timeline duration.");

            _speedEdit = ftk::DoubleEdit::create(context, _layout);
            _speedEdit->setRange(ftk::RangeD(1.0, 99999.0));
            _speedEdit->setStep(1.0);
            _speedEdit->setLargeStep(10.0);
            _speedEdit->setTooltip("The timeline speed.");

            _speedMultLabel = ftk::Label::create(context, _layout);
            _speedMultLabel->setHMarginRole(ftk::SizeRole::MarginInside);
            _speedMultLabel->setTooltip("Playback speed multiplier.");

            _timeUnitsComboBox = ftk::ComboBox::create(
                context,
                getTimeUnitsLabels(),
                _layout);

            _currentTimeEdit->setCallback(
                [this](const OTIO_NS::RationalTime& value)
                {
                    if (_player)
                    {
                        _player->stop();
                        _player->seek(value);
                    }
                });

            _speedEdit->setCallback(
                [this](double value)
                {
                    if (_player)
                    {
                        _player->setSpeed(value);
                    }
                });

            std::weak_ptr<App> appWeak(app);
            _timeUnitsComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getTimeUnitsModel()->setTimeUnits(
                            static_cast<TimeUnits>(value));
                    }
                });

            _playerObserver = ftk::Observer<std::shared_ptr<Player> >::create(
                app->getFilesModel()->observePlayer(),
                [this](const std::shared_ptr<Player>& value)
                {
                    _player = value;

                    if (value)
                    {
                        _durationLabel->setValue(value->getTimeRange().duration());

                        _currentTimeObserver = ftk::Observer<OTIO_NS::RationalTime>::create(
                            value->observeCurrentTime(),
                            [this](const OTIO_NS::RationalTime& value)
                            {
                                _currentTimeEdit->setValue(value);
                            });

                        _speedObserver = ftk::Observer<double>::create(
                            value->observeSpeed(),
                            [this](double value)
                            {
                                _speedEdit->setValue(value);
                            });

                        _speedMultObserver = ftk::Observer<double>::create(
                            value->observeSpeedMult(),
                            [this](double value)
                            {
                                _speedMultLabel->setText(ftk::Format("{0}X").arg(value, 1));
                                _speedMultLabel->setBackgroundRole(value > 1.0 ?
                                    ftk::ColorRole::Checked :
                                    ftk::ColorRole::None);
                            });
                    }
                    else
                    {
                        _currentTimeEdit->setValue(invalidTime);
                        _durationLabel->setValue(invalidTime);
                        _speedMultLabel->setText("1X");
                        _speedMultLabel->setBackgroundRole(ftk::ColorRole::None);

                        _currentTimeObserver.reset();
                        _speedObserver.reset();
                        _speedMultObserver.reset();
                    }

                    _currentTimeEdit->setEnabled(value.get());
                    _durationLabel->setEnabled(value.get());
                    _speedEdit->setEnabled(value.get());
                    _speedMultLabel->setEnabled(value.get());
                });

            _timeUnitsObserver = ftk::Observer<TimeUnits>::create(
                app->getTimeUnitsModel()->observeTimeUnits(),
                [this](TimeUnits value)
                {
                    _timeUnitsComboBox->setCurrentIndex(static_cast<int>(value));
                });
        }

        PlaybackBar::~PlaybackBar()
        {}

        std::shared_ptr<PlaybackBar> PlaybackBar::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<ftk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackBar>(new PlaybackBar);
            out->_init(context, app, actions, parent);
            return out;
        }

        ftk::Size2I PlaybackBar::getSizeHint() const
        {
            return _layout->getSizeHint();
        }

        void PlaybackBar::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }
    }
}