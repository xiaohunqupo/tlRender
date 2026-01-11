// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UI/PlaybackLoopWidget.h>

#include <ftk/UI/ButtonGroup.h>
#include <ftk/UI/IWidgetPopup.h>
#include <ftk/UI/RowLayout.h>
#include <ftk/UI/ToolButton.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            class PlaybackLoopPopup : public ftk::IWidgetPopup
            {
                FTK_NON_COPYABLE(PlaybackLoopPopup);

            protected:
                void _init(
                    const std::shared_ptr<ftk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                PlaybackLoopPopup() = default;

            public:
                virtual ~PlaybackLoopPopup() = default;

                static std::shared_ptr<PlaybackLoopPopup> create(
                    const std::shared_ptr<ftk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setLoop(Loop);
                void setCallback(const std::function<void(Loop)>&);

            private:
                std::shared_ptr<ftk::ButtonGroup> _buttonGroup;
                std::vector<std::shared_ptr<ftk::ToolButton> > _buttons;
                std::function<void(Loop)> _callback;
            };

            void PlaybackLoopPopup::_init(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidgetPopup::_init(context, "tl::ui::PlaybackLoopPopup", parent);

                _buttonGroup = ftk::ButtonGroup::create(context, ftk::ButtonGroupType::Radio);

                auto layout = ftk::VerticalLayout::create(context);
                layout->setSpacingRole(ftk::SizeRole::None);
                setWidget(layout);

                const std::vector<std::string> icons =
                {
                    "PlaybackLoop",
                    "PlaybackOnce",
                    "PlaybackPingPong"
                };
                const std::vector<std::string> tooltips =
                {
                    "Loop playback continuously.",
                    "Playback onece and stop.",
                    "Playback forwards and reverse."
                };
                for (size_t i = 0; i < icons.size(); ++i)
                {
                    auto button = ftk::ToolButton::create(context, layout);
                    button->setCheckable(true);
                    button->setIcon(icons[i]);
                    button->setTooltip(tooltips[i]);
                    _buttonGroup->addButton(button);
                    _buttons.push_back(button);
                }

                _buttonGroup->setCheckedCallback(
                    [this](int index, bool value)
                    {
                        if (value && _callback)
                        {
                            _callback(static_cast<Loop>(index));
                        }
                        close();
                    });
            }

            std::shared_ptr<PlaybackLoopPopup> PlaybackLoopPopup::create(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                std::shared_ptr<PlaybackLoopPopup> out(new PlaybackLoopPopup);
                out->_init(context, parent);
                return out;
            }

            void PlaybackLoopPopup::setLoop(Loop value)
            {
                _buttonGroup->setChecked(static_cast<int>(value));
            }

            void PlaybackLoopPopup::setCallback(const std::function<void(Loop)>& value)
            {
                _callback = value;
            }
        }

        struct PlaybackLoopWidget::Private
        {
            Loop loop = Loop::Loop;

            std::shared_ptr<ftk::ToolButton> button;
            std::shared_ptr<PlaybackLoopPopup> popup;

            std::function<void(Loop)> callback;
        };

        void PlaybackLoopWidget::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::ui::PlaybackLoopWidget", parent);
            FTK_P();

            p.button = ftk::ToolButton::create(context, shared_from_this());

            _widgetUpdate();

            p.button->setPressedCallback(
                [this]
                {
                    _showPopup();
                });
        }

        PlaybackLoopWidget::PlaybackLoopWidget() :
            _p(new Private)
        {}

        PlaybackLoopWidget::~PlaybackLoopWidget()
        {}

        std::shared_ptr<PlaybackLoopWidget> PlaybackLoopWidget::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PlaybackLoopWidget>(new PlaybackLoopWidget);
            out->_init(context, parent);
            return out;
        }
        
        Loop PlaybackLoopWidget::getLoop() const
        {
            return _p->loop;
        }

        void PlaybackLoopWidget::setLoop(Loop value)
        {
            FTK_P();
            if (value == p.loop)
                return;
            p.loop = value;
            _widgetUpdate();
        }

        void PlaybackLoopWidget::setCallback(const std::function<void(Loop)>& value)
        {
            _p->callback = value;
        }

        ftk::Size2I PlaybackLoopWidget::getSizeHint() const
        {
            return _p->button->getSizeHint();
        }

        void PlaybackLoopWidget::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->button->setGeometry(value);
        }

        void PlaybackLoopWidget::_widgetUpdate()
        {
            FTK_P();
            std::string icon;
            switch (p.loop)
            {
            case Loop::Loop: icon = "PlaybackLoop"; break;
            case Loop::Once: icon = "PlaybackOnce"; break;
            case Loop::PingPong: icon = "PlaybackPingPong"; break;
            default: break;
            }
            p.button->setIcon(icon);
        }

        void PlaybackLoopWidget::_showPopup()
        {
            FTK_P();
            auto context = getContext();
            auto window = getWindow();
            if (context && window)
            {
                if (!p.popup)
                {
                    p.popup = PlaybackLoopPopup::create(context);
                    p.popup->setLoop(p.loop);
                    p.popup->open(window, p.button->getGeometry());
                    std::weak_ptr<PlaybackLoopWidget> weak(std::dynamic_pointer_cast<PlaybackLoopWidget>(shared_from_this()));
                    p.popup->setCallback(
                        [weak](Loop value)
                        {
                            if (auto widget = weak.lock())
                            {
                                widget->_p->loop = value;
                                widget->_widgetUpdate();
                                if (widget->_p->callback)
                                {
                                    widget->_p->callback(value);
                                }
                            }
                        });
                    p.popup->setCloseCallback(
                        [weak]
                        {
                            if (auto widget = weak.lock())
                            {
                                widget->_p->popup.reset();
                            }
                        });
                }
                else
                {
                    p.popup->close();
                    p.popup.reset();
                }
            }
        }
    }
}
