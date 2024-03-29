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

#ifndef kgvpagelayoutsize_h
#define kgvpagelayoutsize_h

// lib
#include "KgvPageLayout.h"
#include "KgvUnit.h"
// Qt
#include <QWidget>
#include <QButtonGroup>

class KgvUnitDoubleSpinBox;
class KgvPagePreview;

class QGroupBox;
class QComboBox;

/**
 * This class is a widget that shows the KgvPageLayout data structure and allows the user to change it.
 */
class KgvPageLayoutSize : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent the parent widget
     * @param layout the page layout that this widget should be initialized with.
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param columns the KgvColumns (amout of columns) that the preview should be initialized with
     * @param unitChooser if true a combobox with the unit-type is shown for the user to change
     * @param enableBorders if true enable the user to change the margins (aka borders) of the page
     */
    KgvPageLayoutSize(QWidget *parent, const KgvPageLayout &layout, KgvUnit::Unit unit, const KgvColumns &columns, bool unitChooser, bool enableBorders);

    /**
     * @return if the dialog is in a sane state and the values can be used.
     */
    bool queryClose();
    /**
     * Update the page preview widget with the param columns.
     * @param columns the new columns
     */
    void setColumns(KgvColumns &columns);

public Q_SLOTS:
    /**
     * Set a new unit for the widget updating the widgets.
     * @param unit the new unit
     */
    void setUnit(KgvUnit::Unit unit);
    /**
     * Enable the user to edit the page border size
     * @param on if true enable the user to change the margins (aka borders) of the page
     */
    void setEnableBorders(bool on);

Q_SIGNALS:
    /**
     * Emitted whenever the user changed something in the dialog.
     * @param layout the update layout structure with currently displayed info.
     * Note that the info may not be fully correct and physically possible (in which
     * case queryClose will return false)
     */
    void propertyChange(KgvPageLayout &layout);

protected:
    QComboBox *cpgFormat;
    KgvUnitDoubleSpinBox *epgWidth;
    KgvUnitDoubleSpinBox *epgHeight;
    KgvUnitDoubleSpinBox *ebrLeft;
    KgvUnitDoubleSpinBox *ebrRight;
    KgvUnitDoubleSpinBox *ebrTop;
    KgvUnitDoubleSpinBox *ebrBottom;
    KgvPagePreview *pgPreview;
    QGroupBox *m_orientGroup;
    QButtonGroup m_orientButtons;

protected Q_SLOTS:
    void formatChanged(int);
    void widthChanged(double);
    void heightChanged(double);
    void leftChanged(double);
    void rightChanged(double);
    void topChanged(double);
    void bottomChanged(double);
    void orientationChanged(int);
    void setUnitInt(int unit);

private:
    void updatePreview();
    void setValues();

    KgvUnit::Unit m_unit;
    KgvPageLayout m_layout;

    bool m_blockSignals, m_haveBorders;
};

#endif
