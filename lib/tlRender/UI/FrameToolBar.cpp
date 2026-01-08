// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UI/FrameToolBar.h>

#include <tlRender/Timeline/Player.h>

#include <ftk/UI/Action.h>
#include <ftk/UI/ToolButton.h>

namespace tl
{
    namespace ui
    {
        struct FrameToolBar::Private
        {
            std::map<std::string, std::shared_ptr<ftk::Action> > actions;
            std::shared_ptr<Player> player;
        };

        void FrameToolBar::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, ftk::Orientation::Horizontal, parent);
            FTK_P();

            p.actions["Start"] = ftk::Action::create(
                "Start Frame",
                "FrameStart",
                ftk::Key::Home,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->gotoStart();
                    }
                });
            p.actions["Start"]->setTooltip("Go to the start frame.");

            p.actions["Prev"] = ftk::Action::create(
                "Previous Frame",
                "FramePrev",
                ftk::Key::Left,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->framePrev();
                    }
                });
            p.actions["Prev"]->setTooltip("Go to the previous frame.");

            p.actions["Next"] = ftk::Action::create(
                "Next Frame",
                "FrameNext",
                ftk::Key::Right,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->frameNext();
                    }
                });
            p.actions["Next"]->setTooltip("Go to the next frame.");

            p.actions["End"] = ftk::Action::create(
                "End Frame",
                "FrameEnd",
                ftk::Key::End,
                [this]
                {
                    if (_p->player)
                    {
                        _p->player->gotoEnd();
                    }
                });
            p.actions["End"]->setTooltip("Go to the end frame.");

            addAction(p.actions["Start"]);
            auto button = addAction(p.actions["Prev"]);
            button->setRepeatClick(true);
            button = addAction(p.actions["Next"]);
            button->setRepeatClick(true);
            addAction(p.actions["End"]);

            _widgetUpdate();
        }

        FrameToolBar::FrameToolBar() :
            _p(new Private)
        {}

        FrameToolBar::~FrameToolBar()
        {}

        std::shared_ptr<FrameToolBar> FrameToolBar::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FrameToolBar>(new FrameToolBar);
            out->_init(context, parent);
            return out;
        }
            
        const std::map<std::string, std::shared_ptr<ftk::Action> >& FrameToolBar::getActions() const
        {
            return _p->actions;
        }

        const std::shared_ptr<Player>& FrameToolBar::getPlayer() const
        {
            return _p->player;
        }

        void FrameToolBar::setPlayer(const std::shared_ptr<Player>& player)
        {
            _p->player = player;
            _widgetUpdate();
        }

        void FrameToolBar::_widgetUpdate()
        {
            FTK_P();
            p.actions["Start"]->setEnabled(p.player.get());
            p.actions["Prev"]->setEnabled(p.player.get());
            p.actions["Next"]->setEnabled(p.player.get());
            p.actions["End"]->setEnabled(p.player.get());
        }
    }
}
