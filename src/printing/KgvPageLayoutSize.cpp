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

#include <KgvPageLayoutDia.h>
#include <KgvPageLayoutSize.h>
#include <KgvUnit.h>
#include <KgvUnitWidgets.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qhbuttongroup.h>

KgvPageLayoutSize::KgvPageLayoutSize(QWidget *parent, const KgvPageLayout& layout, KgvUnit::Unit unit,const KgvColumns& columns,  bool unitChooser, bool enableBorders)
    : QWidget(parent), m_blockSignals(false) {
    m_layout = layout;
    m_unit = unit;

    QGridLayout *grid1 = new QGridLayout( this, 5, 2, 0, KDialog::spacingHint() );
    if ( unitChooser ) {
        // ------------- unit _______________
        QWidget* unitFrame = new QWidget( this );
        grid1->addWidget( unitFrame, 0, 0, Qt::AlignLeft );
        QBoxLayout* unitLayout = new QHBoxLayout( unitFrame, 0, KDialog::spacingHint() );

        // label unit
        QLabel *lpgUnit = new QLabel( i18n( "Unit:" ), unitFrame );
        unitLayout->addWidget( lpgUnit, 0, Qt::AlignRight | Qt::AlignVCenter );

        // combo unit
        QComboBox *cpgUnit = new QComboBox( false, unitFrame, "cpgUnit" );
        lpgUnit->setBuddy( cpgUnit );
        cpgUnit->insertStringList( KgvUnit::listOfUnitName() );
        cpgUnit->setCurrentItem( unit );
        unitLayout->addWidget( cpgUnit, 0, Qt::AlignLeft | Qt::AlignVCenter );
        connect( cpgUnit, SIGNAL( activated( int ) ), this, SLOT( setUnitInt( int ) ) );
    }
    else {
        QString str=KgvUnit::unitDescription(unit);

        QLabel *lpgUnit = new QLabel( i18n("All values are given in %1.").arg(str), this );
        grid1->addWidget( lpgUnit, 0, 0, Qt::AlignLeft );
    }

    // -------------- page size -----------------
    QVGroupBox *formatFrame = new QVGroupBox( i18n( "Page Size" ), this );
    grid1->addWidget( formatFrame, 1, 0 );

    QHBox *formatPageSize = new QHBox( formatFrame );
    formatPageSize->setSpacing( KDialog::spacingHint() );

    // label page size
    QLabel *lpgFormat = new QLabel( i18n( "&Size:" ), formatPageSize );

    // combo size
    cpgFormat = new QComboBox( false, formatPageSize, "cpgFormat" );
    cpgFormat->insertStringList( KgvPageFormat::allFormats() );
    lpgFormat->setBuddy( cpgFormat );
    connect( cpgFormat, SIGNAL( activated( int ) ), this, SLOT( formatChanged( int ) ) );

    // spacer
    formatPageSize->setStretchFactor( new QWidget( formatPageSize ), 10 );

    QHBox *formatCustomSize = new QHBox( formatFrame );
    formatCustomSize->setSpacing( KDialog::spacingHint() );

    // label width
    QLabel *lpgWidth = new QLabel( i18n( "&Width:" ), formatCustomSize );

    // linedit width
    epgWidth = new KgvUnitDoubleSpinBox( formatCustomSize, "Width" );
    lpgWidth->setBuddy( epgWidth );
    if ( m_layout.format != PG_CUSTOM )
        epgWidth->setEnabled( false );
    connect( epgWidth, SIGNAL( valueChangedPt(double) ), this, SLOT( widthChanged(double) ) );

    // label height
    QLabel *lpgHeight = new QLabel( i18n( "&Height:" ), formatCustomSize );

    // linedit height
    epgHeight = new KgvUnitDoubleSpinBox( formatCustomSize, "Height" );
    lpgHeight->setBuddy( epgHeight );
    if ( m_layout.format != PG_CUSTOM )
        epgHeight->setEnabled( false );
    connect( epgHeight, SIGNAL( valueChangedPt(double ) ), this, SLOT( heightChanged(double) ) );

    // --------------- orientation ---------------
    m_orientGroup = new QHButtonGroup( i18n( "Orientation" ), this );
    m_orientGroup->setInsideSpacing( KDialog::spacingHint() );
    grid1->addWidget( m_orientGroup, 2, 0 );

    QLabel* lbPortrait = new QLabel( m_orientGroup );
    lbPortrait->setPixmap( QPixmap( UserIcon( "koPortrait" ) ) );
    lbPortrait->setMaximumWidth( lbPortrait->pixmap()->width() );
    new QRadioButton( i18n("&Portrait"), m_orientGroup );

    QLabel* lbLandscape = new QLabel( m_orientGroup );
    lbLandscape->setPixmap( QPixmap( UserIcon( "koLandscape" ) ) );
    lbLandscape->setMaximumWidth( lbLandscape->pixmap()->width() );
    new QRadioButton( i18n("La&ndscape"), m_orientGroup );

    connect( m_orientGroup, SIGNAL (clicked (int)), this, SLOT( orientationChanged(int) ));

    // --------------- page margins ---------------
    QVGroupBox *marginsFrame = new QVGroupBox( i18n( "Margins" ), this );
    marginsFrame->setColumnLayout( 0, Qt::Vertical );
    marginsFrame->setMargin( KDialog::marginHint() );
    grid1->addWidget( marginsFrame, 3, 0 );

    QGridLayout *marginsLayout = new QGridLayout( marginsFrame->layout(), 3, 3,
       KDialog::spacingHint() );

    // left margin
    ebrLeft = new KgvUnitDoubleSpinBox( marginsFrame, "Left" );
    marginsLayout->addWidget( ebrLeft, 1, 0 );
    connect( ebrLeft, SIGNAL( valueChangedPt( double ) ), this, SLOT( leftChanged( double ) ) );

    // right margin
    ebrRight = new KgvUnitDoubleSpinBox( marginsFrame, "Right" );
    marginsLayout->addWidget( ebrRight, 1, 2 );
    connect( ebrRight, SIGNAL( valueChangedPt( double ) ), this, SLOT( rightChanged( double ) ) );

    // top margin
    ebrTop = new KgvUnitDoubleSpinBox( marginsFrame, "Top" );
    marginsLayout->addWidget( ebrTop, 0, 1 , Qt::AlignCenter );
    connect( ebrTop, SIGNAL( valueChangedPt( double ) ), this, SLOT( topChanged( double ) ) );

    // bottom margin
    ebrBottom = new KgvUnitDoubleSpinBox( marginsFrame, "Bottom" );
    marginsLayout->addWidget( ebrBottom, 2, 1, Qt::AlignCenter );
    connect( ebrBottom, SIGNAL( valueChangedPt( double ) ), this, SLOT( bottomChanged( double ) ) );

    // ------------- preview -----------
    pgPreview = new KgvPagePreview( this, "Preview", m_layout );
    grid1->addMultiCellWidget( pgPreview, 1, 3, 1, 1 );

    // ------------- spacers -----------
    QWidget* spacer1 = new QWidget( this );
    QWidget* spacer2 = new QWidget( this );
    spacer1->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
       QSizePolicy::Expanding ) );
    spacer2->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
       QSizePolicy::Expanding ) );
    grid1->addWidget( spacer1, 4, 0 );
    grid1->addWidget( spacer2, 4, 1 );

    setValues();
    updatePreview();
    pgPreview->setPageColumns( columns );
    setEnableBorders(enableBorders);
}

void KgvPageLayoutSize::setEnableBorders(bool on) {
    m_haveBorders = on;
    ebrLeft->setEnabled( on );
    ebrRight->setEnabled( on );
    ebrTop->setEnabled( on );
    ebrBottom->setEnabled( on );

    // update m_layout
    m_layout.ptLeft = on?ebrLeft->value():0;
    m_layout.ptRight = on?ebrRight->value():0;
    m_layout.ptTop = on?ebrTop->value():0;
    m_layout.ptBottom = on?ebrBottom->value():0;

    // use updated m_layout
    updatePreview();
    emit propertyChange(m_layout);
}

void KgvPageLayoutSize::updatePreview() {
    pgPreview->setPageLayout( m_layout );
}

void KgvPageLayoutSize::setValues() {
    // page format
    cpgFormat->setCurrentItem( m_layout.format );
    // orientation
    m_orientGroup->setButton( m_layout.orientation == PG_PORTRAIT ? 0: 1 );

    setUnit( m_unit );
    updatePreview();
}

void KgvPageLayoutSize::setUnit( KgvUnit::Unit unit ) {
    m_unit = unit;
    m_blockSignals = true; // due to non-atomic changes the propertyChange emits should be blocked

    epgWidth->setUnit( m_unit );
    epgWidth->setMinMaxStep( 0, KgvUnit::fromUserValue( 9999, m_unit ), KgvUnit::fromUserValue( 0.01, m_unit ) );
    epgWidth->changeValue( m_layout.ptWidth );

    epgHeight->setUnit( m_unit );
    epgHeight->setMinMaxStep( 0, KgvUnit::fromUserValue( 9999, m_unit ), KgvUnit::fromUserValue( 0.01, m_unit ) );
    epgHeight->changeValue( m_layout.ptHeight );

    double dStep = KgvUnit::fromUserValue( 0.2, m_unit );

    ebrLeft->setUnit( m_unit );
    ebrLeft->changeValue( m_layout.ptLeft );
    ebrLeft->setMinMaxStep( 0, m_layout.ptWidth, dStep );

    ebrRight->setUnit( m_unit );
    ebrRight->changeValue( m_layout.ptRight );
    ebrRight->setMinMaxStep( 0, m_layout.ptWidth, dStep );

    ebrTop->setUnit( m_unit );
    ebrTop->changeValue( m_layout.ptTop );
    ebrTop->setMinMaxStep( 0, m_layout.ptHeight, dStep );

    ebrBottom->setUnit( m_unit );
    ebrBottom->changeValue( m_layout.ptBottom );
    ebrBottom->setMinMaxStep( 0, m_layout.ptHeight, dStep );

    m_blockSignals = false;
}

void KgvPageLayoutSize::setUnitInt( int unit ) {
    setUnit((KgvUnit::Unit)unit);
}

void KgvPageLayoutSize::formatChanged( int format ) {
    if ( ( KgvFormat )format == m_layout.format )
        return;
    m_layout.format = ( KgvFormat )format;
    bool enable =  (KgvFormat) format == PG_CUSTOM;
    epgWidth->setEnabled( enable );
    epgHeight->setEnabled( enable );

    if ( m_layout.format != PG_CUSTOM ) {
        m_layout.ptWidth = MM_TO_POINT( KgvPageFormat::width(
                    m_layout.format, m_layout.orientation ) );
        m_layout.ptHeight = MM_TO_POINT( KgvPageFormat::height(
                    m_layout.format, m_layout.orientation ) );
    }

    epgWidth->changeValue( m_layout.ptWidth );
    epgHeight->changeValue( m_layout.ptHeight );

    updatePreview( );
    emit propertyChange(m_layout);
}

void KgvPageLayoutSize::orientationChanged(int which) {
    m_layout.orientation = which == 0 ? PG_PORTRAIT : PG_LANDSCAPE;

    // swap dimension
    double val = epgWidth->value();
    epgWidth->changeValue(epgHeight->value());
    epgHeight->changeValue(val);
    // and adjust margins
    m_blockSignals = true;
    val = ebrTop->value();
    if(m_layout.orientation == PG_PORTRAIT) { // clockwise
        ebrTop->changeValue(ebrRight->value());
        ebrRight->changeValue(ebrBottom->value());
        ebrBottom->changeValue(ebrLeft->value());
        ebrLeft->changeValue(val);
    } else { // counter clockwise
        ebrTop->changeValue(ebrLeft->value());
        ebrLeft->changeValue(ebrBottom->value());
        ebrBottom->changeValue(ebrRight->value());
        ebrRight->changeValue(val);
    }
    m_blockSignals = false;

    setEnableBorders(m_haveBorders); // will update preview+emit
}

void KgvPageLayoutSize::widthChanged(double width) {
    if(m_blockSignals) return;
    m_layout.ptWidth = width;
    updatePreview();
    emit propertyChange(m_layout);
}
void KgvPageLayoutSize::heightChanged(double height) {
    if(m_blockSignals) return;
    m_layout.ptHeight = height;
    updatePreview( );
    emit propertyChange(m_layout);
}
void KgvPageLayoutSize::leftChanged( double left ) {
    if(m_blockSignals) return;
    m_layout.ptLeft = left;
    updatePreview();
    emit propertyChange(m_layout);
}
void KgvPageLayoutSize::rightChanged(double right) {
    if(m_blockSignals) return;
    m_layout.ptRight = right;
    updatePreview();
    emit propertyChange(m_layout);
}
void KgvPageLayoutSize::topChanged(double top) {
    if(m_blockSignals) return;
    m_layout.ptTop = top;
    updatePreview();
    emit propertyChange(m_layout);
}
void KgvPageLayoutSize::bottomChanged(double bottom) {
    if(m_blockSignals) return;
    m_layout.ptBottom = bottom;
    updatePreview();
    emit propertyChange(m_layout);
}

bool KgvPageLayoutSize::queryClose() {
    if ( m_layout.ptLeft + m_layout.ptRight > m_layout.ptWidth ) {
        KMessageBox::error( this,
            i18n("The page width is smaller than the left and right margins."),
                            i18n("Page Layout Problem") );
        return false;
    }
    if ( m_layout.ptTop + m_layout.ptBottom > m_layout.ptHeight ) {
        KMessageBox::error( this,
            i18n("The page height is smaller than the top and bottom margins."),
                            i18n("Page Layout Problem") );
        return false;
    }
    return true;
}

void KgvPageLayoutSize::setColumns(KgvColumns &columns) {
    pgPreview->setPageColumns(columns);
}

#include <KgvPageLayoutSize.moc>
