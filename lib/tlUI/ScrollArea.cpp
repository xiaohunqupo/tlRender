// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ScrollArea.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ScrollArea::Private
        {
            ScrollType scrollType = ScrollType::Both;
            math::Vector2i scrollSize;
            math::Vector2i scrollPos;
            std::function<void(const math::Vector2i&)> scrollSizeCallback;
            std::function<void(const math::Vector2i&)> scrollPosCallback;
            bool border = true;

            struct SizeData
            {
                int border = 0;
            };
            SizeData size;
        };

        void ScrollArea::_init(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::ScrollArea", context, parent);
            TLRENDER_P();
            p.scrollType = scrollType;
        }

        ScrollArea::ScrollArea() :
            _p(new Private)
        {}

        ScrollArea::~ScrollArea()
        {}

        std::shared_ptr<ScrollArea> ScrollArea::create(
            const std::shared_ptr<system::Context>& context,
            ScrollType scrollType,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ScrollArea>(new ScrollArea);
            out->_init(context, scrollType, parent);
            return out;
        }

        const math::Vector2i& ScrollArea::getScrollSize() const
        {
            return _p->scrollSize;
        }

        void ScrollArea::setScrollSizeCallback(const std::function<void(const math::Vector2i&)>& value)
        {
            _p->scrollSizeCallback = value;
        }

        const math::Vector2i& ScrollArea::getScrollPos() const
        {
            return _p->scrollPos;
        }

        void ScrollArea::setScrollPos(const math::Vector2i& value, bool clamp)
        {
            TLRENDER_P();
            math::Vector2i tmp = value;
            if (clamp)
            {
                const math::BBox2i g = _geometry.margin(-p.size.border);
                tmp = math::Vector2i(
                    math::clamp(tmp.x, 0, std::max(0, p.scrollSize.x - g.w())),
                    math::clamp(tmp.y, 0, std::max(0, p.scrollSize.y - g.h())));
            }
            if (tmp == p.scrollPos)
                return;
            p.scrollPos = tmp;
            _updates |= Update::Size;
            _updates |= Update::Draw;
            if (p.scrollPosCallback)
            {
                p.scrollPosCallback(p.scrollPos);
            }
        }

        void ScrollArea::setScrollPosCallback(const std::function<void(const math::Vector2i&)>& value)
        {
            _p->scrollPosCallback = value;
        }

        math::BBox2i ScrollArea::getChildrenClipRect() const
        {
            TLRENDER_P();
            return _geometry.margin(-p.size.border);
        }

        void ScrollArea::setBorder(bool value)
        {
            TLRENDER_P();
            if (value == p.border)
                return;
            p.border = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void ScrollArea::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            const math::BBox2i g = value.margin(-p.size.border);

            math::Vector2i scrollSize;
            for (const auto& child : _children)
            {
                math::Vector2i size = child->getSizeHint();
                switch (p.scrollType)
                {
                case ScrollType::Horizontal:
                    size.y = std::max(size.y, g.h());
                    break;
                case ScrollType::Vertical:
                    size.x = std::max(size.x, g.w());
                    break;
                default: break;
                }
                scrollSize.x = std::max(scrollSize.x, size.x);
                scrollSize.y = std::max(scrollSize.y, size.y);
                const math::BBox2i g2(
                    g.min.x - p.scrollPos.x,
                    g.min.y - p.scrollPos.y,
                    size.x,
                    size.y);
                child->setGeometry(g2);
            }
            if (scrollSize != p.scrollSize)
            {
                p.scrollSize = scrollSize;
                _updates |= Update::Size;
                _updates |= Update::Draw;
                if (p.scrollSizeCallback)
                {
                    p.scrollSizeCallback(p.scrollSize);
                }
            }

            const math::Vector2i scrollPos(
                math::clamp(p.scrollPos.x, 0, std::max(0, p.scrollSize.x - g.w())),
                math::clamp(p.scrollPos.y, 0, std::max(0, p.scrollSize.y - g.h())));
            if (scrollPos != p.scrollPos)
            {
                p.scrollPos = scrollPos;
                _updates |= Update::Size;
                _updates |= Update::Draw;
                if (p.scrollPosCallback)
                {
                    p.scrollPosCallback(p.scrollPos);
                }
            }
        }

        void ScrollArea::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            p.size.border = p.border ?
                event.style->getSizeRole(SizeRole::Border, event.displayScale) :
                0;

            _sizeHint = math::Vector2i();
            switch (p.scrollType)
            {
                case ScrollType::Horizontal:
                    _sizeHint.x =
                        event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);
                    for (const auto& child : _children)
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        _sizeHint.y = std::max(_sizeHint.y, sizeHint.y);
                    }
                    break;
                case ScrollType::Vertical:
                    _sizeHint.y =
                        event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);
                    for (const auto& child : _children)
                    {
                        const math::Vector2i& sizeHint = child->getSizeHint();
                        _sizeHint.x = std::max(_sizeHint.x, sizeHint.x);
                    }
                    break;
                case ScrollType::Both:
                    _sizeHint.x = _sizeHint.y =
                        event.style->getSizeRole(SizeRole::ScrollArea, event.displayScale);
                    break;
            }
            _sizeHint.x += p.size.border * 2;
            _sizeHint.y += p.size.border * 2;
        }

        void ScrollArea::drawEvent(
            const math::BBox2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::BBox2i& g = _geometry;

            if (p.border)
            {
                event.render->drawMesh(
                    border(g, p.size.border),
                    math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));
            }
        }
    }
}
