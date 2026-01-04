// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UI/TimeUnitsWidget.h>

#include <ftk/UI/ComboBox.h>

namespace tl
{
    namespace ui
    {
        struct TimeUnitsWidget::Private
        {
            std::shared_ptr<timeline::TimeUnitsModel> model;

            std::shared_ptr<ftk::ComboBox> comboBox;

            std::shared_ptr<ftk::Observer<timeline::TimeUnits> > timeUnitsObserver;
        };

        void TimeUnitsWidget::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "tl::ui::TimeUnitsWidget", parent);
            FTK_P();

            p.model = model;

            p.comboBox = ftk::ComboBox::create(context, timeline::getTimeUnitsLabels(), shared_from_this());

            p.comboBox->setIndexCallback(
                [this](int index)
                {
                    _p->model->setTimeUnits(static_cast<timeline::TimeUnits>(index));
                });

            p.timeUnitsObserver = ftk::Observer<timeline::TimeUnits>::create(
                p.model->observeTimeUnits(),
                [this](timeline::TimeUnits value)
                {
                    _p->comboBox->setCurrentIndex(static_cast<int>(value));
                });
        }

        TimeUnitsWidget::TimeUnitsWidget() :
            _p(new Private)
        {}

        TimeUnitsWidget::~TimeUnitsWidget()
        {}

        std::shared_ptr<TimeUnitsWidget> TimeUnitsWidget::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<TimeUnitsWidget>(new TimeUnitsWidget);
            out->_init(context, model, parent);
            return out;
        }

        ftk::Size2I TimeUnitsWidget::getSizeHint() const
        {
            return _p->comboBox->getSizeHint();
        }

        void TimeUnitsWidget::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->comboBox->setGeometry(value);
        }
    }
}
