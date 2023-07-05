// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/CompareToolBar.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct CompareToolBar::Private
        {
            std::shared_ptr<ui::ButtonGroup> buttonGroup;
            std::map<timeline::CompareMode, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareOptionsObserver;
        };

        void CompareToolBar::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("tl::examples::play_gl::CompareToolBar", context);
            TLRENDER_P();

            p.buttonGroup = ui::ButtonGroup::create(ui::ButtonGroupType::Radio, context);
            const std::array<std::string, static_cast<size_t>(timeline::CompareMode::Count)> icons =
            {
                "CompareA",
                "CompareB",
                "CompareWipe",
                "CompareOverlay",
                "CompareDifference",
                "CompareHorizontal",
                "CompareVertical",
                "CompareTile"
            };
            const auto enums = timeline::getCompareModeEnums();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                const auto mode = enums[i];
                p.buttons[mode] = ui::ToolButton::create(context);
                p.buttons[mode]->setCheckable(true);
                p.buttons[mode]->setIcon(icons[i]);
                p.buttonGroup->addButton(p.buttons[mode]);
            }

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            for (size_t i = 0; i < enums.size(); ++i)
            {
                p.buttons[enums[i]]->setParent(p.layout);
            }

            auto appWeak = std::weak_ptr<App>(app);
            p.buttonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (value)
                        {
                            auto options = app->getFilesModel()->getCompareOptions();
                            options.mode = static_cast<timeline::CompareMode>(index);
                            app->getFilesModel()->setCompareOptions(options);
                        }
                    }
                });

            p.compareOptionsObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });
        }

        CompareToolBar::CompareToolBar() :
            _p(new Private)
        {}

        CompareToolBar::~CompareToolBar()
        {}

        std::shared_ptr<CompareToolBar> CompareToolBar::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<CompareToolBar>(new CompareToolBar);
            out->_init(app, context);
            return out;
        }

        void CompareToolBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void CompareToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void CompareToolBar::_compareUpdate(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            p.buttonGroup->setChecked(static_cast<int>(value.mode), true);
        }
    }
}