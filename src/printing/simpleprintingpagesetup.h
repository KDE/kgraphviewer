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

#ifndef KGVSIMPLEPRINTINGPAGESETUP_H
#define KGVSIMPLEPRINTINGPAGESETUP_H

class DotGraphView;
class KGVSimplePrintingPageSetupBase;
class KGVSimplePrintingCommand;
class KgvPageLayoutSize;
class KGVSimplePrintPreviewWindow;

#include "simpleprintingengine.h"
#include <qbuttongroup.h>

//! @short A window for displaying settings for simple printing.
class KGVSimplePrintingPageSetup : public QWidget
{
	Q_OBJECT

	public:
		KGVSimplePrintingPageSetup(KGVSimplePrintingCommand* command, 
      DotGraphView *mainWin, QWidget *parent, QMap<QString,QString>* args );
		~KGVSimplePrintingPageSetup();

	public slots:
		void slotPrint();
		void slotPrintPreview();

	signals:
		void print(KGVSimplePrintingSettings* settings, const QString& titleText);
		void printPreview(KGVSimplePrintingSettings* settings, const QString& titleText, bool reload);
		void print();
		void printPreview();
    void needsRedraw();

	protected slots:
		void slotSaveSetup();
		void slotChangeTitleFont();
		void slotChangePageSizeAndMargins();
		void slotAddPageNumbersCheckboxToggled(bool set);
		void slotAddDateTimeCheckboxToggled(bool set);
		void slotAddTableBordersCheckboxToggled(bool set);
		void slotTitleTextChanged(const QString&);
    void slotClose();
    void slotFittingButtonClicked(int id);
    void slotHorizFitChanged(int newValue);
    void slotVertFitChanged(int newValue);
    void slotMaintainAspectButtonToggled();

	protected:
		void setupPrintingCommand();
		void updatePageLayoutAndUnitInfo();
		void setDirty(bool set);

		KGVSimplePrintingSettings* m_settings;

//		KGVSimplePrintingEngine *m_engine;
//    get engine with m_command->engine()

		KgvUnit::Unit m_unit;
		KGVSimplePrintingPageSetupBase *m_contents;
		KgvPageLayoutSize *m_pageLayoutWidget;
		DotGraphView *m_graphView;
		KGVSimplePrintingCommand *m_command;
		QString m_origCaptionLabelText;
		bool m_printPreviewNeedsReloading : 1;
  
    QButtonGroup m_fittingModeButtons;
};

#endif
