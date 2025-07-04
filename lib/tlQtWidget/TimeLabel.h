// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Time label.
        class TimeLabel : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                OTIO_NS::RationalTime value
                READ value
                WRITE setValue)
            Q_PROPERTY(
                tl::timeline::TimeUnits timeUnits
                READ timeUnits
                WRITE setTimeUnits)

        public:
            TimeLabel(QWidget* parent = nullptr);

            virtual ~TimeLabel();

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Get the time value.
            const OTIO_NS::RationalTime& value() const;

            //! Get the time units.
            timeline::TimeUnits timeUnits() const;

        public Q_SLOTS:
            //! Set the time value.
            void setValue(const OTIO_NS::RationalTime&);

            //! Set the time units.
            void setTimeUnits(tl::timeline::TimeUnits);

        private:
            void _textUpdate();

            FEATHER_TK_PRIVATE();
        };
    }
}
