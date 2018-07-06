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
#include "KgvPageLayout.h"

#include <KgvUnit.h>
#include <QDebug>
#include <KSharedConfig>
#include <QPageSize>
#include <QPrinterInfo>
#include <qdom.h>
#include <klocalizedstring.h>

KgvPageLayout KgvPageLayout::standardLayout()
{
    KgvPageLayout layout;
    layout.format = KgvPageFormat::defaultFormat();
    layout.orientation = PG_PORTRAIT;
    layout.ptWidth = MM_TO_POINT( KgvPageFormat::width( layout.format, layout.orientation ) );
    layout.ptHeight = MM_TO_POINT( KgvPageFormat::height( layout.format, layout.orientation ) );
    layout.ptLeft = MM_TO_POINT( 20.0 );
    layout.ptRight = MM_TO_POINT( 20.0 );
    layout.ptTop = MM_TO_POINT( 20.0 );
    layout.ptBottom = MM_TO_POINT( 20.0 );
    layout.ptPageEdge = -1;
    layout.ptBindingSide = -1;
    qCDebug(KGRAPHVIEWERLIB_LOG) << "Returning standardLayout";
    return layout;
}

struct PageFormatInfo
{
    KgvFormat format;
    QPageSize::PageSizeId pageSize;
    const char* shortName; // Short name
    const char* descriptiveName; // Full name, which will be translated; nullptr to use QPageSize
};

const PageFormatInfo pageFormatInfo[]=
{
    { PG_DIN_A3,        QPageSize::A3,          "A3",           nullptr },
    { PG_DIN_A4,        QPageSize::A4,          "A4",           nullptr },
    { PG_DIN_A5,        QPageSize::A5,          "A5",           nullptr },
    { PG_US_LETTER,     QPageSize::Letter,      "Letter",       nullptr },
    { PG_US_LEGAL,      QPageSize::Legal,       "Legal",        nullptr },
    { PG_SCREEN,        QPageSize::A4,          "Screen",       I18N_NOOP2("Page size", "Screen") }, // Custom, so fall back to A4
    { PG_CUSTOM,        QPageSize::A4,          "Custom",       I18N_NOOP2("Page size", "Custom") }, // Custom, so fall back to A4
    { PG_DIN_B5,        QPageSize::B5,          "B5",           nullptr },
    { PG_US_EXECUTIVE,  QPageSize::Executive,   "Executive",    nullptr },
    { PG_DIN_A0,        QPageSize::A0,          "A0",           nullptr },
    { PG_DIN_A1,        QPageSize::A1,          "A1",           nullptr },
    { PG_DIN_A2,        QPageSize::A2,          "A2",           nullptr },
    { PG_DIN_A6,        QPageSize::A6,          "A6",           nullptr },
    { PG_DIN_A7,        QPageSize::A7,          "A7",           nullptr },
    { PG_DIN_A8,        QPageSize::A8,          "A8",           nullptr },
    { PG_DIN_A9,        QPageSize::A9,          "A9",           nullptr },
    { PG_DIN_B0,        QPageSize::B0,          "B0",           nullptr },
    { PG_DIN_B1,        QPageSize::B1,          "B1",           nullptr },
    { PG_DIN_B10,       QPageSize::B10,         "B10",          nullptr },
    { PG_DIN_B2,        QPageSize::B2,          "B2",           nullptr },
    { PG_DIN_B3,        QPageSize::B3,          "B3",           nullptr },
    { PG_DIN_B4,        QPageSize::B4,          "B4",           nullptr },
    { PG_DIN_B6,        QPageSize::B6,          "B6",           nullptr },
    { PG_ISO_C5,        QPageSize::C5E,         "C5",           nullptr },
    { PG_US_COMM10,     QPageSize::Comm10E,     "Comm10",       nullptr },
    { PG_ISO_DL,        QPageSize::DLE,         "DL",           nullptr },
    { PG_US_FOLIO,      QPageSize::Folio,       "Folio",        nullptr },
    { PG_US_LEDGER,     QPageSize::Ledger,      "Ledger",       nullptr },
    { PG_US_TABLOID,    QPageSize::Tabloid,     "Tabloid",      nullptr }
};

int KgvPageFormat::printerPageSize( KgvFormat format )
{
    if ( format == PG_SCREEN )
    {
            qCWarning(KGRAPHVIEWERLIB_LOG) << "You use the page layout SCREEN. Printing in DIN A4 LANDSCAPE.";
            return QPageSize::A4;
    }
    else if ( format == PG_CUSTOM )
    {
            qCWarning(KGRAPHVIEWERLIB_LOG) << "The used page layout (CUSTOM) is not supported by QPrinter. Printing in A4.";
            return QPageSize::A4;
    }
    else if ( format <= PG_LAST_FORMAT )
        return pageFormatInfo[ format ].pageSize;
    else
        return QPageSize::A4;
}

double KgvPageFormat::width( KgvFormat format, KgvOrientation orientation )
{
    if ( orientation == PG_LANDSCAPE )
        return height( format, PG_PORTRAIT );
    if ( format <= PG_LAST_FORMAT )
        return QPageSize::size( pageFormatInfo[ format ].pageSize, QPageSize::Millimeter ).width();
    return QPageSize::size( QPageSize::A4, QPageSize::Millimeter ).width();   // should never happen
}

double KgvPageFormat::height( KgvFormat format, KgvOrientation orientation )
{
    if ( orientation == PG_LANDSCAPE )
        return width( format, PG_PORTRAIT );
    if ( format <= PG_LAST_FORMAT )
        return QPageSize::size( pageFormatInfo[ format ].pageSize, QPageSize::Millimeter ).height();
    return QPageSize::size( QPageSize::A4, QPageSize::Millimeter ).height();
}

KgvFormat KgvPageFormat::guessFormat( double width, double height )
{
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        const QSizeF ps = QPageSize::size( pageFormatInfo[i].pageSize, QPageSize::Millimeter );
        // We have some tolerance. 1pt is a third of a mm, this is
        // barely noticeable for a page size.
        if ( i != PG_CUSTOM
             && qAbs( width - ps.width() ) < 1.0
             && qAbs( height - ps.height() ) < 1.0 )
            return static_cast<KgvFormat>(i);
    }
    return PG_CUSTOM;
}

QString KgvPageFormat::formatString( KgvFormat format )
{
    if ( format <= PG_LAST_FORMAT )
        return QString::fromLatin1( pageFormatInfo[ format ].shortName );
    return QString::fromLatin1( "A4" );
}

KgvFormat KgvPageFormat::formatFromString( const QString & string )
{
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        if (string == QString::fromLatin1( pageFormatInfo[ i ].shortName ))
            return pageFormatInfo[ i ].format;
    }
    // We do not know the format name, so we have a custom format
    return PG_CUSTOM;
}

KgvFormat KgvPageFormat::defaultFormat()
{
    const QPageSize::PageSizeId pageSize = QPrinterInfo::defaultPrinter().defaultPageSize().id();
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        if ( pageFormatInfo[ i ].pageSize == pageSize )
            return static_cast<KgvFormat>(i);
    }
    return PG_DIN_A4;
}

QString KgvPageFormat::name( KgvFormat format )
{
    if ( format <= PG_LAST_FORMAT )
    {
        if ( pageFormatInfo[ format ].descriptiveName )
            return i18nc( "Page size", pageFormatInfo[ format ].descriptiveName );
        else
            return QPageSize::name( pageFormatInfo[ format ].pageSize );
    }
    return QPageSize::name( pageFormatInfo[ PG_DIN_A4 ].pageSize );
}

QStringList KgvPageFormat::allFormats()
{
    QStringList lst;
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        if ( pageFormatInfo[ i ].descriptiveName )
            lst << i18nc( "Page size", pageFormatInfo[ i ].descriptiveName );
        else
            lst << QPageSize::name( pageFormatInfo[ i ].pageSize );
    }
    return lst;
}
