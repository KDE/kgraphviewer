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
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#ifndef KGVSIMPLEPRINTPREVIEWWINDOW_H
#define KGVSIMPLEPRINTPREVIEWWINDOW_H

#include <qpainter.h>
//Added by qt3to4:
#include <QEvent>
#include <ktoolbar.h>
#include <kactioncollection.h>
#include <KgvPageLayoutDia.h>

class QLabel;
class QScrollArea;

class KGVSimplePrintPreviewScrollView;
class KGVSimplePrintPreviewView;
class KGVSimplePrintingSettings;
class KGVSimplePrintingEngine;

//! @short A window for displaying print preview for simple printing.
class KGVSimplePrintPreviewWindow : public QWidget
{
	Q_OBJECT

	public:
		KGVSimplePrintPreviewWindow(KGVSimplePrintingEngine &engine,
			const QString& previewName, QWidget *parent, Qt::WFlags f = 0);
		~KGVSimplePrintPreviewWindow();

		int currentPage() const { return m_pageNumber; }

		KGVSimplePrintingSettings* settings() const { return m_settings; }

	public slots:
		void updatePagesCount();
//		void setPagesCount(int pagesCount);
		void goToPage(int pageNumber);
    void setFullWidth();
    void slotRedraw();

	signals:
		void printRequested();
		void pageSetupRequested();

	protected slots:
		void slotPageSetup();
		void slotPrintClicked();
		void slotZoomInClicked();
		void slotZoomOutClicked();
		void slotFirstClicked();
		void slotPreviousClicked();
		void slotNextClicked();
		void slotLastClicked();
		void initLater();

	protected:
		virtual bool event( QEvent * e );

		KGVSimplePrintingEngine &m_engine;
		KGVSimplePrintingSettings* m_settings;
		KToolBar *m_toolbar, *m_navToolbar;
		int m_pageNumber;//, m_pagesCount;
		int m_idFirst, m_idLast, m_idPrevious, m_idNext;
    QLabel* m_pageNumberLabel;
//     QScrollArea* m_scrollView;
		KGVSimplePrintPreviewScrollView *m_scrollView;
		KGVSimplePrintPreviewView *m_view;
    KActionCollection m_actions;

		friend class KGVSimplePrintPreviewView;
};

#endif
