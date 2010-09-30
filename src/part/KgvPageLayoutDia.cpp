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

// Description: Page Layout Dialog (sources)

/******************************************************************/

#include <KgvPageLayoutDia.h>
#include <KgvGlobal.h>
#include <KgvPageLayoutColumns.h>
#include <KgvPageLayoutSize.h>
#include <KgvPageLayoutHeader.h>
#include <KgvUnit.h>
#include <KgvUnitWidgets.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <khbox.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QPaintEngine>

/******************************************************************/
/* class KgvPagePreview                                            */
/******************************************************************/

/*===================== constrcutor ==============================*/
KgvPagePreview::KgvPagePreview( QWidget* parent, const char *name, const KgvPageLayout& layout )
    : QGroupBox( i18n( "Page Preview" ), parent, name )
{
    setPageLayout( layout );
    columns = 1;
    setMinimumSize( 150, 150 );
}

/*====================== destructor ==============================*/
KgvPagePreview::~KgvPagePreview()
{
}

/*=================== set layout =================================*/
void KgvPagePreview::setPageLayout( const KgvPageLayout &layout )
{
    // resolution[XY] is in pixel per pt
    double resolutionX = POINT_TO_INCH( static_cast<double>(KgvGlobal::dpiX()) );
    double resolutionY = POINT_TO_INCH( static_cast<double>(KgvGlobal::dpiY()) );

    m_pageWidth = layout.ptWidth * resolutionX;
    m_pageHeight = layout.ptHeight * resolutionY;

    double zh = 110.0 / m_pageHeight;
    double zw = 110.0 / m_pageWidth;
    double z = qMin( zw, zh );

    m_pageWidth *= z;
    m_pageHeight *= z;

    m_textFrameX = layout.ptLeft * resolutionX * z;
    m_textFrameY = layout.ptTop * resolutionY * z;
    m_textFrameWidth = m_pageWidth - ( layout.ptLeft + layout.ptRight ) * resolutionX * z;
    m_textFrameHeight = m_pageHeight - ( layout.ptTop + layout.ptBottom ) * resolutionY * z;

    kDebug() << "repaint in setPageLayout";
    repaint( true );
}

/*=================== set layout =================================*/
void KgvPagePreview::setPageColumns( const KgvColumns &_columns )
{
    columns = _columns.columns;
    repaint( true );
}

/*======================== draw contents =========================*/
void KgvPagePreview::paintEvent ( QPaintEvent * event ) 
{
  QGroupBox::paintEvent(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  double cw = m_textFrameWidth;
  if(columns!=1)
  {
    cw/=static_cast<double>(columns);
  }
  painter.setBrush( Qt::white );
  painter.setPen( QPen( Qt::black ) );

  int x=static_cast<int>( ( width() - m_pageWidth ) * 0.5 );
  int y=static_cast<int>( ( height() - m_pageHeight ) * 0.5 );
  int w=static_cast<int>(m_pageWidth);
  int h=static_cast<int>(m_pageHeight);
  //painter.drawRect( x + 1, y + 1, w, h);
  painter.drawRect( x, y, w, h );

  painter.setBrush( QBrush( Qt::black, Qt::HorPattern ) );
  if ( m_textFrameWidth == m_pageWidth || m_textFrameHeight == m_pageHeight )
  {
    painter.setPen( Qt::NoPen );
  }
  else
  {
    painter.setPen( Qt::lightGray );
  }

  for ( int i = 0; i < columns; ++i )
  {
    painter.drawRect( x + static_cast<int>(m_textFrameX) + static_cast<int>(i * cw),
                        y + static_cast<int>(m_textFrameY), static_cast<int>(cw),
                        static_cast<int>(m_textFrameHeight) );
  }
}

/******************************************************************/
/* class KgvPageLayoutDia                                          */
/******************************************************************/

/*==================== constructor ===============================*/
KgvPageLayoutDia::KgvPageLayoutDia( QWidget* parent,
                                  KgvPageLayout& layout,
                                  int tabs,
                                  KgvUnit::Unit unit)
                                  : KPageDialog(parent),
/*                                  : KDialogBase( KDialogBase::Tabbed, i18n("Page Layout"), KDialogBase::Ok | KDialogBase::Cancel,
                                  KDialogBase::Ok, parent, name, modal),*/
      m_layout(layout),
      m_unit(unit),
      flags(tabs),
      m_pageSizeTab(0),
      m_columnsTab(0),
      m_headerTab(0)

{
    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

    m_column.columns = 1;

    if ( tabs & FORMAT_AND_BORDERS ) setupTab1( true );
//     if ( tabs & HEADER_AND_FOOTER ) setupTab2( hf );

    setFocusPolicy( Qt::StrongFocus );
    setFocus();
    connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
}

/*==================== constructor ===============================*/
KgvPageLayoutDia::KgvPageLayoutDia( QWidget* parent,
                  KgvPageLayout& layout,
                  const KgvColumns& columns,
                  int tabs, KgvUnit::Unit unit )
                  : KPageDialog( parent ),
/*                  : KDialogBase( KDialogBase::Tabbed, i18n("Page Layout"), KDialogBase::Ok | KDialogBase::Cancel,
                  KDialogBase::Ok, parent, name, true),*/
      m_layout(layout),
    m_column(columns),
    m_unit(unit),
    flags(tabs),
    m_pageSizeTab(0),
    m_columnsTab(0),
    m_headerTab(0)
{
  setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

    if ( tabs & FORMAT_AND_BORDERS ) setupTab1( !( tabs & DISABLE_BORDERS ) );
//     if ( tabs & HEADER_AND_FOOTER ) setupTab2( hf );
//     if ( tabs & COLUMNS ) setupTab3();
//     if ( tabs & KW_HEADER_AND_FOOTER ) setupTab4(kwhf);

    setFocusPolicy( Qt::StrongFocus );
    setFocus();
    connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
}

/*===================== destructor ===============================*/
KgvPageLayoutDia::~KgvPageLayoutDia()
{
}

/*======================= show dialog ============================*/
bool KgvPageLayoutDia::pageLayout( KgvPageLayout& layout, KgvHeadFoot& hf, int tabs, KgvUnit::Unit& unit, QWidget* parent )
{
    bool res = false;
    QPointer<KgvPageLayoutDia> dlg = new KgvPageLayoutDia( parent, layout, tabs, unit );

    if ( dlg->exec() == QDialog::Accepted ) {
        res = true;
        if ( tabs & FORMAT_AND_BORDERS ) layout = dlg->layout();
        if ( tabs & HEADER_AND_FOOTER ) hf = dlg->headFoot();
        unit = dlg->unit();
    }

    delete dlg;

    return res;
}

/*======================= show dialog ============================*/
bool KgvPageLayoutDia::pageLayout( KgvPageLayout& layout, KgvHeadFoot& hf, KgvColumns& columns,
                                  KgvKWHeaderFooter &_kwhf, int tabs, KgvUnit::Unit& unit, QWidget* parent )
{
    bool res = false;
    KgvPageLayoutDia *dlg = new KgvPageLayoutDia( parent, layout, columns, tabs, unit );

    if ( dlg->exec() == QDialog::Accepted ) {
        res = true;
        if ( tabs & FORMAT_AND_BORDERS ) layout = dlg->layout();
        if ( tabs & HEADER_AND_FOOTER ) hf = dlg->headFoot();
        if ( tabs & COLUMNS ) columns = dlg->columns();
        if ( tabs & KW_HEADER_AND_FOOTER ) _kwhf = dlg->headerFooter();
        unit = dlg->unit();
    }

    delete dlg;

    return res;
}

/*===================== get a standard page layout ===============*/
KgvPageLayout KgvPageLayoutDia::standardLayout()
{
    return KgvPageLayout::standardLayout();
}

/*====================== get header - footer =====================*/
KgvHeadFoot KgvPageLayoutDia::headFoot() const
{
    KgvHeadFoot hf;
    hf.headLeft = eHeadLeft->text();
    hf.headMid = eHeadMid->text();
    hf.headRight = eHeadRight->text();
    hf.footLeft = eFootLeft->text();
    hf.footMid = eFootMid->text();
    hf.footRight = eFootRight->text();
    return hf;
}

/*================================================================*/
const KgvKWHeaderFooter& KgvPageLayoutDia::headerFooter()
{
    return m_headerTab->headerFooter();
}

/*================ setup page size & margins tab ==================*/
void KgvPageLayoutDia::setupTab1( bool enableBorders )
{
  KHBox *page = new KHBox();
  addPage(page, i18n( "Page Size & Margins" ));
// QWidget *tab1 = addPage(i18n( "Page Size && &Margins" ));
//     QHBoxLayout *lay = new QHBoxLayout(tab1);
    m_pageSizeTab = new KgvPageLayoutSize(page, m_layout, m_unit, m_column, !(flags & DISABLE_UNIT), enableBorders );
//     lay->addWidget(m_pageSizeTab);
    m_pageSizeTab->show();
    connect (m_pageSizeTab, SIGNAL( propertyChange(KgvPageLayout&)),
            this, SLOT (sizeUpdated( KgvPageLayout&)));
}

void KgvPageLayoutDia::sizeUpdated(KgvPageLayout &layout) {
    m_layout.ptWidth = layout.ptWidth;
    m_layout.ptHeight = layout.ptHeight;
    m_layout.ptLeft = layout.ptLeft;
    m_layout.ptRight = layout.ptRight;
    m_layout.ptTop = layout.ptTop;
    m_layout.ptBottom = layout.ptBottom;
    m_layout.format = layout.format;
    m_layout.orientation = layout.orientation;
    if(m_columnsTab)
        m_columnsTab->setLayout(layout);
}

// /*================ setup header and footer tab ===================*/
// void KgvPageLayoutDia::setupTab2( const KgvHeadFoot& hf )
// {
//     QWidget *tab2 = addPage(i18n( "H&eader && Footer" ));
//     QGridLayout *grid2 = new QGridLayout( tab2, 7, 2, 0, KDialog::spacingHint() );
// 
//     // ------------- header ---------------
//     QGroupBox *gHead = new QGroupBox( 0, Qt::Vertical, i18n( "Head Line" ), tab2 );
//     gHead->layout()->setSpacing(KDialog::spacingHint());
//     gHead->layout()->setMargin(KDialog::marginHint());
//     QGridLayout *headGrid = new QGridLayout( gHead->layout(), 2, 3 );
// 
//     QLabel *lHeadLeft = new QLabel( i18n( "Left:" ), gHead );
//     headGrid->addWidget( lHeadLeft, 0, 0 );
// 
//     eHeadLeft = new QLineEdit( gHead );
//     headGrid->addWidget( eHeadLeft, 1, 0 );
//     eHeadLeft->setText( hf.headLeft );
// 
//     QLabel *lHeadMid = new QLabel( i18n( "Mid:" ), gHead );
//     headGrid->addWidget( lHeadMid, 0, 1 );
// 
//     eHeadMid = new QLineEdit( gHead );
//     headGrid->addWidget( eHeadMid, 1, 1 );
//     eHeadMid->setText( hf.headMid );
// 
//     QLabel *lHeadRight = new QLabel( i18n( "Right:" ), gHead );
//     headGrid->addWidget( lHeadRight, 0, 2 );
// 
//     eHeadRight = new QLineEdit( gHead );
//     headGrid->addWidget( eHeadRight, 1, 2 );
//     eHeadRight->setText( hf.headRight );
// 
//     grid2->addMultiCellWidget( gHead, 0, 1, 0, 1 );
// 
//     // ------------- footer ---------------
//     QGroupBox *gFoot = new QGroupBox( 0, Qt::Vertical, i18n( "Foot Line" ), tab2 );
//     gFoot->layout()->setSpacing(KDialog::spacingHint());
//     gFoot->layout()->setMargin(KDialog::marginHint());
//     QGridLayout *footGrid = new QGridLayout( gFoot->layout(), 2, 3 );
// 
//     QLabel *lFootLeft = new QLabel( i18n( "Left:" ), gFoot );
//     footGrid->addWidget( lFootLeft, 0, 0 );
// 
//     eFootLeft = new QLineEdit( gFoot );
//     footGrid->addWidget( eFootLeft, 1, 0 );
//     eFootLeft->setText( hf.footLeft );
// 
//     QLabel *lFootMid = new QLabel( i18n( "Mid:" ), gFoot );
//     footGrid->addWidget( lFootMid, 0, 1 );
// 
//     eFootMid = new QLineEdit( gFoot );
//     footGrid->addWidget( eFootMid, 1, 1 );
//     eFootMid->setText( hf.footMid );
// 
//     QLabel *lFootRight = new QLabel( i18n( "Right:" ), gFoot );
//     footGrid->addWidget( lFootRight, 0, 2 );
// 
//     eFootRight = new QLineEdit( gFoot );
//     footGrid->addWidget( eFootRight, 1, 2 );
//     eFootRight->setText( hf.footRight );
// 
//     grid2->addMultiCellWidget( gFoot, 2, 3, 0, 1 );
// 
//     QLabel *lMacros2 = new QLabel( i18n( "You can insert several tags in the text:" ), tab2 );
//     grid2->addMultiCellWidget( lMacros2, 4, 4, 0, 1 );
// 
//     QLabel *lMacros3 = new QLabel( i18n("<qt><ul><li>&lt;sheet&gt; The sheet name</li>"
//                            "<li>&lt;page&gt; The current page</li>"
//                            "<li>&lt;pages&gt; The total number of pages</li>"
//                            "<li>&lt;name&gt; The filename or URL</li>"
//                            "<li>&lt;file&gt; The filename with complete path or the URL</li></ul></qt>"), tab2 );
//     grid2->addMultiCellWidget( lMacros3, 5, 6, 0, 0, Qt::AlignTop );
// 
//     QLabel *lMacros4 = new QLabel( i18n("<qt><ul><li>&lt;time&gt; The current time</li>"
//                            "<li>&lt;date&gt; The current date</li>"
//                            "<li>&lt;author&gt; Your full name</li>"
//                            "<li>&lt;org&gt; Your organization</li>"
//                            "<li>&lt;email&gt; Your email address</li></ul></qt>"), tab2 );
//     grid2->addMultiCellWidget( lMacros4, 5, 6, 1, 1, Qt::AlignTop );
// }
// 
// /*================================================================*/
// void KgvPageLayoutDia::setupTab3()
// {
//     QWidget *tab3 = addPage(i18n( "Col&umns" ));
//     QHBoxLayout *lay = new QHBoxLayout(tab3);
//     m_columnsTab = new KgvPageLayoutColumns(tab3, m_column, m_unit, m_layout);
//     m_columnsTab->layout()->setMargin(0);
//     lay->addWidget(m_columnsTab);
//     m_columnsTab->show();
//     connect (m_columnsTab, SIGNAL( propertyChange(KgvColumns&)),
//             this, SLOT (columnsUpdated( KgvColumns&)));
// }
// 
// void KgvPageLayoutDia::columnsUpdated(KgvColumns &columns) {
//     m_column.columns = columns.columns;
//     m_column.ptColumnSpacing = columns.ptColumnSpacing;
//     if(m_pageSizeTab)
//         m_pageSizeTab->setColumns(columns);
// }
// 
// /*================================================================*/
// void KgvPageLayoutDia::setupTab4(const KgvKWHeaderFooter kwhf )
// {
//     QWidget *tab4 = addPage(i18n( "H&eader && Footer" ));
//     QHBoxLayout *lay = new QHBoxLayout(tab4);
//     m_headerTab = new KgvPageLayoutHeader(tab4, m_unit, kwhf);
//     m_headerTab->layout()->setMargin(0);
//     lay->addWidget(m_headerTab);
//     m_headerTab->show();
// 
// }
// 

/* Validation when closing. Error messages are never liked, but
  better let the users enter all values in any order, and have one
  final validation, than preventing them from entering values. */
void KgvPageLayoutDia::slotOk()
{
    if( m_pageSizeTab )
        m_pageSizeTab->queryClose();
    KPageDialog::accept(); // accept
}

#include <KgvPageLayoutDia.moc>
