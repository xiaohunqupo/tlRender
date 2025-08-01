// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineWidget.h>

#include <tlQtWidget/Util.h>

#include <tlTimelineUI/TimelineWidget.h>
#include <tlTimelineUI/TimelineWidget.h>

#include <tlTimeline/Edit.h>

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>

namespace tl
{
    namespace qtwidget
    {
        struct TimelineWidget::Private
        {
            int currentTrack = -1;

            std::shared_ptr<timelineui::TimelineWidget> timelineWidget;
            QAction* trackEnabledAction = nullptr;

            std::shared_ptr<feather_tk::ValueObserver<bool> > editableObserver;
            std::shared_ptr<feather_tk::ValueObserver<bool> > frameViewObserver;
            std::shared_ptr<feather_tk::ValueObserver<bool> > scrubObserver;
            std::shared_ptr<feather_tk::ValueObserver<OTIO_NS::RationalTime> > timeScrubObserver;
        };

        TimelineWidget::TimelineWidget(
            const std::shared_ptr<feather_tk::Context>& context,
            const std::shared_ptr<timeline::ITimeUnitsModel>& timeUnitsModel,
            const std::shared_ptr<feather_tk::Style>& style,
            QWidget* parent) :
            ContainerWidget(context, style, parent),
            _p(new Private)
        {
            FEATHER_TK_P();

            p.timelineWidget = timelineui::TimelineWidget::create(context, timeUnitsModel);
            //p.timelineWidget->setScrollBarsVisible(false);
            setWidget(p.timelineWidget);

            p.trackEnabledAction = new QAction(this);
            p.trackEnabledAction->setText("Track enabled");
            p.trackEnabledAction->setCheckable(true);

            connect(
                p.trackEnabledAction,
                SIGNAL(toggled(bool)),
                SLOT(_trackEnabledCallback(bool)));

            p.editableObserver = feather_tk::ValueObserver<bool>::create(
                p.timelineWidget->observeEditable(),
                [this](bool value)
                {
                    Q_EMIT editableChanged(value);
                });

            p.frameViewObserver = feather_tk::ValueObserver<bool>::create(
                p.timelineWidget->observeFrameView(),
                [this](bool value)
                {
                    Q_EMIT frameViewChanged(value);
                });

            p.scrubObserver = feather_tk::ValueObserver<bool>::create(
                p.timelineWidget->observeScrub(),
                [this](bool value)
                {
                    Q_EMIT scrubChanged(value);
                });

            p.timeScrubObserver = feather_tk::ValueObserver<OTIO_NS::RationalTime>::create(
                p.timelineWidget->observeTimeScrub(),
                [this](const OTIO_NS::RationalTime& value)
                {
                    Q_EMIT timeScrubbed(value);
                });
        }

        TimelineWidget::~TimelineWidget()
        {}

        std::shared_ptr<timeline::Player>& TimelineWidget::player() const
        {
            return _p->timelineWidget->getPlayer();
        }

        void TimelineWidget::setPlayer(const std::shared_ptr<timeline::Player>& player)
        {
            _p->timelineWidget->setPlayer(player);
        }

        bool TimelineWidget::isEditable() const
        {
            return _p->timelineWidget->isEditable();
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->timelineWidget->hasFrameView();
        }

        bool TimelineWidget::areScrollBarsVisible() const
        {
            return _p->timelineWidget->areScrollBarsVisible();
        }

        bool TimelineWidget::hasAutoScroll() const
        {
            return _p->timelineWidget->hasAutoScroll();
        }

        bool TimelineWidget::hasStopOnScrub() const
        {
            return _p->timelineWidget->hasStopOnScrub();
        }

        const std::vector<int>& TimelineWidget::frameMarkers() const
        {
            return _p->timelineWidget->getFrameMarkers();
        }

        const timelineui::ItemOptions& TimelineWidget::itemOptions() const
        {
            return _p->timelineWidget->getItemOptions();
        }

        const timelineui::DisplayOptions& TimelineWidget::displayOptions() const
        {
            return _p->timelineWidget->getDisplayOptions();
        }

        void TimelineWidget::setEditable(bool value)
        {
            _p->timelineWidget->setEditable(value);
        }

        void TimelineWidget::setFrameView(bool value)
        {
            _p->timelineWidget->setFrameView(value);
        }

        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->timelineWidget->setScrollBarsVisible(value);
        }

        void TimelineWidget::setAutoScroll(bool value)
        {
            _p->timelineWidget->setAutoScroll(value);
        }

        void TimelineWidget::setScrollBinding(int button, feather_tk::KeyModifier modifier)
        {
            _p->timelineWidget->setScrollBinding(button, modifier);
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            _p->timelineWidget->setMouseWheelScale(value);
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            _p->timelineWidget->setStopOnScrub(value);
        }

        void TimelineWidget::setFrameMarkers(const std::vector<int>& value)
        {
            _p->timelineWidget->setFrameMarkers(value);
        }

        void TimelineWidget::setItemOptions(const timelineui::ItemOptions& value)
        {
            FEATHER_TK_P();
            p.timelineWidget->setItemOptions(value);
            setInputEnabled(value.inputEnabled);
        }

        void TimelineWidget::setDisplayOptions(const timelineui::DisplayOptions& value)
        {
            _p->timelineWidget->setDisplayOptions(value);
        }

        void TimelineWidget::contextMenuEvent(QContextMenuEvent* event)
        {
            FEATHER_TK_P();
            if (auto player = p.timelineWidget->getPlayer())
            {
                const feather_tk::V2I pos = _toUI(feather_tk::V2I(event->x(), event->y()));
                const std::vector<feather_tk::Box2I> trackGeom = p.timelineWidget->getTrackGeom();
                for (int i = 0; i < trackGeom.size(); ++i)
                {
                    if (feather_tk::contains(trackGeom[i], pos))
                    {
                        p.currentTrack = i;

                        bool checked = false;
                        auto otioTimeline = player->getTimeline()->getTimeline();
                        const auto& children = otioTimeline->tracks()->children();
                        if (i >= 0 && i < children.size())
                        {
                            if (auto track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(children[i]))
                            {
                                checked = track->enabled();
                            }
                        }
                        p.trackEnabledAction->setChecked(checked);

                        QMenu menu(this);
                        menu.addAction(p.trackEnabledAction);
                        menu.exec(event->globalPos());
                        break;
                    }
                }
            }
        }

        void TimelineWidget::_trackEnabledCallback(bool value)
        {
            FEATHER_TK_P();
            if (auto player = p.timelineWidget->getPlayer())
            {
                auto otioTimeline = player->getTimeline()->getTimeline();
                auto children = otioTimeline->tracks()->children();
                if (p.currentTrack >= 0 && p.currentTrack < children.size())
                {
                    if (auto track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(children[p.currentTrack]))
                    {
                        if (value != track->enabled())
                        {
                            auto otioTimelineNew = timeline::copy(player->getTimeline()->getTimeline().value);
                            children = otioTimelineNew->tracks()->children();
                            track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(children[p.currentTrack]);
                            track->set_enabled(value);
                            player->getTimeline()->setTimeline(otioTimelineNew);
                        }
                    }
                }
            }
        }
    }
}
