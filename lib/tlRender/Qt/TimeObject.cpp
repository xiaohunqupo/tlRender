// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/Qt/TimeObject.h>

#include <ftk/Core/Error.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/String.h>

#include <QDataStream>

#include <array>

namespace tl
{
    namespace qt
    {
        TimeObject::TimeObject(
            const std::shared_ptr<TimeUnitsModel>& model,
            QObject* parent) :
            QObject(parent),
            _model(model)
        {}

        TimeUnits TimeObject::timeUnits() const
        {
            return _model->getTimeUnits();
        }

        void TimeObject::setTimeUnits(TimeUnits value)
        {
            const TimeUnits units = _model->getTimeUnits();
            _model->setTimeUnits(value);
            if (units != _model->getTimeUnits())
            {
                Q_EMIT timeUnitsChanged(_model->getTimeUnits());
            }
        }

        QDataStream& operator << (QDataStream& ds, const TimeUnits& value)
        {
            ds << static_cast<qint32>(value);
            return ds;
        }

        QDataStream& operator >> (QDataStream& ds, TimeUnits& value)
        {
            qint32 tmp = 0;
            ds >> tmp;
            value = static_cast<TimeUnits>(tmp);
            return ds;
        }

    }
}
