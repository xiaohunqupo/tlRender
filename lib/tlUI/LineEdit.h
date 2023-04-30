// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Text line edit.
        class LineEdit : public IWidget
        {
            TLRENDER_NON_COPYABLE(LineEdit);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            LineEdit();

        public:
            ~LineEdit() override;

            //! Create a new text line edit.
            static std::shared_ptr<LineEdit> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Get the text.
            const std::string& getText() const;

            //! Set the text.
            void setText(const std::string&);

            //! Set the text callback.
            void setTextCallback(const std::function<void(const std::string&)>&);

            //! Set the formatting text.
            void setFormat(const std::string&);

            //! Set the font role.
            void setFontRole(FontRole);

            bool acceptsKeyFocus() const override;
            void clipEvent(bool, const ClipEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const DrawEvent&) override;
            void enterEvent() override;
            void leaveEvent() override;
            void mouseMoveEvent(MouseMoveEvent&) override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;
            void keyPressEvent(KeyEvent&) override;
            void keyReleaseEvent(KeyEvent&) override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
