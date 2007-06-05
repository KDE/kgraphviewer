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

#ifndef kgvpagelayoutsize_h
#define kgvpagelayoutsize_h

#include <qgroupbox.h>
// #include <KgvGlobal.h>
#include <KgvUnit.h>
#include <kdialogbase.h>
#include <KgvPageLayout.h>
#include <KgvPageLayoutDia.h>

class QComboBox;
class KgvUnitDoubleSpinBox;
class KgvPageLayoutColumns;

/**
 * This class is a widget that shows the KgvPageLayout data structure and allows the user to change it.
 */
class KgvPageLayoutSize : public QWidget {
    Q_OBJECT

public:
    /**
     * Contructor
     * @param parent the parent widget
     * @param layout the page layout that this widget should be initialzed with.
     * @param unit the unit-type (mm/cm/inch) that the dialog should show
     * @param columns the KgvColumns (amout of columns) that the preview should be initialized with
     * @param unitChooser if true a combobox with the unit-type is shown for the user to change
     * @param enableBorders if true enable the user to change the margins (aka borders) of the page
     */
    KgvPageLayoutSize(QWidget *parent, const KgvPageLayout& layout, KgvUnit::Unit unit,
            const KgvColumns& columns, bool unitChooser, bool enableBorders);

    /**
     * @return if the dialog is in a sane state and the values can be used.
     */
    bool queryClose();
    /**
     * Update the page preview widget with the param columns.
     * @param columns the new columns
     */
    void setColumns(KgvColumns &columns);

public slots:
    /**
     * Set a new unit for the widget updating the widgets.
     * @param unit the new unit
     */
    void setUnit( KgvUnit::Unit unit );
    /**
     * Enable the user to edit the page border size
     * @param on if true enable the user to change the margins (aka borders) of the page
     */
    void setEnableBorders(bool on);

signals:
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
    QButtonGroup *m_orientGroup;

protected slots:
    void formatChanged( int );
    void widthChanged( double );
    void heightChanged( double );
    void leftChanged( double );
    void rightChanged( double );
    void topChanged( double );
    void bottomChanged( double );
    void orientationChanged( int );
    void setUnitInt( int unit );

private:
    void updatePreview();
    void setValues();

    KgvUnit::Unit m_unit;
    KgvPageLayout m_layout;

    bool m_blockSignals, m_haveBorders;
};

#endif
