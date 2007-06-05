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

#include <KgvPageLayoutColumns.h>
#include <KgvPageLayoutDia.h>
#include <KgvUnit.h>
#include <KgvUnitWidgets.h>

#include <qlabel.h>
#include <qlayout.h>

KgvPageLayoutColumns::KgvPageLayoutColumns(QWidget *parent, const KgvColumns& columns, KgvUnit::Unit unit, const KgvPageLayout& layout)
    : KgvPageLayoutColumnsBase(parent) {
    m_columns = columns;
    QHBoxLayout *lay = new QHBoxLayout(previewPane);
    m_preview = new KgvPagePreview( previewPane, "Preview", layout );
    lay->addWidget(m_preview);
    lay = new QHBoxLayout(columnSpacingPane);
    m_spacing = new KgvUnitDoubleSpinBox( columnSpacingPane );
    m_spacing->setValue(  m_columns.ptColumnSpacing );
    m_spacing->setUnit( unit );
    double dStep = KgvUnit::fromUserValue( 0.2, unit );
    m_spacing->setMinMaxStep( 0, layout.ptWidth/2, dStep );
    lay->addWidget(m_spacing);
    labelSpacing->setBuddy( m_spacing );
    nColumns->setValue( m_columns.columns );
    m_preview->setPageColumns( m_columns );

    connect( nColumns, SIGNAL( valueChanged( int ) ), this, SLOT( nColChanged( int ) ) );
    connect( m_spacing, SIGNAL( valueChangedPt(double) ), this, SLOT( nSpaceChanged( double ) ) );
}

void KgvPageLayoutColumns::setEnableColumns(bool on) {
    nColumns->setEnabled(on);
    m_spacing->setEnabled(on);
    nColChanged(on ? nColumns->value(): 1 );
}

void KgvPageLayoutColumns::nColChanged( int columns ) {
    m_columns.columns = columns;
    m_preview->setPageColumns( m_columns );
    emit propertyChange(m_columns);
}

void KgvPageLayoutColumns::nSpaceChanged( double spacing ) {
    m_columns.ptColumnSpacing = spacing;
    emit propertyChange(m_columns);
}

void KgvPageLayoutColumns::setLayout(KgvPageLayout &layout) {
    m_preview->setPageLayout( layout );
}

#include <KgvPageLayoutColumns.moc>
