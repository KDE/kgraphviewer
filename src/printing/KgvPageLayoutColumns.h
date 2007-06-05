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

// Description: Page Layout Dialog (sources)

#ifndef kgvpagelayoutcolumns_h
#define kgvpagelayoutcolumns_h

#include <KgvUnit.h>
#include <KgvPageLayout.h>
#include <KgvPageLayoutColumnsBase.h>

class QWidget;
class KgvUnitDoubleSpinBox;
class KgvPagePreview;

/**
 * This class is a widget that shows the KgvColumns data structure and allows the user to change it.
 */
class KgvPageLayoutColumns : public KgvPageLayoutColumnsBase {
    Q_OBJECT

public:
    /**
     * Contructor
     * @param parent the parent widget
     * @param columns the KgvColumns data structure that this dialog should be initialzed with
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param layout the page layout that the preview should be initialzed with.
     */
    KgvPageLayoutColumns(QWidget *parent, const KgvColumns& columns, KgvUnit::Unit unit, const KgvPageLayout& layout);

    /**
     * Update the page preview widget with the param layout.
     * @param layout the new layout
     */
    void setLayout(KgvPageLayout &layout);
public slots:

    /**
     * Enable the user to edit the columns
     * @param on if true enable the user to change the columns count
     */
    void setEnableColumns(bool on);

signals:
    void propertyChange(KgvColumns &columns);

protected:
    KgvColumns m_columns;
    KgvPagePreview *m_preview;
    KgvUnitDoubleSpinBox *m_spacing;

private slots:
    void nColChanged( int );
    void nSpaceChanged( double );
};

#endif
