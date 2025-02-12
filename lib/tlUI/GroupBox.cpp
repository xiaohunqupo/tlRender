// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/GroupBox.h>

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace ui
    {
        struct GroupBox::Private
        {
            std::string text;
            FontRole fontRole = FontRole::Label;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int spacing = 0;
                int border = 0;

                bool textInit = true;
                image::FontInfo fontInfo;
                image::FontMetrics fontMetrics;
                math::Size2i textSize;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<std::shared_ptr<image::Glyph> > glyphs;
            };
            DrawData draw;
        };

        void GroupBox::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::GroupBox", context, parent);
        }

        GroupBox::GroupBox() :
            _p(new Private)
        {}

        GroupBox::~GroupBox()
        {}

        std::shared_ptr<GroupBox> GroupBox::create(
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<GroupBox>(new GroupBox);
            out->_init(context, parent);
            return out;
        }

        std::shared_ptr<GroupBox> GroupBox::create(
            const std::string& text,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GroupBox>(new GroupBox);
            out->_init(context, parent);
            out->setText(text);
            return out;
        }

        void GroupBox::setText(const std::string& value)
        {
            TLRENDER_P();
            if (value == p.text)
                return;
            p.text = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setFontRole(FontRole value)
        {
            TLRENDER_P();
            if (value == p.fontRole)
                return;
            p.fontRole = value;
            p.size.textInit = true;
            p.draw.glyphs.clear();
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GroupBox::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();
            math::Box2i g = value;
            g.min.y += p.size.fontMetrics.lineHeight + p.size.spacing;
            g = g.margin(-(p.size.border + p.size.margin));
            for (const auto& child : _children)
            {
                child->setGeometry(g);
            }
        }

        void GroupBox::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged = event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin = event.style->getSizeRole(SizeRole::MarginSmall, _displayScale);
                p.size.spacing = event.style->getSizeRole(SizeRole::SpacingSmall, _displayScale);
                p.size.border = event.style->getSizeRole(SizeRole::Border, _displayScale);
            }
            if (displayScaleChanged || p.size.textInit || p.size.sizeInit)
            {
                p.size.fontInfo = event.style->getFontRole(p.fontRole, _displayScale);
                p.size.fontMetrics = event.fontSystem->getMetrics(p.size.fontInfo);
                p.size.textSize = event.fontSystem->getSize(p.text, p.size.fontInfo);
                p.draw.glyphs.clear();
            }
            p.size.sizeInit = false;
            p.size.textInit = false;

            _sizeHint = math::Size2i();
            for (const auto& child : _children)
            {
                const math::Size2i& sizeHint = child->getSizeHint();
                _sizeHint.w = std::max(_sizeHint.w, sizeHint.w);
                _sizeHint.h = std::max(_sizeHint.h, sizeHint.h);
            }
            _sizeHint.w += p.size.margin * 2 + p.size.border * 2;
            _sizeHint.h += p.size.margin * 2 + p.size.border * 2;
            _sizeHint.w = std::max(_sizeHint.w, p.size.textSize.w);
            _sizeHint.h += p.size.fontMetrics.lineHeight + p.size.spacing;
        }

        void GroupBox::clipEvent(const math::Box2i& clipRect, bool clipped)
        {
            IWidget::clipEvent(clipRect, clipped);
            TLRENDER_P();
            if (clipped)
            {
                p.draw.glyphs.clear();
            }
        }

        void GroupBox::drawEvent(
            const math::Box2i& drawRect,
            const DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            TLRENDER_P();

            const math::Box2i& g = _geometry;

            if (!p.text.empty() && p.draw.glyphs.empty())
            {
                p.draw.glyphs = event.fontSystem->getGlyphs(p.text, p.size.fontInfo);
            }
            event.render->drawText(
                p.draw.glyphs,
                math::Vector2i(g.x(), g.y() + p.size.fontMetrics.ascender),
                event.style->getColorRole(ColorRole::Text));

            const math::Box2i g2(
                math::Vector2i(
                    g.min.x,
                    g.min.y + p.size.fontMetrics.lineHeight + p.size.spacing),
                g.max);
            event.render->drawMesh(
                border(g2, p.size.border, p.size.margin),
                math::Vector2i(),
                event.style->getColorRole(ColorRole::Border));
        }
    }
}
