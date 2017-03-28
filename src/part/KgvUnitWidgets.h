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

#ifndef KGVUNITWIDGETS_H
#define KGVUNITWIDGETS_H

#include <QSpinBox>
#include <QIntValidator>
#include <QLineEdit>
#include <QComboBox>
#include <KgvUnit.h>
#include <QEvent>
#include <QDoubleSpinBox>

// ----------------------------------------------------------------
//                          Support classes


class KgvUnitDoubleBase;

// ### TODO: put it out of the public header file (if possible)
/**
 * Validator for the unit widget classes
 * \internal
 * \since 1.4 (change of behavior)
 */
class KgvUnitDoubleValidator : public QDoubleValidator
{
public:
    KgvUnitDoubleValidator(KgvUnitDoubleBase *base, QObject *parent);

    QValidator::State validate(QString&, int&) const override;

private:
    KgvUnitDoubleBase *m_base;
};


/**
 * Base for the unit widgets
 * \since 1.4 (change of behavior)
 */
class KgvUnitDoubleBase
{
public:
    KgvUnitDoubleBase( KgvUnit::Unit unit, unsigned int precision ) : m_unit( unit ), m_precision( precision ) {}
    virtual ~KgvUnitDoubleBase() {}

    virtual void changeValue( double ) = 0;
    virtual void setUnit( KgvUnit::Unit = KgvUnit::U_PT ) = 0;

    void setValueInUnit( double value, KgvUnit::Unit unit )
    {
        changeValue( KgvUnit::ptToUnit( KgvUnit::fromUserValue( value, unit ), m_unit ) );
    }

    void setPrecision( unsigned int precision ) { m_precision = precision; };

protected:
    friend class KgvUnitDoubleValidator;
    /**
     * Transform the double in a nice text, using locale symbols
     * @param value the number as double
     * @return the resulting string
     */
    QString getVisibleText( double value ) const;
    /**
     * Transform a string into a double, while taking care of locale specific symbols.
     * @param str the string to transform into a number
     * @param ok true, if the conversion was successful
     * @return the value as double
     */
    double toDouble( const QString& str, bool* ok ) const;

protected:
    KgvUnitDoubleValidator *m_validator;
    KgvUnit::Unit m_unit;
    unsigned int m_precision;
};


// ----------------------------------------------------------------
//                          Widget classes


/**
 * Spin box for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KgvUnitDoubleSpinBox : public QDoubleSpinBox, public KgvUnitDoubleBase
{
    Q_OBJECT
public:
    explicit KgvUnitDoubleSpinBox(QWidget *parent = nullptr);
    // lower, upper, step and value are in pt
    KgvUnitDoubleSpinBox( QWidget *parent, double lower, double upper, double step, double value = 0.0,
                         KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2);
    // added so the class can be used in .ui files(by Tymoteusz Majewski, maju7@o2.pl)
    void changeValue(double) override;
    void setUnit(KgvUnit::Unit = KgvUnit::U_PT) override;

    /// @return the current value, converted in points
    double value( void ) const;

    /// Set minimum value in points.
    void setMinValue(double min);

    /// Set maximum value in points.
    void setMaxValue(double max);

    /// Set step size in the current unit.
    void setLineStep(double step);

    /// Set step size in points.
    void setLineStepPt(double step);

    /// Set minimum, maximum value and the step size (all in points) (by Tymoteusz Majewski, maju7@o2.pl)
    void setMinMaxStep( double min, double max, double step );

signals:
    /// emitted like valueChanged in the parent, but this one emits the point value
    void valueChangedPt( double );


private:
    double m_lowerInPoints; ///< lowest value in points
    double m_upperInPoints; ///< highest value in points
    double m_stepInPoints;  ///< step in points

private slots:
    // exists to do emits for valueChangedPt
    void privateValueChanged();
};


/**
 * Line edit for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KgvUnitDoubleLineEdit : public QLineEdit, public KgvUnitDoubleBase
{
    Q_OBJECT
public:
    explicit KgvUnitDoubleLineEdit(QWidget *parent = nullptr);
    KgvUnitDoubleLineEdit( QWidget *parent, double lower, double upper, double value = 0.0, KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2);

    void changeValue(double) override;
    void setUnit(KgvUnit::Unit = KgvUnit::U_PT) override;

    /// @return the current value, converted in points
    double value( void ) const;

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    double m_value;
    double m_lower;
    double m_upper;
    double m_lowerInPoints; ///< lowest value in points
    double m_upperInPoints; ///< highest value in points
};

/**
 * Combo box for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KgvUnitDoubleComboBox : public QComboBox, public KgvUnitDoubleBase
{
    Q_OBJECT
public:
    explicit KgvUnitDoubleComboBox(QWidget *parent = nullptr);
    KgvUnitDoubleComboBox( QWidget *parent, double lower, double upper, double value = 0.0, KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2);

    void changeValue(double) override;
    void updateValue( double );
    void setUnit(KgvUnit::Unit = KgvUnit::U_PT) override;

    /// @return the current value, converted in points
    double value( void ) const;
    void insertItem( double, int index = -1 );

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

signals:
    void valueChanged(double);

private slots:
    void slotActivated( int );

protected:
    double m_value;
    double m_lower;
    double m_upper;
    double m_lowerInPoints; ///< lowest value in points
    double m_upperInPoints; ///< highest value in points
};

/**
 * Combo box (with spin control) for double precision numbers with unit display
 * \since 1.4 (change of behavior)
 */
class KgvUnitDoubleSpinComboBox : public QWidget
{
    Q_OBJECT
public:
    explicit KgvUnitDoubleSpinComboBox(QWidget *parent = nullptr);
    KgvUnitDoubleSpinComboBox( QWidget *parent, double lower, double upper, double step, double value = 0.0, KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2);

    void insertItem( double, int index = -1 );
    void updateValue( double );
    /// @return the current value, converted in points
    double value( void ) const;

signals:
    void valueChanged(double);

private slots:
    void slotUpClicked();
    void slotDownClicked();

private:
    KgvUnitDoubleComboBox *m_combo;
    double m_step;
};

#endif // KGVUNITWIDGETS_H

