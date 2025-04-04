// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Util.h>

namespace tl
{
    namespace qtwidget
    {
        QSize toQt(const dtk::Size2I& value)
        {
            return QSize(value.w, value.h);
        }

        dtk::Size2I fromQt(const QSize& value)
        {
            return dtk::Size2I(value.width(), value.height());
        }

        QColor toQt(const dtk::Color4F& value)
        {
            return QColor::fromRgbF(value.r, value.g, value.b, value.a);
        }

        dtk::Color4F fromQt(const QColor& value)
        {
            return dtk::Color4F(
                value.redF(),
                value.greenF(),
                value.blueF(),
                value.alphaF());
        }

        void setFloatOnTop(bool value, QWidget* window)
        {
            if (value && !(window->windowFlags() & Qt::WindowStaysOnTopHint))
            {
                window->setWindowFlags(window->windowFlags() | Qt::WindowStaysOnTopHint);
                window->show();
            }
            else if (!value && (window->windowFlags() & Qt::WindowStaysOnTopHint))
            {
                window->setWindowFlags(window->windowFlags() & ~Qt::WindowStaysOnTopHint);
                window->show();
            }
        }
    }
}
