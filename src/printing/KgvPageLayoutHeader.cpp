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

#include <KgvPageLayoutHeader.h>
#include <KgvPageLayoutHeader.moc>
#include <KgvUnitWidgets.h>

#include <qlayout.h>
#include <qcheckbox.h>

KgvPageLayoutHeader::KgvPageLayoutHeader(QWidget *parent, KgvUnit::Unit unit, const KgvKWHeaderFooter &kwhf)
    : KgvPageLayoutHeaderBase(parent) {
    m_headerFooters = kwhf;
    QHBoxLayout *lay = new QHBoxLayout(headerSpacingPane);
    m_headerSpacing = new KgvUnitDoubleSpinBox( headerSpacingPane, 0.0, 999.0, 0.5, kwhf.ptHeaderBodySpacing, unit );
    lay->addWidget(m_headerSpacing);

    lay = new QHBoxLayout(footerSpacingPane);
    m_footerSpacing = new KgvUnitDoubleSpinBox( footerSpacingPane, 0.0, 999.0, 0.5, kwhf.ptFooterBodySpacing, unit );
    lay->addWidget(m_footerSpacing);

    lay = new QHBoxLayout(footnotePane);
    m_footnoteSpacing = new KgvUnitDoubleSpinBox( footnotePane, 0.0, 999.0, 0.5, kwhf.ptFootNoteBodySpacing, unit );
    lay->addWidget(m_footnoteSpacing);

    if ( kwhf.header == HF_FIRST_DIFF || kwhf.header == HF_FIRST_EO_DIFF )
        rhFirst->setChecked( true );
    if ( kwhf.header == HF_EO_DIFF || kwhf.header == HF_FIRST_EO_DIFF )
        rhEvenOdd->setChecked( true );
    if ( kwhf.footer == HF_FIRST_DIFF || kwhf.footer == HF_FIRST_EO_DIFF )
        rfFirst->setChecked( true );
    if ( kwhf.footer == HF_EO_DIFF || kwhf.footer == HF_FIRST_EO_DIFF )
        rfEvenOdd->setChecked( true );
}

const KgvKWHeaderFooter& KgvPageLayoutHeader::headerFooter() {
    if ( rhFirst->isChecked() && rhEvenOdd->isChecked() )
        m_headerFooters.header = HF_FIRST_EO_DIFF;
    else if ( rhFirst->isChecked() )
        m_headerFooters.header = HF_FIRST_DIFF;
    else if ( rhEvenOdd->isChecked() )
        m_headerFooters.header = HF_EO_DIFF;
    else
        m_headerFooters.header = HF_SAME;

    m_headerFooters.ptHeaderBodySpacing = m_headerSpacing->value();
    m_headerFooters.ptFooterBodySpacing = m_footerSpacing->value();
    m_headerFooters.ptFootNoteBodySpacing = m_footnoteSpacing->value();
    if ( rfFirst->isChecked() && rfEvenOdd->isChecked() )
        m_headerFooters.footer = HF_FIRST_EO_DIFF;
    else if ( rfFirst->isChecked() )
        m_headerFooters.footer = HF_FIRST_DIFF;
    else if ( rfEvenOdd->isChecked() )
        m_headerFooters.footer = HF_EO_DIFF;
    else
        m_headerFooters.footer = HF_SAME;
    return m_headerFooters;
}
