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

#include "KgvPageLayout.h"
#include <KgvUnit.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <QtGui/QPrinter>
#include <qdom.h>

// paper formats ( mm )
#define PG_A3_WIDTH		297.0
#define PG_A3_HEIGHT		420.0
#define PG_A4_WIDTH		210.0
#define PG_A4_HEIGHT		297.0
#define PG_A5_WIDTH		148.0
#define PG_A5_HEIGHT		210.0
#define PG_B5_WIDTH		182.0
#define PG_B5_HEIGHT		257.0
#define PG_US_LETTER_WIDTH	216.0
#define PG_US_LETTER_HEIGHT	279.0
#define PG_US_LEGAL_WIDTH	216.0
#define PG_US_LEGAL_HEIGHT	356.0
#define PG_US_EXECUTIVE_WIDTH	191.0
#define PG_US_EXECUTIVE_HEIGHT	254.0

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
    kDebug() << "Returning standardLayout";
    return layout;
}

struct PageFormatInfo
{
    KgvFormat format;
    QPrinter::PageSize kprinter;
    const char* shortName; // Short name
    const char* descriptiveName; // Full name, which will be translated
    double width; // in mm
    double height; // in mm
};

// NOTES:
// - the width and height of non-ISO formats are rounded
// http://en.wikipedia.org/wiki/Paper_size can help
// - the comments "should be..." indicates the exact values if the inch sizes would be multiplied by 25.4 mm/inch

const PageFormatInfo pageFormatInfo[]=
{
    { PG_DIN_A3,        QPrinter::A3,           "A3",           I18N_NOOP2("Page size", "ISO A3"),       297.0,  420.0 },
    { PG_DIN_A4,        QPrinter::A4,           "A4",           I18N_NOOP2("Page size", "ISO A4"),       210.0,  297.0 },
    { PG_DIN_A5,        QPrinter::A5,           "A5",           I18N_NOOP2("Page size", "ISO A5"),       148.0,  210.0 },
    { PG_US_LETTER,     QPrinter::Letter,       "Letter",       I18N_NOOP2("Page size", "US Letter"),    215.9,  279.4 },
    { PG_US_LEGAL,      QPrinter::Legal,        "Legal",        I18N_NOOP2("Page size", "US Legal"),     215.9,  355.6 },
    { PG_SCREEN,        QPrinter::A4,           "Screen",       I18N_NOOP2("Page size", "Screen"), PG_A4_HEIGHT, PG_A4_WIDTH }, // Custom, so fall back to A4
    { PG_CUSTOM,        QPrinter::A4,           "Custom",       I18N_NOOP2("Page size", "Custom"), PG_A4_WIDTH, PG_A4_HEIGHT }, // Custom, so fall back to A4
    { PG_DIN_B5,        QPrinter::B5,           "B5",           I18N_NOOP2("Page size", "ISO B5"),       182.0,  257.0 },
    // Hmm, wikipedia says 184.15 * 266.7 for executive !
    { PG_US_EXECUTIVE,  QPrinter::Executive,    "Executive",    I18N_NOOP2("Page size", "US Executive"), 191.0,  254.0 }, // should be 190.5 mm x 254.0 mm
    { PG_DIN_A0,        QPrinter::A0,           "A0",           I18N_NOOP2("Page size", "ISO A0"),       841.0, 1189.0 },
    { PG_DIN_A1,        QPrinter::A1,           "A1",           I18N_NOOP2("Page size", "ISO A1"),       594.0,  841.0 },
    { PG_DIN_A2,        QPrinter::A2,           "A2",           I18N_NOOP2("Page size", "ISO A2"),       420.0,  594.0 },
    { PG_DIN_A6,        QPrinter::A6,           "A6",           I18N_NOOP2("Page size", "ISO A6"),       105.0,  148.0 },
    { PG_DIN_A7,        QPrinter::A7,           "A7",           I18N_NOOP2("Page size", "ISO A7"),        74.0,  105.0 },
    { PG_DIN_A8,        QPrinter::A8,           "A8",           I18N_NOOP2("Page size", "ISO A8"),        52.0,   74.0 },
    { PG_DIN_A9,        QPrinter::A9,           "A9",           I18N_NOOP2("Page size", "ISO A9"),        37.0,   52.0 },
    { PG_DIN_B0,        QPrinter::B0,           "B0",           I18N_NOOP2("Page size", "ISO B0"),      1030.0, 1456.0 },
    { PG_DIN_B1,        QPrinter::B1,           "B1",           I18N_NOOP2("Page size", "ISO B1"),       728.0, 1030.0 },
    { PG_DIN_B10,       QPrinter::B10,          "B10",          I18N_NOOP2("Page size", "ISO B10"),       32.0,   45.0 },
    { PG_DIN_B2,        QPrinter::B2,           "B2",           I18N_NOOP2("Page size", "ISO B2"),       515.0,  728.0 },
    { PG_DIN_B3,        QPrinter::B3,           "B3",           I18N_NOOP2("Page size", "ISO B3"),       364.0,  515.0 },
    { PG_DIN_B4,        QPrinter::B4,           "B4",           I18N_NOOP2("Page size", "ISO B4"),       257.0,  364.0 },
    { PG_DIN_B6,        QPrinter::B6,           "B6",           I18N_NOOP2("Page size", "ISO B6"),       128.0,  182.0 },
    { PG_ISO_C5,        QPrinter::C5E,          "C5",           I18N_NOOP2("Page size", "ISO C5"),       163.0,  229.0 }, // Some sources tells: 162 mm x 228 mm
    { PG_US_COMM10,     QPrinter::Comm10E,      "Comm10",       I18N_NOOP2("Page size", "US Common 10"), 105.0,  241.0 }, // should be 104.775 mm x 241.3 mm
    { PG_ISO_DL,        QPrinter::DLE,          "DL",           I18N_NOOP2("Page size", "ISO DL"),       110.0,  220.0 },
    { PG_US_FOLIO,      QPrinter::Folio,        "Folio",        I18N_NOOP2("Page size", "US Folio"),     210.0,  330.0 }, // should be 209.54 mm x 330.2 mm
    { PG_US_LEDGER,     QPrinter::Ledger,       "Ledger",       I18N_NOOP2("Page size", "US Ledger"),    432.0,  279.0 }, // should be 431.8 mm x 297.4 mm
    { PG_US_TABLOID,    QPrinter::Tabloid,      "Tabloid",      I18N_NOOP2("Page size", "US Tabloid"),   279.0,  432.0 }  // should be 297.4 mm x 431.8 mm
};

int KgvPageFormat::printerPageSize( KgvFormat format )
{
    if ( format == PG_SCREEN )
    {
            kWarning() << "You use the page layout SCREEN. Printing in DIN A4 LANDSCAPE.";
            return QPrinter::A4;
    }
    else if ( format == PG_CUSTOM )
    {
            kWarning() << "The used page layout (CUSTOM) is not supported by QPrinter. Printing in A4.";
            return QPrinter::A4;
    }
    else if ( format <= PG_LAST_FORMAT )
        return pageFormatInfo[ format ].kprinter;
    else
        return QPrinter::A4;
}

double KgvPageFormat::width( KgvFormat format, KgvOrientation orientation )
{
    if ( orientation == PG_LANDSCAPE )
        return height( format, PG_PORTRAIT );
    if ( format <= PG_LAST_FORMAT )
        return pageFormatInfo[ format ].width;
    return PG_A4_WIDTH;   // should never happen
}

double KgvPageFormat::height( KgvFormat format, KgvOrientation orientation )
{
    if ( orientation == PG_LANDSCAPE )
        return width( format, PG_PORTRAIT );
    if ( format <= PG_LAST_FORMAT )
        return pageFormatInfo[ format ].height;
    return PG_A4_HEIGHT;
}

KgvFormat KgvPageFormat::guessFormat( double width, double height )
{
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        // We have some tolerance. 1pt is a third of a mm, this is
        // barely noticeable for a page size.
        if ( i != PG_CUSTOM
             && qAbs( width - pageFormatInfo[i].width ) < 1.0
             && qAbs( height - pageFormatInfo[i].height ) < 1.0 )
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
    int kprinter = KGlobal::locale()->pageSize();
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        if ( pageFormatInfo[ i ].kprinter == kprinter )
            return static_cast<KgvFormat>(i);
    }
    return PG_DIN_A4;
}

QString KgvPageFormat::name( KgvFormat format )
{
    if ( format <= PG_LAST_FORMAT )
        return i18nc( "Page size", pageFormatInfo[ format ].descriptiveName );
    return i18nc( "Page size", pageFormatInfo[ PG_DIN_A4 ].descriptiveName );
}

QStringList KgvPageFormat::allFormats()
{
    QStringList lst;
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        lst << i18nc( "Page size", pageFormatInfo[ i ].descriptiveName );
    }
    return lst;
}
