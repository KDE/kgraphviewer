/* This file is part of KGraphViewer.
   Copyright (C) 2005-2006 Gaël de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* This file was part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#ifndef __KOUNITWIDGETS_H__
#define __KOUNITWIDGETS_H__

#include <knuminput.h>
#include <knumvalidator.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <KgvUnit.h>


// ----------------------------------------------------------------
//                          Support classes


class KgvUnitDoubleBase;

// ### TODO: put it out of the public header file (if possible)
/**
 * Validator for the unit widget classes
 * \internal
 * \since 1.4 (change of behavior)
 */
class KgvUnitDoubleValidator : public KDoubleValidator
{
public:
    KgvUnitDoubleValidator( KgvUnitDoubleBase *base, QObject *parent, const char *name = 0 );

    virtual QValidator::State validate( QString &, int & ) const;

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
     * Transfrom a string into a double, while taking care of locale specific symbols.
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
class KgvUnitDoubleSpinBox : public KDoubleSpinBox, public KgvUnitDoubleBase
{
    Q_OBJECT
public:
    KgvUnitDoubleSpinBox( QWidget *parent = 0L, const char *name = 0L );
    // lower, upper, step and value are in pt
    KgvUnitDoubleSpinBox( QWidget *parent, double lower, double upper, double step, double value = 0.0,
                         KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2, const char *name = 0 );
    // added so the class can be used in .ui files(by Tymoteusz Majewski, maju7@o2.pl)
    virtual void changeValue( double );
    virtual void setUnit( KgvUnit::Unit = KgvUnit::U_PT );

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
class KgvUnitDoubleLineEdit : public KLineEdit, public KgvUnitDoubleBase
{
    Q_OBJECT
public:
    KgvUnitDoubleLineEdit( QWidget *parent = 0L, const char *name = 0L );
    KgvUnitDoubleLineEdit( QWidget *parent, double lower, double upper, double value = 0.0, KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2, const char *name = 0 );

    virtual void changeValue( double );
    virtual void setUnit( KgvUnit::Unit = KgvUnit::U_PT );

    /// @return the current value, converted in points
    double value( void ) const;

protected:
    bool eventFilter( QObject* obj, QEvent* ev );

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
class KgvUnitDoubleComboBox : public KComboBox, public KgvUnitDoubleBase
{
    Q_OBJECT
public:
    KgvUnitDoubleComboBox( QWidget *parent = 0L, const char *name = 0L );
    KgvUnitDoubleComboBox( QWidget *parent, double lower, double upper, double value = 0.0, KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2, const char *name = 0 );

    virtual void changeValue( double );
    void updateValue( double );
    virtual void setUnit( KgvUnit::Unit = KgvUnit::U_PT );

    /// @return the current value, converted in points
    double value( void ) const;
    void insertItem( double, int index = -1 );

protected:
    bool eventFilter( QObject* obj, QEvent* ev );

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
    KgvUnitDoubleSpinComboBox( QWidget *parent = 0L, const char *name = 0L );
    KgvUnitDoubleSpinComboBox( QWidget *parent, double lower, double upper, double step, double value = 0.0, KgvUnit::Unit unit = KgvUnit::U_PT, unsigned int precision = 2, const char *name = 0 );

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

#endif

