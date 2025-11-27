// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UI/PlaybackButtons.h>

#include <ftk/UI/ButtonGroup.h>
#include <ftk/UI/RowLayout.h>
#include <ftk/UI/ToolButton.h>

namespace tl
{
    namespace timelineui
    {
        struct PlaybackButtons::Private
        {
            std::map<timeline::Playback, std::shared_ptr<ftk::ToolButton> > buttons;
            std::function<void(timeline::Playback)> callback;
            std::shared_ptr<ftk::ButtonGroup> buttonGroup;
            std::shared_ptr<ftk::HorizontalLayout> layout;
        };

        void PlaybackButtons::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::timelineui::PlaybackButtons", parent);
            FTK_P();

            p.buttonGroup = ftk::ButtonGroup::create(context, ftk::ButtonGroupType::Radio);
            std::map<timeline::Playback, std::string> tooltips =
            {
                { timeline::Playback::Stop, "Stop playback" },
                { timeline::Playback::Forward, "Start forward playback" },
                { timeline::Playback::Reverse, "Start reverse playback" },
            };
            for (auto playback : timeline::getPlaybackEnums())
            {
                p.buttons[playback] = ftk::ToolButton::create(context);
                p.buttons[playback]->setIcon("Playback" + getLabel(playback));
                p.buttons[playback]->setTooltip(tooltips[playback]);
                p.buttonGroup->addButton(p.buttons[playback]);
            }
            p.buttons[timeline::Playback::Stop]->setChecked(true);

            p.layout = ftk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ftk::SizeRole::SpacingTool);
            p.buttons[timeline::Playback::Reverse]->setParent(p.layout);
            p.buttons[timeline::Playback::Stop]->setParent(p.layout);
            p.buttons[timeline::Playback::Forward]->setParent(p.layout);

            p.buttonGroup->setCheckedCallback(
                [this](int index, bool value)
                {
                    FTK_P();
                    if (value)
                    {
                        if (p.callback)
                        {
                            p.callback(static_cast<timeline::Playback>(index));
                        }
                    }
                });
        }

        PlaybackButtons::PlaybackButtons() :
            _p(new Private)
        {}

        PlaybackButtons::~PlaybackButtons()
        {}

        std::shared_ptr<PlaybackButtons> PlaybackButtons::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackButtons>(new PlaybackButtons);
            out->_init(context, parent);
            return out;
        }

        timeline::Playback PlaybackButtons::getPlayback() const
        {
            FTK_P();
            timeline::Playback out = timeline::Playback::Stop;
            for (auto i : p.buttons)
            {
                if (i.second->isChecked())
                {
                    out = i.first;
                    break;
                }
            }
            return out;
        }

        void PlaybackButtons::setPlayback(timeline::Playback value)
        {
            _p->buttons[value]->setChecked(true);
        }

        void PlaybackButtons::setCallback(const std::function<void(timeline::Playback)>& value)
        {
            _p->callback = value;
        }

        void PlaybackButtons::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void PlaybackButtons::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }
    }
}
