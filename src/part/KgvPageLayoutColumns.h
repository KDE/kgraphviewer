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

#ifndef kgvpagelayoutcolumns_h
#define kgvpagelayoutcolumns_h

// lib
#include "ui_KgvPageLayoutColumnsBase.h"
#include "KgvPageLayout.h"
#include "KgvUnit.h"
// Qt
#include <QWidget>

class KgvPagePreview;
class KgvUnitDoubleSpinBox;

/**
 * This class is a widget that shows the KgvColumns data structure and allows the user to change it.
 */
class KgvPageLayoutColumns : public QWidget, public Ui::KgvPageLayoutColumnsBase
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent the parent widget
     * @param columns the KgvColumns data structure that this dialog should be initialized with
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param layout the page layout that the preview should be initialized with.
     */
    KgvPageLayoutColumns(QWidget *parent, const KgvColumns &columns, KgvUnit::Unit unit, const KgvPageLayout &layout);

    /**
     * Update the page preview widget with the param layout.
     * @param layout the new layout
     */
    void setLayout(KgvPageLayout &layout);
public Q_SLOTS:

    /**
     * Enable the user to edit the columns
     * @param on if true enable the user to change the columns count
     */
    void setEnableColumns(bool on);

Q_SIGNALS:
    void propertyChange(KgvColumns &columns);

protected:
    KgvColumns m_columns;
    KgvPagePreview *m_preview;
    KgvUnitDoubleSpinBox *m_spacing;

private Q_SLOTS:
    void nColChanged(int);
    void nSpaceChanged(double);
};

#endif
