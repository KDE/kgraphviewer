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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/* This file was part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

//#include <KoGlobal.h>
#include "KgvUnit.h"

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include <qregexp.h>

QStringList KgvUnit::listOfUnitName()
{
    QStringList lst;
    for ( uint i = 0 ; i <= KgvUnit::U_LASTUNIT ; ++i )
    {
        KgvUnit::Unit unit = static_cast<KgvUnit::Unit>( i );
        lst.append( KgvUnit::unitDescription( unit ) );
    }
    return lst;
}

QString KgvUnit::unitDescription( Unit _unit )
{
    switch ( _unit )
    {
    case KgvUnit::U_MM:
        return i18n("Millimeters (mm)");
    case KgvUnit::U_CM:
        return i18n("Centimeters (cm)");
    case KgvUnit::U_DM:
        return i18n("Decimeters (dm)");
    case KgvUnit::U_INCH:
        return i18n("Inches (in)");
    case KgvUnit::U_PI:
        return i18n("Pica (pi)");
    case KgvUnit::U_DD:
        return i18n("Didot (dd)");
    case KgvUnit::U_CC:
        return i18n("Cicero (cc)");
    case KgvUnit::U_PT:
        return i18n("Points (pt)" );
    default:
        return i18n("Error!");
    }
}

double KgvUnit::toUserValue( double ptValue, Unit unit )
{
    switch ( unit ) {
    case U_MM:
        return toMM( ptValue );
    case U_CM:
        return toCM( ptValue );
    case U_DM:
        return toDM( ptValue );
    case U_INCH:
        return toInch( ptValue );
    case U_PI:
        return toPI( ptValue );
    case U_DD:
        return toDD( ptValue );
    case U_CC:
        return toCC( ptValue );
    case U_PT:
    default:
        return toPoint( ptValue );
    }
}

double KgvUnit::ptToUnit( const double ptValue, const Unit unit )
{
    switch ( unit )
    {
    case U_MM:
        return POINT_TO_MM( ptValue );
    case U_CM:
        return POINT_TO_CM( ptValue );
    case U_DM:
        return POINT_TO_DM( ptValue );
    case U_INCH:
        return POINT_TO_INCH( ptValue );
    case U_PI:
        return POINT_TO_PI( ptValue );
    case U_DD:
        return POINT_TO_DD( ptValue );
    case U_CC:
        return POINT_TO_CC( ptValue );
    case U_PT:
    default:
        return ptValue;
    }
}

QString KgvUnit::toUserStringValue( double ptValue, Unit unit )
{
    return KGlobal::locale()->formatNumber( toUserValue( ptValue, unit ) );
}

double KgvUnit::fromUserValue( double value, Unit unit )
{
    switch ( unit ) {
    case U_MM:
        return MM_TO_POINT( value );
    case U_CM:
        return CM_TO_POINT( value );
    case U_DM:
        return DM_TO_POINT( value );
    case U_INCH:
        return INCH_TO_POINT( value );
    case U_PI:
        return PI_TO_POINT( value );
    case U_DD:
        return DD_TO_POINT( value );
    case U_CC:
        return CC_TO_POINT( value );
    case U_PT:
    default:
        return value;
    }
}

double KgvUnit::fromUserValue( const QString& value, Unit unit, bool* ok )
{
    return fromUserValue( KGlobal::locale()->readNumber( value, ok ), unit );
}

double KgvUnit::parseValue( QString value, double defaultVal )
{
    value.simplifyWhiteSpace();
    value.remove( ' ' );

    if( value.isEmpty() )
        return defaultVal;

    int index = value.find( QRegExp( "[a-z]+$" ) );
    if ( index == -1 )
        return value.toDouble();

    QString unit = value.mid( index );
    value.truncate ( index );
    double val = value.toDouble();

    if ( unit == "pt" )
        return val;

    bool ok;
    Unit u = KgvUnit::unit( unit, &ok );
    if( ok )
        return fromUserValue( val, u );

    if( unit == "m" )
        return fromUserValue( val * 10.0, U_DM );
    else if( unit == "km" )
        return fromUserValue( val * 10000.0, U_DM );
    kdWarning() << "KgvUnit::parseValue: Unit " << unit << " is not supported, please report." << endl;

    // TODO : add support for mi/ft ?
    return defaultVal;
}

KgvUnit::Unit KgvUnit::unit( const QString &_unitName, bool* ok )
{
    if ( ok )
        *ok = true;
    if ( _unitName == QString::fromLatin1( "mm" ) ) return U_MM;
    if ( _unitName == QString::fromLatin1( "cm" ) ) return U_CM;
    if ( _unitName == QString::fromLatin1( "dm" ) ) return U_DM;
    if ( _unitName == QString::fromLatin1( "in" )
         || _unitName == QString::fromLatin1("inch") /*compat*/ ) return U_INCH;
    if ( _unitName == QString::fromLatin1( "pi" ) ) return U_PI;
    if ( _unitName == QString::fromLatin1( "dd" ) ) return U_DD;
    if ( _unitName == QString::fromLatin1( "cc" ) ) return U_CC;
    if ( _unitName == QString::fromLatin1( "pt" ) ) return U_PT;
    if ( ok )
        *ok = false;
    return U_PT;
}

QString KgvUnit::unitName( Unit _unit )
{
    if ( _unit == U_MM ) return QString::fromLatin1( "mm" );
    if ( _unit == U_CM ) return QString::fromLatin1( "cm" );
    if ( _unit == U_DM ) return QString::fromLatin1( "dm" );
    if ( _unit == U_INCH ) return QString::fromLatin1( "in" );
    if ( _unit == U_PI ) return QString::fromLatin1( "pi" );
    if ( _unit == U_DD ) return QString::fromLatin1( "dd" );
    if ( _unit == U_CC ) return QString::fromLatin1( "cc" );
    return QString::fromLatin1( "pt" );
}

