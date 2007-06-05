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

// Description: Page Layout Dialog (header)

#ifndef __KGVPGLAYOUTDIA_H__
#define __KGVPGLAYOUTDIA_H__

#include <qgroupbox.h>
#include <KgvUnit.h>
#include <kdialogbase.h>
#include <KgvPageLayout.h>

class QButtonGroup;
class QComboBox;
class QLineEdit;
class QCheckBox;
class KgvUnitDoubleSpinBox;
class KgvPageLayoutColumns;
class KgvPageLayoutSize;
class KgvPageLayoutHeader;

enum { FORMAT_AND_BORDERS = 1, HEADER_AND_FOOTER = 2, COLUMNS = 4, DISABLE_BORDERS = 8,
       KW_HEADER_AND_FOOTER = 16, DISABLE_UNIT = 32 };

/**
 *  KgvPagePreview.
 *  Internal to KgvPageLayoutDia.
 */
class KgvPagePreview : public QGroupBox
{
    Q_OBJECT

public:

    /**
     *  constructor
     */
    KgvPagePreview( QWidget*, const char*, const KgvPageLayout & );
    /**
     *  destructor
     */
    ~KgvPagePreview();

    /**
     *  set page layout
     */
    void setPageLayout( const KgvPageLayout& );
    void setPageColumns( const KgvColumns& );

protected:

    // paint page
    void drawContents( QPainter* );

    double m_pageHeight, m_pageWidth, m_textFrameX, m_textFrameY, m_textFrameWidth, m_textFrameHeight;
    int columns;
};

class KgvPageLayoutDiaPrivate;

/**
 *  With this dialog the user can specify the layout of the paper during printing.
 */
class KgvPageLayoutDia : public KDialogBase
{
    Q_OBJECT

public:

    /**
     *  Constructor.
     *
     *  @param parent   The parent of the dialog.
     *  @param name     The name of the dialog.
     *  @param layout   The layout.
     *  @param headfoot The header and the footer.
     *  @param flags     a variable with all features this dialog should show.
     *  @param unit     The unit to use for displaying the values to the user.
     *  @param modal    Whether the dialog is modal or not.
     */
    KgvPageLayoutDia( QWidget* parent, const char* name,
             KgvPageLayout& layout,
             const KgvHeadFoot& headfoot,
             int flags, KgvUnit::Unit unit, bool modal=true );

    /**
     *  Constructor.
     *
     *  @param parent     The parent of the dialog.
     *  @param name       The name of the dialog.
     *  @param layout     The layout.
     *  @param headfoot   The header and the footer.
     *  @param columns    The number of columns on the page.
     *  @param kwheadfoot The KWord header and footer.
     *  @param tabs       The number of tabs.
     *  @param unit       The unit to use for displaying the values to the user
     */
    KgvPageLayoutDia( QWidget* parent, const char* name,
             KgvPageLayout& layout,
             const KgvHeadFoot& headfoot,
             const KgvColumns& columns,
             const KgvKWHeaderFooter& kwheadfoot,
             int tabs, KgvUnit::Unit unit );

    /**
     *  Destructor.
     */
    ~KgvPageLayoutDia();

    /**
     *  Show page layout dialog.
     *  See constructor for documentation on the parameters
     */
    static bool pageLayout( KgvPageLayout&, KgvHeadFoot&, int tabs, KgvUnit::Unit& unit, QWidget* parent = 0 );

    /**
     *  Show page layout dialog.
     *  See constructor for documentation on the parameters
     */
    static bool pageLayout( KgvPageLayout&, KgvHeadFoot&, KgvColumns&, KgvKWHeaderFooter&, int tabs, KgvUnit::Unit& unit, QWidget* parent = 0 );
    /**
     *  Retrieves a standard page layout.
     *  Deprecated: better use KgvPageLayout::standardLayout()
     */
    static KDE_DEPRECATED KgvPageLayout standardLayout();

    /**
     *  Returns the layout
     */
    const KgvPageLayout& layout() const { return m_layout; }

    /**
     *  Returns the header and footer information
     */
    KgvHeadFoot headFoot() const;

    /**
     *  Returns the unit
     */
    KgvUnit::Unit unit() const { return m_unit; }

private:
    const KgvColumns& columns() { return m_column; }
    const KgvKWHeaderFooter& headerFooter();

    // setup tabs
    void setupTab1( bool enableBorders );
//     void setupTab2( const KgvHeadFoot& hf );
//     void setupTab3();
//     void setupTab4( const KgvKWHeaderFooter kwhf );

    // dialog objects
    QLineEdit *eHeadLeft;
    QLineEdit *eHeadMid;
    QLineEdit *eHeadRight;
    QLineEdit *eFootLeft;
    QLineEdit *eFootMid;
    QLineEdit *eFootRight;

    // layout
    KgvPageLayout& m_layout;
    KgvColumns m_column;

    KgvUnit::Unit m_unit;

    int flags;

protected slots:
    virtual void slotOk();

private slots:
    void sizeUpdated(KgvPageLayout &layout);
    void columnsUpdated(KgvColumns &columns);

private:
    KgvPageLayoutSize *m_pageSizeTab;
    KgvPageLayoutColumns *m_columnsTab;
    KgvPageLayoutHeader *m_headerTab;
    KgvPageLayoutDiaPrivate *d;
};

#endif
