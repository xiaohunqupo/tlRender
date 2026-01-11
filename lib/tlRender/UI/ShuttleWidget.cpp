// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <tlRender/UI/ShuttleWidget.h>

#include <ftk/Core/Format.h>

#include <optional>

namespace tl
{
    namespace ui
    {
        struct ShuttleWidget::Private
        {
            std::string iconPrefix;
            std::vector<std::shared_ptr<ftk::Image> > iconImages;
            int iconIndex = 0;
            std::function<void(bool)> activeCallback;
            std::function<void(int)> callback;

            struct SizeData
            {
                std::optional<float> displayScale;
                int margin = 0;
                float iconScale = 1.F;
                ftk::Size2I sizeHint;
            };
            SizeData size;
        };

        void ShuttleWidget::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& iconPrefix,
            const std::shared_ptr<IWidget>& parent)
        {
            IMouseWidget::_init(
                context,
                "tl::ui::ShuttleWidget",
                parent);
            FTK_P();

            _setMouseHoverEnabled(true);
            _setMousePressEnabled(true);

            p.iconPrefix = iconPrefix;
        }

        ShuttleWidget::ShuttleWidget() :
            _p(new Private)
        {}

        ShuttleWidget::~ShuttleWidget()
        {}

        std::shared_ptr<ShuttleWidget> ShuttleWidget::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& iconPrefix,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ShuttleWidget>(new ShuttleWidget);
            out->_init(context, iconPrefix, parent);
            return out;
        }

        void ShuttleWidget::setActiveCallback(const std::function<void(bool)>& value)
        {
            _p->activeCallback = value;
        }

        void ShuttleWidget::setCallback(const std::function<void(int)>& value)
        {
            _p->callback = value;
        }

        ftk::Size2I ShuttleWidget::getSizeHint() const
        {
            return _p->size.sizeHint;
        }

        void ShuttleWidget::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            FTK_P();
            if (!p.size.displayScale.has_value() ||
                (p.size.displayScale.has_value() && p.size.displayScale.value() != event.displayScale))
            {
                p.size.displayScale = event.displayScale;
                p.size.margin = event.style->getSizeRole(ftk::SizeRole::MarginInside, event.displayScale);
                p.size.iconScale = event.displayScale;
                p.iconImages.clear();
                for (int i = 0; i < 8; ++i)
                {
                    p.iconImages.push_back(event.iconSystem->get(
                        ftk::Format("{0}{1}").
                        arg(p.iconPrefix).
                        arg(i),
                        event.displayScale));
                }
                p.size.sizeHint = ftk::Size2I();
                if (!p.iconImages.empty())
                {
                    p.size.sizeHint = p.iconImages.front()->getSize();
                }
                p.size.sizeHint = margin(p.size.sizeHint, p.size.margin);
            }
        }

        void ShuttleWidget::drawEvent(const ftk::Box2I& drawRect, const ftk::DrawEvent& event)
        {
            IMouseWidget::drawEvent(drawRect, event);
            FTK_P();
            const ftk::Box2I& g = getGeometry();
            if (p.iconIndex >= 0 && p.iconIndex < p.iconImages.size())
            {
                const ftk::Size2I& iconSize = p.iconImages.front()->getSize();
                const ftk::Box2I g2(
                    g.min.x + g.w() / 2 - iconSize.w / 2,
                    g.min.y + g.h() / 2 - iconSize.h / 2,
                    iconSize.w,
                    iconSize.h);
                event.render->drawImage(
                    p.iconImages[p.iconIndex],
                    g2,
                    event.style->getColorRole(isEnabled() ?
                        ftk::ColorRole::Text :
                        ftk::ColorRole::TextDisabled));
            }
            if (_isMousePressed())
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ftk::ColorRole::Pressed));
            }
            else if (_isMouseInside())
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ftk::ColorRole::Hover));
            }
        }

        void ShuttleWidget::mouseEnterEvent(ftk::MouseEnterEvent& event)
        {
            IMouseWidget::mouseEnterEvent(event);
            setDrawUpdate();
        }

        void ShuttleWidget::mouseLeaveEvent()
        {
            IMouseWidget::mouseLeaveEvent();
            setDrawUpdate();
        }

        void ShuttleWidget::mouseMoveEvent(ftk::MouseMoveEvent& event)
        {
            IMouseWidget::mouseMoveEvent(event);
            FTK_P();
            const ftk::Box2I& g = getGeometry();
            if (_isMousePressed() && g.isValid())
            {
                const int d = event.pos.x - _getMousePressPos().x;
                const int i = d / (g.w() / 4);
                if (i != p.iconIndex)
                {
                    p.iconIndex = i % p.iconImages.size();
                    setDrawUpdate();
                    if (p.callback)
                    {
                        p.callback(i);
                    }
                }
            }
        }

        void ShuttleWidget::mousePressEvent(ftk::MouseClickEvent& event)
        {
            IMouseWidget::mousePressEvent(event);
            FTK_P();
            setDrawUpdate();
            if (p.activeCallback)
            {
                p.activeCallback(true);
            }
        }

        void ShuttleWidget::mouseReleaseEvent(ftk::MouseClickEvent& event)
        {
            IMouseWidget::mouseReleaseEvent(event);
            FTK_P();
            p.iconIndex = 0;
            setDrawUpdate();
            if (p.activeCallback)
            {
                p.activeCallback(false);
            }
        }
    }
}
