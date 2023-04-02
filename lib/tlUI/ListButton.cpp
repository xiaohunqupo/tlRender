// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/ListButton.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct ListButton::Private
        {
            struct Size
            {
                imaging::FontInfo fontInfo;
                imaging::FontMetrics fontMetrics;
                math::Vector2i textSize;
                int margin = 0;
                int spacing = 0;
            };
            Size size;
        };

        void ListButton::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IButton::_init("tl::ui::ListButton", context, parent);
            setButtonRole(ColorRole::None);
        }

        ListButton::ListButton() :
            _p(new Private)
        {}

        ListButton::~ListButton()
        {}

        std::shared_ptr<ListButton> ListButton::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ListButton>(new ListButton);
            out->_init(context, parent);
            return out;
        }

        void ListButton::sizeEvent(const SizeEvent& event)
        {
            IButton::sizeEvent(event);
            TLRENDER_P();
            
            p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall) * event.contentScale;
            p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall) * event.contentScale;

            _sizeHint.x = 0;
            _sizeHint.y = 0;
            if (!_text.empty())
            {
                p.size.fontInfo = _fontInfo;
                p.size.fontInfo.size *= event.contentScale;
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->measure(_text, p.size.fontInfo);

                _sizeHint.x = event.fontSystem->measure(_text, p.size.fontInfo).x;
                _sizeHint.y = p.size.fontMetrics.lineHeight;
            }
            if (_iconImage)
            {
                _sizeHint.x += _iconImage->getWidth();
                _sizeHint.y = std::max(
                    _sizeHint.y,
                    static_cast<int>(_iconImage->getHeight()));

                if (!_text.empty())
                {
                    _sizeHint.x += p.size.spacing;
                }
            }
            _sizeHint.x += p.size.margin * 2;
            _sizeHint.y += p.size.margin * 2;
        }

        void ListButton::drawEvent(const DrawEvent& event)
        {
            IButton::drawEvent(event);
            TLRENDER_P();

            math::BBox2i g = _geometry;

            const ColorRole colorRole = _checked->get() ?
                ColorRole::Checked :
                _buttonRole;
            if (colorRole != ColorRole::None)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(colorRole));
            }

            if (_pressed && _geometry.contains(_cursorPos))
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Pressed));
            }
            else if (_inside)
            {
                event.render->drawRect(
                    g,
                    event.style->getColorRole(ColorRole::Hover));
            }

            int x = g.x() + p.size.margin;
            if (_iconImage)
            {
                const auto iconSize = _iconImage->getSize();
                event.render->drawImage(
                  _iconImage,
                  math::BBox2i(x, g.y() + p.size.margin, iconSize.w, iconSize.h));
                x += iconSize.w + p.size.spacing;
            }
            
            if (!_text.empty())
            {
                math::Vector2i pos(
                    x,
                    g.y() + g.h() / 2 - p.size.textSize.y / 2 +
                        p.size.fontMetrics.ascender);
                event.render->drawText(
                    event.fontSystem->getGlyphs(_text, p.size.fontInfo),
                    pos,
                    event.style->getColorRole(ColorRole::Text));
            }
        }
    }
}