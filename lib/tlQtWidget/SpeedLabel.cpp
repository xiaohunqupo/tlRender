// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/SpeedLabel.h>

#include <tlQtWidget/Util.h>

#include <QHBoxLayout>
#include <QLabel>

namespace tl
{
    namespace qtwidget
    {
        struct SpeedLabel::Private
        {
            OTIO_NS::RationalTime value = time::invalidTime;
            QLabel* label = nullptr;
        };

        SpeedLabel::SpeedLabel(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            FEATHER_TK_P();

            const QFont fixedFont("Noto Mono");
            setFont(fixedFont);

            p.label = new QLabel;

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.label);
            setLayout(layout);

            _textUpdate();
        }

        SpeedLabel::~SpeedLabel()
        {}

        const OTIO_NS::RationalTime& SpeedLabel::value() const
        {
            return _p->value;
        }

        void SpeedLabel::setValue(const OTIO_NS::RationalTime& value)
        {
            FEATHER_TK_P();
            if (value.value() == p.value.value() &&
                value.rate() == p.value.rate())
                return;
            p.value = value;
            _textUpdate();
        }

        void SpeedLabel::_textUpdate()
        {
            FEATHER_TK_P();
            p.label->setText(QString("%1").
                arg(time::isValid(p.value) ? p.value.rate() : 0.0, 0, 'f', 2));
        }
    }
}
