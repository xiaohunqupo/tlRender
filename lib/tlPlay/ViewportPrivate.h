// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlay/Viewport.h>

namespace tl
{
    namespace play
    {
        class ViewportColorWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(ViewportColorWidget);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ViewportColorWidget();

        public:
            virtual ~ViewportColorWidget();

            static std::shared_ptr<ViewportColorWidget> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setColor(const image::Color4f&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;

        private:
            void _colorUpdate();

            TLRENDER_PRIVATE();
        };
    }
}

