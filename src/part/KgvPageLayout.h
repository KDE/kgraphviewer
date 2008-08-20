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

#ifndef KGVPAGELAYOUT_H
#define KGVPAGELAYOUT_H

// #include <KgvGenStyles.h>
#include <qstringlist.h>

/**
 * @brief Represents the paper format a document shall be printed on.
 *
 * For compatibility reasons, and because of screen and custom,
 * this enum doesn't map to QPrinter::PageSize but KgvPageFormat::printerPageSize
 * does the conversion.
 *
 * @todo convert DIN to ISO in the names
 */
enum KgvFormat {
    PG_DIN_A3 = 0,
    PG_DIN_A4 = 1,
    PG_DIN_A5 = 2,
    PG_US_LETTER = 3,
    PG_US_LEGAL = 4,
    PG_SCREEN = 5,
    PG_CUSTOM = 6,
    PG_DIN_B5 = 7,
    PG_US_EXECUTIVE = 8,
    PG_DIN_A0 = 9,
    PG_DIN_A1 = 10,
    PG_DIN_A2 = 11,
    PG_DIN_A6 = 12,
    PG_DIN_A7 = 13,
    PG_DIN_A8 = 14,
    PG_DIN_A9 = 15,
    PG_DIN_B0 = 16,
    PG_DIN_B1 = 17,
    PG_DIN_B10 = 18,
    PG_DIN_B2 = 19,
    PG_DIN_B3 = 20,
    PG_DIN_B4 = 21,
    PG_DIN_B6 = 22,
    PG_ISO_C5 = 23,
    PG_US_COMM10 = 24,
    PG_ISO_DL = 25,
    PG_US_FOLIO = 26,
    PG_US_LEDGER = 27,
    PG_US_TABLOID = 28,
    // update the number below and the static arrays if you add more values to the enum
    PG_LAST_FORMAT = PG_US_TABLOID // used by koPageLayout.cpp
};

/**
 *  Represents the orientation of a printed document.
 */
enum KgvOrientation {
    PG_PORTRAIT = 0,
    PG_LANDSCAPE = 1
};

namespace KgvPageFormat
{
    /**
     * @brief Convert a KgvFormat into a QPrinter::PageSize.
     *
     * If format is 'screen' it will use A4 landscape.
     * If format is 'custom' it will use A4 portrait.
     * (you may want to take care of those cases separately).
     * Usually passed to QPrinter::setPageSize().
     *
     * @note We return int instead of the enum to avoid including QPrinter
     */
    int /*QPrinter::PageSize*/ printerPageSize( KgvFormat format );

    /**
     * Returns the width (in mm) for a given page format and orientation
     * 'Custom' isn't supported by this function, obviously.
     */
    double width( KgvFormat format, KgvOrientation orientation );

    /**
     * Returns the height (in mm) for a given page format and orientation
     * 'Custom' isn't supported by this function, obviously.
     */
    double height( KgvFormat format, KgvOrientation orientation );

    /**
     * Returns the internal name of the given page format.
     * Use for saving.
     */
    QString formatString( KgvFormat format );

    /**
     * Convert a format string (internal name) to a page format value.
     * Use for loading.
     */
    KgvFormat formatFromString( const QString & string );

    /**
     * Returns the default format (based on the KControl settings)
     */
    KgvFormat defaultFormat();

    /**
     * Returns the translated name of the given page format.
     * Use for showing the user.
     */
    QString name( KgvFormat format );

    /**
     * Lists the translated names of all the available formats
     */
    QStringList allFormats();

    /**
     * Try to find the paper format for the given width and height (in mm).
     * Useful to some import filters.
     */
    KgvFormat guessFormat( double width, double height );
}


/**
 * @brief Header/Footer type.
 *
 * @note Yes, this should have been a bitfield, but there was only 0, 2, 3 in koffice-1.0. Don't ask why.
 * In the long run this should be replaced with a more flexible repetition/section concept.
 */
enum KgvHFType {
    HF_SAME = 0,            ///< 0: Header/Footer is the same on all pages
    HF_FIRST_EO_DIFF = 1,   ///< 1: Header/Footer is different on first, even and odd pages (2&3)
    HF_FIRST_DIFF = 2,      ///< 2: Header/Footer for the first page differs
    HF_EO_DIFF = 3          ///< 3: Header/Footer for even - odd pages are different
};

/**
 * This structure defines the page layout, including
 * its size in pt, its format (e.g. A4), orientation, unit, margins etc.
 */
struct KgvPageLayout
{
    /** Page format */
    KgvFormat format;
    /** Page orientation */
    KgvOrientation orientation;

    /** Page width in pt */
    double ptWidth;
    /** Page height in pt */
    double ptHeight;
    /** Left margin in pt */
    double ptLeft;
    /** Right margin in pt */
    double ptRight;
    /** Top margin in pt */
    double ptTop;
    /** Bottom margin in pt */
    double ptBottom;
    double ptPageEdge;
    double ptBindingSide;

    bool operator==( const KgvPageLayout& l ) const {
       return ( ptWidth == l.ptWidth &&
                ptHeight == l.ptHeight &&
                ptLeft == l.ptLeft &&
                ptRight == l.ptRight &&
                ptTop == l.ptTop &&
                ptBottom == l.ptBottom &&
                ptPageEdge == l.ptPageEdge &&
                ptBindingSide == l.ptBindingSide);
    }
    bool operator!=( const KgvPageLayout& l ) const {
        return !( (*this) == l );
    }

    /**
     * @return a page layout with the default page size depending on the locale settings,
     * default margins (2 cm), and portrait orientation.
     * @since 1.4
     */
    static KgvPageLayout standardLayout();
};

/** structure for header-footer */
struct KgvHeadFoot
{
    QString headLeft;
    QString headMid;
    QString headRight;
    QString footLeft;
    QString footMid;
    QString footRight;
};

/** structure for columns */
struct KgvColumns
{
    int columns;
    double ptColumnSpacing;
    bool operator==( const KgvColumns& rhs ) const {
        return columns == rhs.columns &&
               qAbs(ptColumnSpacing - rhs.ptColumnSpacing) <= 1E-10;
    }
    bool operator!=( const KgvColumns& rhs ) const {
        return columns != rhs.columns ||
               qAbs(ptColumnSpacing - rhs.ptColumnSpacing) > 1E-10;
    }
};

/** structure for KWord header-footer */
struct KgvKWHeaderFooter
{
    KgvHFType header;
    KgvHFType footer;
    double ptHeaderBodySpacing;
    double ptFooterBodySpacing;
    double ptFootNoteBodySpacing;
    bool operator==( const KgvKWHeaderFooter& rhs ) const {
        return header == rhs.header && footer == rhs.footer &&
               qAbs(ptHeaderBodySpacing - rhs.ptHeaderBodySpacing) <= 1E-10 &&
               qAbs(ptFooterBodySpacing - rhs.ptFooterBodySpacing) <= 1E-10 &&
               qAbs(ptFootNoteBodySpacing - rhs.ptFootNoteBodySpacing) <= 1E-10;
    }
    bool operator!=( const KgvKWHeaderFooter& rhs ) const {
        return !( *this == rhs );
    }
};

#endif /* KOPAGELAYOUT_H */

