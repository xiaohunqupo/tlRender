// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UI/TimeUnitsWidget.h>

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
            class TimeUnitsPopup : public ftk::IWidgetPopup
            {
                FTK_NON_COPYABLE(TimeUnitsPopup);

            protected:
                void _init(
                    const std::shared_ptr<ftk::Context>&,
                    const std::shared_ptr<TimeUnitsModel>&,
                    const std::shared_ptr<IWidget>& parent);

                TimeUnitsPopup() = default;

            public:
                virtual ~TimeUnitsPopup() = default;

                static std::shared_ptr<TimeUnitsPopup> create(
                    const std::shared_ptr<ftk::Context>&,
                    const std::shared_ptr<TimeUnitsModel>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            private:
                std::shared_ptr<ftk::ButtonGroup> _buttonGroup;
                std::vector<std::shared_ptr<ftk::ToolButton> > _buttons;
                std::shared_ptr<ftk::Observer<TimeUnits> > _timeUnitsObserver;
            };

            void TimeUnitsPopup::_init(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<TimeUnitsModel>& model,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidgetPopup::_init(context, "tl::ui::TimeUnitsPopup", parent);

                _buttonGroup = ftk::ButtonGroup::create(context, ftk::ButtonGroupType::Radio);

                auto layout = ftk::VerticalLayout::create(context);
                layout->setSpacingRole(ftk::SizeRole::None);
                setWidget(layout);

                for (const auto& label : getTimeUnitsLabels())
                {
                    auto button = ftk::ToolButton::create(context, label, layout);
                    button->setCheckable(true);
                    _buttonGroup->addButton(button);
                    _buttons.push_back(button);
                }

                _buttonGroup->setCheckedCallback(
                    [this, model](int index, bool value)
                    {
                        model->setTimeUnits(static_cast<TimeUnits>(index));
                        close();
                    });

                _timeUnitsObserver = ftk::Observer<TimeUnits>::create(
                    model->observeTimeUnits(),
                    [this](TimeUnits value)
                    {
                        _buttonGroup->setChecked(static_cast<int>(value));
                    });
            }

            std::shared_ptr<TimeUnitsPopup> TimeUnitsPopup::create(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<TimeUnitsModel>& model,
                const std::shared_ptr<IWidget>& parent)
            {
                std::shared_ptr<TimeUnitsPopup> out(new TimeUnitsPopup);
                out->_init(context, model, parent);
                return out;
            }
        }

        struct TimeUnitsWidget::Private
        {
            std::shared_ptr<TimeUnitsModel> model;

            std::shared_ptr<ftk::ToolButton> button;
            std::shared_ptr<TimeUnitsPopup> popup;
        };

        void TimeUnitsWidget::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<TimeUnitsModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::ui::TimeUnitsWidget", parent);
            FTK_P();

            p.model = model;

            p.button = ftk::ToolButton::create(context, shared_from_this());
            p.button->setText("TU");

            p.button->setPressedCallback(
                [this]
                {
                    _showPopup();
                });
        }

        TimeUnitsWidget::TimeUnitsWidget() :
            _p(new Private)
        {}

        TimeUnitsWidget::~TimeUnitsWidget()
        {}

        std::shared_ptr<TimeUnitsWidget> TimeUnitsWidget::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<TimeUnitsModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimeUnitsWidget>(new TimeUnitsWidget);
            out->_init(context, model, parent);
            return out;
        }

        ftk::Size2I TimeUnitsWidget::getSizeHint() const
        {
            return _p->button->getSizeHint();
        }

        void TimeUnitsWidget::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->button->setGeometry(value);
        }

        void TimeUnitsWidget::_showPopup()
        {
            FTK_P();
            auto context = getContext();
            auto window = getWindow();
            if (context && window)
            {
                if (!p.popup)
                {
                    p.popup = TimeUnitsPopup::create(context, p.model);
                    p.popup->open(window, p.button->getGeometry());
                    std::weak_ptr<TimeUnitsWidget> weak(std::dynamic_pointer_cast<TimeUnitsWidget>(shared_from_this()));
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
