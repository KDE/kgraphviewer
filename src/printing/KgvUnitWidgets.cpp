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

#include "KgvUnitWidgets.h"
#include "KgvUnitWidgets.moc"
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qlayout.h>


// ----------------------------------------------------------------
//                          Support classes


KgvUnitDoubleValidator::KgvUnitDoubleValidator( KgvUnitDoubleBase *base, QObject *parent, const char *name )
: KDoubleValidator( parent, name ), m_base( base )
{
}

QValidator::State
KgvUnitDoubleValidator::validate( QString &s, int &pos ) const
{

    kdDebug(30004) << "KgvUnitDoubleValidator::validate : " << s << " at " << pos << endl;
    QValidator::State result = Acceptable;

    QRegExp regexp ("([ a-zA-Z]+)$"); // Letters or spaces at end
    const int res = regexp.search( s );

    if ( res == -1 )
    {
        // Nothing like an unit? The user is probably editing the unit
        kdDebug(30004) << "Intermediate (no unit)" << endl;
        return Intermediate;
    }

    // ### TODO: are all the QString::stripWhiteSpace really necessary?
    const QString number ( s.left( res ).stripWhiteSpace() );
    const QString unitName ( regexp.cap( 1 ).stripWhiteSpace().lower() );

    kdDebug(30004) << "Split:" << number << ":" << unitName << ":" << endl;

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
            kdDebug(30004) << "Intermediate (unknown unit)" << endl;
            return Intermediate;
        }
    }
    else
    {
        kdWarning(30004) << "Not a number: " << number << endl;
        return Invalid;
    }

    newVal = KgvUnit::ptToUnit( newVal, m_base->m_unit );

    s = m_base->getVisibleText( newVal );

    return result;
}


QString KgvUnitDoubleBase::getVisibleText( double value ) const
{
    const QString num ( QString( "%1%2").arg( KGlobal::locale()->formatNumber( value, m_precision ), KgvUnit::unitName( m_unit ) ) );
    kdDebug(30004) << "getVisibleText: " << QString::number( value, 'f', 12 ) << " => " << num << endl;
    return num;
}

double KgvUnitDoubleBase::toDouble( const QString& str, bool* ok ) const
{
    QString str2( str );
    /* KLocale::readNumber wants the thousand separator exactly at 1000.
       But when editing, it might be anywhere. So we need to remove it. */
    const QString sep( KGlobal::locale()->thousandsSeparator() );
    if ( !sep.isEmpty() )
        str2.remove( sep );
    str2.remove( KgvUnit::unitName( m_unit ) );
    const double dbl = KGlobal::locale()->readNumber( str2, ok );
    if ( ok )
      kdDebug(30004) << "toDouble:" << str << ": => :" << str2 << ": => " << QString::number( dbl, 'f', 12 ) << endl;
    else
        kdWarning(30004) << "toDouble error:" << str << ": => :" << str2 << ":" << endl;
    return dbl;
}


// ----------------------------------------------------------------
//                          Widget classes


KgvUnitDoubleSpinBox::KgvUnitDoubleSpinBox( QWidget *parent, const char *name )
    : KDoubleSpinBox( parent, name ), KgvUnitDoubleBase( KgvUnit::U_PT, 2 )
    , m_lowerInPoints( -9999 )
    , m_upperInPoints( 9999 )
    , m_stepInPoints( 1 )
{
    KDoubleSpinBox::setPrecision( 2 );
    m_validator = new KgvUnitDoubleValidator( this, this );
    QSpinBox::setValidator( m_validator );
    setAcceptLocalizedNumbers( true );
    setUnit( KgvUnit::U_PT );

    connect(this, SIGNAL(valueChanged( double )), SLOT(privateValueChanged()));
}


KgvUnitDoubleSpinBox::KgvUnitDoubleSpinBox( QWidget *parent, 
						    double lower, double upper,
						    double step, 
						    double value, 
						    KgvUnit::Unit unit, 
						    unsigned int precision, 
						    const char *name )
    : KDoubleSpinBox( lower, upper, step, value, precision, parent, name ),
      KgvUnitDoubleBase( unit, precision ),
    m_lowerInPoints( lower ), m_upperInPoints( upper ), m_stepInPoints( step )
{
    m_unit = KgvUnit::U_PT;
    m_validator = new KgvUnitDoubleValidator( this, this );
    QSpinBox::setValidator( m_validator );
    setAcceptLocalizedNumbers( true );
    setUnit( unit );
    changeValue( value );
    setLineStep( 0.5 );

    connect(this, SIGNAL(valueChanged( double )), SLOT(privateValueChanged()));
}

void
KgvUnitDoubleSpinBox::changeValue( double val )
{
    KDoubleSpinBox::setValue( KgvUnit::toUserValue( val, m_unit ) );
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
    double oldvalue = KgvUnit::fromUserValue( KDoubleSpinBox::value(), m_unit );
    KDoubleSpinBox::setMinValue( KgvUnit::toUserValue( m_lowerInPoints, unit ) );
    KDoubleSpinBox::setMaxValue( KgvUnit::toUserValue( m_upperInPoints, unit ) );
    KDoubleSpinBox::setLineStep( KgvUnit::toUserValue( m_stepInPoints, unit ) );
    KDoubleSpinBox::setValue( KgvUnit::ptToUnit( oldvalue, unit ) );
    m_unit = unit;
    setSuffix( KgvUnit::unitName( unit ).prepend( ' ' ) );
}

double KgvUnitDoubleSpinBox::value( void ) const
{
    return KgvUnit::fromUserValue( KDoubleSpinBox::value(), m_unit );
}

void KgvUnitDoubleSpinBox::setMinValue( double min )
{
  m_lowerInPoints = min;
  KDoubleSpinBox::setMinValue( KgvUnit::toUserValue( m_lowerInPoints, m_unit ) );
}

void KgvUnitDoubleSpinBox::setMaxValue( double max )
{
  m_upperInPoints = max;
  KDoubleSpinBox::setMaxValue( KgvUnit::toUserValue( m_upperInPoints, m_unit ) );
}

void KgvUnitDoubleSpinBox::setLineStep( double step )
{
  m_stepInPoints = KgvUnit::toUserValue(step, KgvUnit::U_PT );
  KDoubleSpinBox::setLineStep( step );
}

void KgvUnitDoubleSpinBox::setLineStepPt( double step )
{
  m_stepInPoints = step;
  KDoubleSpinBox::setLineStep( KgvUnit::toUserValue( m_stepInPoints, m_unit ) );
}

void KgvUnitDoubleSpinBox::setMinMaxStep( double min, double max, double step )
{
  setMinValue( min );
  setMaxValue( max );
  setLineStepPt( step );
}

// ----------------------------------------------------------------


KgvUnitDoubleLineEdit::KgvUnitDoubleLineEdit( QWidget *parent, const char *name )
    : KLineEdit( parent, name ), KgvUnitDoubleBase( KgvUnit::U_PT, 2 ), m_value( 0.0 ), m_lower( 0.0 ), m_upper( 9999.99 ),
    m_lowerInPoints( 0.0 ), m_upperInPoints( 9999.99 )
{
    setAlignment( Qt::AlignRight );
    m_validator = new KgvUnitDoubleValidator( this, this );
    setValidator( m_validator );
    setUnit( KgvUnit::U_PT );
    changeValue(  KgvUnit::ptToUnit( 0.0, KgvUnit::U_PT ) );
}

KgvUnitDoubleLineEdit::KgvUnitDoubleLineEdit( QWidget *parent, double lower, double upper, double value, KgvUnit::Unit unit,
    unsigned int precision, const char *name )
    : KLineEdit( parent, name ), KgvUnitDoubleBase( unit, precision ), m_value( value ), m_lower( lower ), m_upper( upper ),
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


KgvUnitDoubleComboBox::KgvUnitDoubleComboBox( QWidget *parent, const char *name )
     : KComboBox( true, parent, name ), KgvUnitDoubleBase( KgvUnit::U_PT, 2 ), m_value( 0.0 ), m_lower( 0.0 ), m_upper( 9999.99 ), m_lowerInPoints( 0.0 ), m_upperInPoints( 9999.99 )
{
    lineEdit()->setAlignment( Qt::AlignRight );
    m_validator = new KgvUnitDoubleValidator( this, this );
    lineEdit()->setValidator( m_validator );
    setUnit( KgvUnit::U_PT );
    changeValue(  KgvUnit::ptToUnit( 0.0, KgvUnit::U_PT ) );
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
}

KgvUnitDoubleComboBox::KgvUnitDoubleComboBox( QWidget *parent, double lower, double upper, double value, KgvUnit::Unit unit,
     unsigned int precision, const char *name )
     : KComboBox( true, parent, name ), KgvUnitDoubleBase( unit, precision ), m_value( value ), m_lower( lower ), m_upper( upper ),
     m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    lineEdit()->setAlignment( Qt::AlignRight );
    m_validator = new KgvUnitDoubleValidator( this, this );
    lineEdit()->setValidator( m_validator );
    setUnit( unit );
    changeValue(  KgvUnit::ptToUnit( value, unit ) );
    connect( this, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
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
    KComboBox::insertItem( getVisibleText( value ), index );
}

void
KgvUnitDoubleComboBox::slotActivated( int index )
{
    double oldvalue = m_value;
    bool ok;
    double value = toDouble( text( index ), &ok );
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


KgvUnitDoubleSpinComboBox::KgvUnitDoubleSpinComboBox( QWidget *parent, const char *name )
    : QWidget( parent ), m_step( 1.0 )
{
    QGridLayout *layout = new QGridLayout( this, 2, 3 );
    //layout->setMargin( 2 );
    QPushButton *up = new QPushButton( "+", this );
    //up->setFlat( true );
    up->setMaximumHeight( 15 );
    up->setMaximumWidth( 15 );
    layout->addWidget( up, 0, 0 );
    connect( up, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );

    QPushButton *down = new QPushButton( "-", this );
    down->setMaximumHeight( 15 );
    down->setMaximumWidth( 15 );
    layout->addWidget( down, 1, 0 );
    connect( down, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );

    m_combo = new KgvUnitDoubleComboBox( this, KgvUnit::ptToUnit( 0.0, KgvUnit::U_PT ), KgvUnit::ptToUnit( 9999.99, KgvUnit::U_PT ), 0.0, KgvUnit::U_PT, 2, name );
    connect( m_combo, SIGNAL( valueChanged( double ) ), this, SIGNAL( valueChanged( double ) ) );
    layout->addMultiCellWidget( m_combo, 0, 1, 2, 2 );
}

KgvUnitDoubleSpinComboBox::KgvUnitDoubleSpinComboBox( QWidget *parent, double lower, double upper, double step, double value,
                                                    KgvUnit::Unit unit, unsigned int precision, const char *name )
    : QWidget( parent ), m_step( step )//, m_lowerInPoints( lower ), m_upperInPoints( upper )
{
    QGridLayout *layout = new QGridLayout( this, 2, 3 );
    //layout->setMargin( 2 );
    QPushButton *up = new QPushButton( "+", this );
    //up->setFlat( true );
    up->setMaximumHeight( 15 );
    up->setMaximumWidth( 15 );
    layout->addWidget( up, 0, 0 );
    connect( up, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );

    QPushButton *down = new QPushButton( "-", this );
    down->setMaximumHeight( 15 );
    down->setMaximumWidth( 15 );
    layout->addWidget( down, 1, 0 );
    connect( down, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );

    m_combo = new KgvUnitDoubleComboBox( this, KgvUnit::ptToUnit( lower, unit ), KgvUnit::ptToUnit( upper, unit ), value, unit, precision, name );
    connect( m_combo, SIGNAL( valueChanged( double ) ), this, SIGNAL( valueChanged( double ) ) );
    layout->addMultiCellWidget( m_combo, 0, 1, 2, 2 );
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

