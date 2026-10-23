/* This file is part of KGraphViewer.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/* This file was part of the KDE project
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#include "KgvUnitWidgets.h"

// lib
#include "kgraphviewerlib_debug.h"

// ----------------------------------------------------------------
//                          Support classes

QString KgvUnitDoubleBase::getVisibleText(double value) const
{
    const QString num(QStringLiteral("%1%2").arg(QLocale().toString(value, m_precision), KgvUnit::unitName(m_unit)));
    qCDebug(KGRAPHVIEWERLIB_LOG) << "getVisibleText: " << QString::number(value, 'f', 12) << " => " << num;
    return num;
}

double KgvUnitDoubleBase::toDouble(const QString &str, bool *ok) const
{
    QString str2(str);
    /* KLocale::readNumber wants the thousand separator exactly at 1000.
       But when editing, it might be anywhere. So we need to remove it. */
    const QString sep(QLocale().groupSeparator());
    if (!sep.isEmpty())
        str2.remove(sep);
    str2.remove(KgvUnit::unitName(m_unit));
    const double dbl = QLocale().toDouble(str2, ok);
    if (ok)
        qCDebug(KGRAPHVIEWERLIB_LOG) << "toDouble:" << str << ": => :" << str2 << ": => " << QString::number(dbl, 'f', 12);
    else
        qCWarning(KGRAPHVIEWERLIB_LOG) << "error:" << str << ": => :" << str2 << ":";
    return dbl;
}

// ----------------------------------------------------------------
//                          Widget classes

KgvUnitDoubleSpinBox::KgvUnitDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
    , KgvUnitDoubleBase(KgvUnit::U_PT, 2)
    , m_lowerInPoints(-9999)
    , m_upperInPoints(9999)
    , m_stepInPoints(1)
{
    QDoubleSpinBox::setDecimals(2);
    setUnit(KgvUnit::U_PT);

    connect(this, &KgvUnitDoubleSpinBox::valueChanged, this, &KgvUnitDoubleSpinBox::privateValueChanged);
}

KgvUnitDoubleSpinBox::KgvUnitDoubleSpinBox(QWidget *parent, double lower, double upper, double step, double value, KgvUnit::Unit unit, unsigned int precision)
    : QDoubleSpinBox(parent)
    , KgvUnitDoubleBase(unit, precision)
    , m_lowerInPoints(lower)
    , m_upperInPoints(upper)
    , m_stepInPoints(step)
{
    setMinimum(lower);
    setMaximum(upper);
    setSingleStep(step);
    setValue(value);
    setDecimals(precision);
    QDoubleSpinBox::setMinimum(lower);
    QDoubleSpinBox::setMaximum(upper);
    QDoubleSpinBox::setSingleStep(step);
    QDoubleSpinBox::setValue(value);
    m_unit = KgvUnit::U_PT;
    setUnit(unit);
    changeValue(value);
    setLineStep(0.5);

    connect(this, &KgvUnitDoubleSpinBox::valueChanged, this, &KgvUnitDoubleSpinBox::privateValueChanged);
}

void KgvUnitDoubleSpinBox::changeValue(double val)
{
    QDoubleSpinBox::setValue(KgvUnit::toUserValue(val, m_unit));
    // TODO: emit valueChanged ONLY if the value was out-of-bounds
    // This will allow the 'user' dialog to set a dirty bool and ensure
    // a proper value is getting saved.
}

void KgvUnitDoubleSpinBox::privateValueChanged()
{
    Q_EMIT valueChangedPt(value());
}

void KgvUnitDoubleSpinBox::setUnit(KgvUnit::Unit unit)
{
    double oldvalue = KgvUnit::fromUserValue(QDoubleSpinBox::value(), m_unit);
    QDoubleSpinBox::setMinimum(KgvUnit::toUserValue(m_lowerInPoints, unit));
    QDoubleSpinBox::setMaximum(KgvUnit::toUserValue(m_upperInPoints, unit));
    QDoubleSpinBox::setSingleStep(KgvUnit::toUserValue(m_stepInPoints, unit));
    QDoubleSpinBox::setValue(KgvUnit::ptToUnit(oldvalue, unit));
    m_unit = unit;
    setSuffix(KgvUnit::unitName(unit).prepend(QLatin1Char(' ')));
}

double KgvUnitDoubleSpinBox::value(void) const
{
    return KgvUnit::fromUserValue(QDoubleSpinBox::value(), m_unit);
}

void KgvUnitDoubleSpinBox::setMinValue(double min)
{
    m_lowerInPoints = min;
    QDoubleSpinBox::setMinimum(KgvUnit::toUserValue(m_lowerInPoints, m_unit));
}

void KgvUnitDoubleSpinBox::setMaxValue(double max)
{
    m_upperInPoints = max;
    QDoubleSpinBox::setMaximum(KgvUnit::toUserValue(m_upperInPoints, m_unit));
}

void KgvUnitDoubleSpinBox::setLineStep(double step)
{
    m_stepInPoints = KgvUnit::toUserValue(step, KgvUnit::U_PT);
    QDoubleSpinBox::setSingleStep(step);
}

void KgvUnitDoubleSpinBox::setLineStepPt(double step)
{
    m_stepInPoints = step;
    QDoubleSpinBox::setSingleStep(KgvUnit::toUserValue(m_stepInPoints, m_unit));
}

void KgvUnitDoubleSpinBox::setMinMaxStep(double min, double max, double step)
{
    setMinimum(min);
    setMaximum(max);
    setLineStepPt(step);
}

#include "moc_KgvUnitWidgets.cpp"
