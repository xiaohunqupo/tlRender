// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimeLabel.h>

#include <tlQtWidget/Util.h>

#include <QHBoxLayout>
#include <QLabel>

namespace tl
{
    namespace qtwidget
    {
        struct TimeLabel::Private
        {
            OTIO_NS::RationalTime value = time::invalidTime;
            timeline::TimeUnits timeUnits = timeline::TimeUnits::Timecode;
            QLabel* label = nullptr;
            qt::TimeObject* timeObject = nullptr;
        };

        TimeLabel::TimeLabel(QWidget* parent) :
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

        TimeLabel::~TimeLabel()
        {}

        void TimeLabel::setTimeObject(qt::TimeObject* timeObject)
        {
            FEATHER_TK_P();
            if (timeObject == p.timeObject)
                return;
            if (p.timeObject)
            {
                disconnect(
                    p.timeObject,
                    SIGNAL(timeUnitsChanged(tl::timeline::TimeUnits)),
                    this,
                    SLOT(setTimeUnits(tl::timeline::TimeUnits)));
            }
            p.timeObject = timeObject;
            if (p.timeObject)
            {
                p.timeUnits = p.timeObject->timeUnits();
                connect(
                    p.timeObject,
                    SIGNAL(timeUnitsChanged(tl::timeline::TimeUnits)),
                    SLOT(setTimeUnits(tl::timeline::TimeUnits)));
            }
            _textUpdate();
            updateGeometry();
        }

        const OTIO_NS::RationalTime& TimeLabel::value() const
        {
            return _p->value;
        }

        timeline::TimeUnits TimeLabel::timeUnits() const
        {
            return _p->timeUnits;
        }

        void TimeLabel::setValue(const OTIO_NS::RationalTime& value)
        {
            FEATHER_TK_P();
            if (value.value() == p.value.value() &&
                value.rate() == p.value.rate())
                return;
            p.value = value;
            _textUpdate();
        }

        void TimeLabel::setTimeUnits(timeline::TimeUnits value)
        {
            FEATHER_TK_P();
            if (value == p.timeUnits)
                return;
            p.timeUnits = value;
            _textUpdate();
            updateGeometry();
        }

        void TimeLabel::_textUpdate()
        {
            FEATHER_TK_P();
            const std::string label = timeline::timeToText(p.value, p.timeUnits);
            p.label->setText(QString::fromUtf8(label.c_str()));
        }
    }
}
