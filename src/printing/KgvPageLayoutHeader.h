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

#ifndef kgvpagelayoutheader_h
#define kgvpagelayoutheader_h

#include <KgvUnit.h>
#include <KgvPageLayout.h>
#include <KgvPageLayoutHeaderBase.h>

class QWidget;
class KgvUnitDoubleSpinBox;
class KgvPagePreview;

/**
 * This class is a widget that shows the KgvKWHeaderFooter data structure and allows the user to change it.
 */
class KgvPageLayoutHeader : public KgvPageLayoutHeaderBase {
    Q_OBJECT

public:
    /**
     * Contructor
     * @param parent the parent widget
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param kwhf the data that this widget will be filled with initially
     */
    KgvPageLayoutHeader(QWidget *parent, KgvUnit::Unit unit, const KgvKWHeaderFooter &kwhf);
    /**
     * @return the altered data as it is currently set by the user.
     */
    const KgvKWHeaderFooter& headerFooter();

private:
    KgvUnitDoubleSpinBox *m_headerSpacing, *m_footerSpacing, *m_footnoteSpacing;

    KgvKWHeaderFooter m_headerFooters;
};

#endif

