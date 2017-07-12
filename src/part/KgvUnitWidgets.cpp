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

#include "kgraphviewerlib_debug.h"
#include "KgvUnitWidgets.h"

#include <QDebug>
#include <QLocale>
#include <qpushbutton.h>
#include <qlayout.h>
#include <QGridLayout>
#include <QEvent>

// ----------------------------------------------------------------
//                          Support classes


KgvUnitDoubleValidator::KgvUnitDoubleValidator(KgvUnitDoubleBase *base, QObject *parent)
: QDoubleValidator( parent ), m_base( base )
{
}

QValidator::State
KgvUnitDoubleValidator::validate( QString &s, int &pos ) const
{

    qCDebug(KGRAPHVIEWERLIB_LOG) << "KgvUnitDoubleValidator::validate : " << s << " at " << pos;
    QValidator::State result = Acceptable;

    QRegExp regexp ("([ a-zA-Z]+)$"); // Letters or spaces at end
    const int res = regexp.indexIn(s);

    if ( res == -1 )
    {
        // Nothing like an unit? The user is probably editing the unit
        qCDebug(KGRAPHVIEWERLIB_LOG) << "Intermediate (no unit)";
        return Intermediate;
    }

    // ### TODO: are all the QString::trimmed really necessary?
    const QString number ( s.left( res ).trimmed() );
    const QString unitName ( regexp.cap( 1 ).trimmed().toLower() );

    qCDebug(KGRAPHVIEWERLIB_LOG) << "Split:" << number << ":" << unitName << ":";

    bool ok = false;
    const double value = m_base->toDouble( number, &ok );
    double newVal = 0.0;
    if( ok )
    {
        KgvUnit::Unit unit = KgvUnit::unit( unitName, &ok );
        if ( ok )
            newVal = KgvUnit::fromUserValue( value, unit );
        else
        {
            // Probably the user is trying to edit the unit
            qCDebug(KGRAPHVIEWERLIB_LOG) << "Intermediate (unknown unit)";
            return Intermediate;
        }
    }
    else
    {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "Not a number:" << number;
        return Invalid;
    }

    newVal = KgvUnit::ptToUnit( newVal, m_base->m_unit );

    s = m_base->getVisibleText( newVal );

    return result;
}


QString KgvUnitDoubleBase::getVisibleText( double value ) const
{
    const QString num ( QString( "%1%2").arg( QLocale().toString( value, m_precision ), KgvUnit::unitName( m_unit ) ) );
    qCDebug(KGRAPHVIEWERLIB_LOG) << "getVisibleText: " << QString::number( value, 'f', 12 ) << " => " << num;
    return num;
}

double KgvUnitDoubleBase::toDouble( const QString& str, bool* ok ) const
{
    QString str2( str );
    /* KLocale::readNumber wants the thousand separator exactly at 1000.
       But when editing, it might be anywhere. So we need to remove it. */
    const QString sep( QLocale().groupSeparator() );
    if ( !sep.isEmpty() )
        str2.remove( sep );
    str2.remove( KgvUnit::unitName( m_unit ) );
    const double dbl = QLocale().toDouble( str2, ok );
    if ( ok )
      qCDebug(KGRAPHVIEWERLIB_LOG) << "toDouble:" << str << ": => :" << str2 << ": => " << QString::number( dbl, 'f', 12 );
    else
        qCWarning(KGRAPHVIEWERLIB_LOG) << "error:" << str << ": => :" << str2 << ":" << endl;
    return dbl;
}


// ----------------------------------------------------------------
//                          Widget classes


KgvUnitDoubleSpinBox::KgvUnitDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox( parent ), KgvUnitDoubleBase( KgvUnit::U_PT, 2 )
    , m_lowerInPoints( -9999 )
    , m_upperInPoints( 9999 )
    , m_stepInPoints( 1 )
{
    QDoubleSpinBox::setDecimals( 2 );
    m_validator = new KgvUnitDoubleValidator( this, this );
//     QSpinBox::setValidator( m_validator );
    //setAcceptLocalizedNumbers( true );
    setUnit( KgvUnit::U_PT );

    connect(this, static_cast<void(KgvUnitDoubleSpinBox::*)(double)>(&KgvUnitDoubleSpinBox::valueChanged),
            this, &KgvUnitDoubleSpinBox::privateValueChanged);
}


KgvUnitDoubleSpinBox::KgvUnitDoubleSpinBox( QWidget *parent, 
						    double lower, double upper,
						    double step, 
						    double value, 
						    KgvUnit::Unit unit, 
						    unsigned int precision)
    : QDoubleSpinBox( parent),
      KgvUnitDoubleBase( unit, precision ),
    m_lowerInPoints( lower ), m_upperInPoints( upper ), m_stepInPoints( step )
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
    m_validator = new KgvUnitDoubleValidator( this, this );
//     QSpinBox::setValidator( m_validator );
    //setAcceptLocalizedNumbers( true );
    setUnit( unit );
    changeValue( value );
    setLineStep( 0.5 );

    connect(this, static_cast<void(KgvUnitDoubleSpinBox::*)(double)>(&KgvUnitDoubleSpinBox::valueChanged),
            this, &KgvUnitDoubleSpinBox::privateValueChanged);
}

void
KgvUnitDoubleSpinBox::changeValue( double val )
{
    QDoubleSpinBox::setValue( KgvUnit::toUserValue( val, m_unit ) );
    // TODO: emit valueChanged ONLY if the value was out-of-bounds
    // This will allow the 'user' dialog to set a dirty bool and ensure
    // a proper value is getting saved.
}

void KgvUnitDoubleSpinBox::privateValueChanged() {
    emit valueChangedPt( value () );
}

void
KgvUnitDoubleSpinBox::setUnit( KgvUnit::Unit unit )
{
    double oldvalue = KgvUnit::fromUserValue( QDoubleSpinBox::value(), m_unit );
    QDoubleSpinBox::setMinimum( KgvUnit::toUserValue( m_lowerInPoints, unit ) );
    QDoubleSpinBox::setMaximum( KgvUnit::toUserValue( m_upperInPoints, unit ) );
    QDoubleSpinBox::setSingleStep( KgvUnit::toUserValue( m_stepInPoints, unit ) );
    QDoubleSpinBox::setValue( KgvUnit::ptToUnit( oldvalue, unit ) );
    m_unit = unit;
    setSuffix( KgvUnit::unitName( unit ).prepend( ' ' ) );
}

double KgvUnitDoubleSpinBox::value( void ) const
{
    return KgvUnit::fromUserValue( QDoubleSpinBox::value(), m_unit );
}

void KgvUnitDoubleSpinBox::setMinValue( double min )
{
  m_lowerInPoints = min;
  QDoubleSpinBox::setMinimum( KgvUnit::toUserValue( m_lowerInPoints, m_unit ) );
}

void KgvUnitDoubleSpinBox::setMaxValue( double max )
{
  m_upperInPoints = max;
  QDoubleSpinBox::setMaximum( KgvUnit::toUserValue( m_upperInPoints, m_unit ) );
}

void KgvUnitDoubleSpinBox::setLineStep( double step )
{
  m_stepInPoints = KgvUnit::toUserValue(step, KgvUnit::U_PT );
  QDoubleSpinBox::setSingleStep( step );
}

void KgvUnitDoubleSpinBox::setLineStepPt( double step )
{
  m_stepInPoints = step;
  QDoubleSpinBox::setSingleStep( KgvUnit::toUserValue( m_stepInPoints, m_unit ) );
}

void KgvUnitDoubleSpinBox::setMinMaxStep( double min, double max, double step )
{
  setMinimum( min );
  setMaximum( max );
  setLineStepPt( step );
}

// ----------------------------------------------------------------


KgvUnitDoubleLineEdit::KgvUnitDoubleLineEdit(QWidget *parent)
    : QLineEdit( parent ), KgvUnitDoubleBase( KgvUnit::U_PT, 2 ), m_value( 0.0 ), m_lower( 0.0 ), m_upper( 9999.99 ),
    m_lowerInPoints( 0.0 ), m_upperInPoints( 9999.99 )
{
    setAlignment( Qt::AlignRight );
    m_validator = new KgvUnitDoubleValidator( this, this );
    setValidator( m_validator );
    setUnit( KgvUnit::U_PT );
    changeValue(  KgvUnit::ptToUnit( 0.0, KgvUnit::U_PT ) );
}

KgvUnitDoubleLineEdit::KgvUnitDoubleLineEdit( QWidget *parent, double lower, double upper, double value, KgvUnit::Unit unit,
    unsigned int precision)
    : QLineEdit( parent ), KgvUnitDoubleBase( unit, precision ), m_value( value ), m_lower( lower ), m_upper( upper ),
    m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    setAlignment( Qt::AlignRight );
    m_validator = new KgvUnitDoubleValidator( this, this );
    setValidator( m_validator );
    setUnit( unit );
    changeValue(  KgvUnit::ptToUnit( value, unit ) );
}

void
KgvUnitDoubleLineEdit::changeValue( double value )
{
    m_value = value < m_lower ? m_lower : ( value > m_upper ? m_upper : value );
    setText( getVisibleText( m_value ) );
}

void
KgvUnitDoubleLineEdit::setUnit( KgvUnit::Unit unit )
{
    KgvUnit::Unit old = m_unit;
    m_unit = unit;
    m_lower = KgvUnit::ptToUnit( m_lowerInPoints, unit );
    m_upper = KgvUnit::ptToUnit( m_upperInPoints, unit );
    changeValue( KgvUnit::ptToUnit( KgvUnit::fromUserValue( m_value, old ), unit ) );
}

bool
KgvUnitDoubleLineEdit::eventFilter( QObject* o, QEvent* ev )
{
#if 0
	if( ev->type() == QEvent::FocusOut || ev->type() == QEvent::Leave || ev->type() == QEvent::Hide )
	{
		bool ok;
		double value = toDouble( text(), &ok );
		changeValue( value );
		return false;
	}
	else
#endif
            return QLineEdit::eventFilter( o, ev );
}

double KgvUnitDoubleLineEdit::value( void ) const
{
    return KgvUnit::fromUserValue( m_value, m_unit );
}


// ----------------------------------------------------------------


KgvUnitDoubleComboBox::KgvUnitDoubleComboBox(QWidget *parent)
     : QComboBox( parent ), KgvUnitDoubleBase( KgvUnit::U_PT, 2 ), m_value( 0.0 ), m_lower( 0.0 ), m_upper( 9999.99 ), m_lowerInPoints( 0.0 ), m_upperInPoints( 9999.99 )
{
    lineEdit()->setAlignment( Qt::AlignRight );
    m_validator = new KgvUnitDoubleValidator( this, this );
    lineEdit()->setValidator( m_validator );
    setUnit( KgvUnit::U_PT );
    changeValue(  KgvUnit::ptToUnit( 0.0, KgvUnit::U_PT ) );
    connect(this, static_cast<void(KgvUnitDoubleComboBox::*)(int)>(&KgvUnitDoubleComboBox::activated),
            this, &KgvUnitDoubleComboBox::slotActivated);
}

KgvUnitDoubleComboBox::KgvUnitDoubleComboBox( QWidget *parent, double lower, double upper, double value, KgvUnit::Unit unit,
     unsigned int precision)
     : QComboBox( parent ), KgvUnitDoubleBase( unit, precision ), m_value( value ), m_lower( lower ), m_upper( upper ),
     m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    lineEdit()->setAlignment( Qt::AlignRight );
    m_validator = new KgvUnitDoubleValidator( this, this );
    lineEdit()->setValidator( m_validator );
    setUnit( unit );
    changeValue(  KgvUnit::ptToUnit( value, unit ) );
    connect(this, static_cast<void(KgvUnitDoubleComboBox::*)(int)>(&KgvUnitDoubleComboBox::activated),
            this, &KgvUnitDoubleComboBox::slotActivated);
}

void
KgvUnitDoubleComboBox::changeValue( double value )
{
    QString oldLabel = lineEdit()->text();
    updateValue( value );
    if( lineEdit()->text() != oldLabel )
        emit valueChanged( m_value );
}

void
KgvUnitDoubleComboBox::updateValue( double value )
{
    m_value = value < m_lower ? m_lower : ( value > m_upper ? m_upper : value );
    lineEdit()->setText( getVisibleText( m_value ) );
}

void
KgvUnitDoubleComboBox::insertItem( double value, int index )
{
    QComboBox::insertItem(index, getVisibleText( value ));
}

void
KgvUnitDoubleComboBox::slotActivated( int index )
{
    double oldvalue = m_value;
    bool ok;
    double value = toDouble( itemText( index ), &ok );
    m_value = value < m_lower ? m_lower : ( value > m_upper ? m_upper : value );
    if( m_value != oldvalue )
        emit valueChanged( m_value );
}

void
KgvUnitDoubleComboBox::setUnit( KgvUnit::Unit unit )
{
    KgvUnit::Unit old = m_unit;
    m_unit = unit;
    m_lower = KgvUnit::ptToUnit( m_lowerInPoints, unit );
    m_upper = KgvUnit::ptToUnit( m_upperInPoints, unit );
    changeValue( KgvUnit::ptToUnit( KgvUnit::fromUserValue( m_value, old ), unit ) );
}

bool
KgvUnitDoubleComboBox::eventFilter( QObject* o, QEvent* ev )
{
#if 0
	if( ev->type() == QEvent::FocusOut || ev->type() == QEvent::Leave || ev->type() == QEvent::Hide )
	{
		bool ok;
		double value = toDouble( lineEdit()->text(), &ok );
		changeValue( value );
		return false;
	}
	else
#endif
            return QComboBox::eventFilter( o, ev );
}

double KgvUnitDoubleComboBox::value( void ) const
{
    return KgvUnit::fromUserValue( m_value, m_unit );
}


// ----------------------------------------------------------------


KgvUnitDoubleSpinComboBox::KgvUnitDoubleSpinComboBox(QWidget *parent)
    : QWidget( parent ), m_step( 1.0 )
{
    QGridLayout *layout = new QGridLayout( this );
    //layout->setMargin( 2 );
    QPushButton *up = new QPushButton( "+", this );
    //up->setFlat( true );
    up->setMaximumHeight( 15 );
    up->setMaximumWidth( 15 );
    layout->addWidget( up, 0, 0 );
    connect(up, &QPushButton::clicked,
            this, &KgvUnitDoubleSpinComboBox::slotUpClicked);

    QPushButton *down = new QPushButton( "-", this );
    down->setMaximumHeight( 15 );
    down->setMaximumWidth( 15 );
    layout->addWidget( down, 1, 0 );
    connect(down, &QPushButton::clicked,
            this, &KgvUnitDoubleSpinComboBox::slotDownClicked);

    m_combo = new KgvUnitDoubleComboBox(this, KgvUnit::ptToUnit(0.0, KgvUnit::U_PT), KgvUnit::ptToUnit(9999.99, KgvUnit::U_PT), 0.0, KgvUnit::U_PT, 2);
    connect(m_combo, &KgvUnitDoubleComboBox::valueChanged,
            this, &KgvUnitDoubleSpinComboBox::valueChanged);
    layout->addWidget( m_combo, 0, 2, 2, 1 );
}

KgvUnitDoubleSpinComboBox::KgvUnitDoubleSpinComboBox( QWidget *parent, double lower, double upper, double step, double value,
                                                    KgvUnit::Unit unit, unsigned int precision)
    : QWidget( parent ), m_step( step )//, m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    QGridLayout *layout = new QGridLayout( this );
    //layout->setMargin( 2 );
    QPushButton *up = new QPushButton( "+", this );
    //up->setFlat( true );
    up->setMaximumHeight( 15 );
    up->setMaximumWidth( 15 );
    layout->addWidget( up, 0, 0 );
    connect(up, &QPushButton::clicked,
            this, &KgvUnitDoubleSpinComboBox::slotUpClicked);

    QPushButton *down = new QPushButton( "-", this );
    down->setMaximumHeight( 15 );
    down->setMaximumWidth( 15 );
    layout->addWidget( down, 1, 0 );
    connect(down, &QPushButton::clicked,
            this, &KgvUnitDoubleSpinComboBox::slotDownClicked);

    m_combo = new KgvUnitDoubleComboBox(this, KgvUnit::ptToUnit(lower, unit), KgvUnit::ptToUnit(upper, unit), value, unit, precision);
    connect(m_combo, &KgvUnitDoubleComboBox::valueChanged,
            this, &KgvUnitDoubleSpinComboBox::valueChanged);
    layout->addWidget( m_combo, 0, 2, 2, 1 );
}

void
KgvUnitDoubleSpinComboBox::slotUpClicked()
{
    m_combo->changeValue( m_combo->value() + m_step );
}

void
KgvUnitDoubleSpinComboBox::slotDownClicked()
{
    m_combo->changeValue( m_combo->value() - m_step );
}

void
KgvUnitDoubleSpinComboBox::insertItem( double value, int index )
{
    m_combo->insertItem( value, index );
}

void
KgvUnitDoubleSpinComboBox::updateValue( double value )
{
    m_combo->updateValue( value );
}

double
KgvUnitDoubleSpinComboBox::value() const
{
    return m_combo->value();
}
