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

#include "simpleprintpreviewwindow.h"
#include "simpleprintingengine.h"
#include "simpleprintpreviewwindow_p.h"
// #include <kexi_version.h>

#include <qlayout.h>
#include <qaccel.h>
#include <qtimer.h>
#include <qlabel.h>

#include <kdialogbase.h>
#include <ktoolbarbutton.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kapplication.h>

#include <iostream>

KGVSimplePrintPreviewWindow::KGVSimplePrintPreviewWindow(
	KGVSimplePrintingEngine &engine, const QString& previewName, 
	QWidget *parent, WFlags f)
 : QWidget(parent, "KGVSimplePrintPreviewWindow", f)
 , m_engine(engine)
 , m_settings(m_engine.settings())
 , m_pageNumber(-1)
{
//	m_pagesCount = INT_MAX;

	setCaption(i18n("%1 - Print Preview - %2").arg(previewName).arg(""));
	setIcon(DesktopIcon("filequickprint"));
	QVBoxLayout *lyr = new QVBoxLayout(this, 6);

	int id;
	m_toolbar = new KToolBar(0, this);
	m_toolbar->setLineWidth(0);
	m_toolbar->setFrameStyle(QFrame::NoFrame);
	m_toolbar->setIconText(KToolBar::IconTextRight);
	lyr->addWidget(m_toolbar);

	id = m_toolbar->insertWidget( -1, 0, new KPushButton(KStdGuiItem::print(), m_toolbar) );
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotPrintClicked()));
	static_cast<KPushButton*>(m_toolbar->getWidget(id))->setAccel(Qt::CTRL|Qt::Key_P);
	m_toolbar->insertSeparator();

	id = m_toolbar->insertWidget(-1, 0, new KPushButton(i18n("Page Set&up..."), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotPageSetup()));
	m_toolbar->insertSeparator();


#ifndef KGV_NO_UNFINISHED 
//! @todo unfinished
	id = m_toolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("viewmag+"), i18n("Zoom In"), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotZoomInClicked()));
	m_toolbar->insertSeparator();

	id = m_toolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("viewmag-"), i18n("Zoom Out"), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotZoomOutClicked()));
	m_toolbar->insertSeparator();
#endif

	id = m_toolbar->insertWidget(-1, 0, new KPushButton(KStdGuiItem::close(), m_toolbar));
	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(close()));
	m_toolbar->alignItemRight(id);

	m_scrollView = new KGVSimplePrintPreviewScrollView(this);
	m_scrollView->setUpdatesEnabled(false);
	m_view = m_scrollView->widget;
	m_scrollView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	lyr->addWidget(m_scrollView);

	m_navToolbar = new KToolBar(0, this);
//	m_navToolbar->setFullWidth(true);
	m_navToolbar->setLineWidth(0);
	m_navToolbar->setFrameStyle(QFrame::NoFrame);
	m_navToolbar->setIconText(KToolBar::IconTextRight);
	lyr->addWidget(m_navToolbar);

	m_idFirst = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("start"), i18n("First Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idFirst, SIGNAL(clicked()), this, SLOT(slotFirstClicked()));
	m_navToolbar->insertSeparator();

	m_idPrevious = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("previous"), i18n("Previous Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idPrevious, SIGNAL(clicked()), this, SLOT(slotPreviousClicked()));
	m_navToolbar->insertSeparator();

	m_idPageNumberLabel = m_navToolbar->insertWidget( -1, 0, new QLabel(m_navToolbar));
	m_navToolbar->insertSeparator();

	m_idNext = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("next"), i18n("Next Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idNext, SIGNAL(clicked()), this, SLOT(slotNextClicked()));
	m_navToolbar->insertSeparator();

	m_idLast = m_navToolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("finish"), i18n("Last Page"), m_navToolbar));
	m_navToolbar->addConnection(m_idLast, SIGNAL(clicked()), this, SLOT(slotLastClicked()));
	m_navToolbar->insertSeparator();

	resize(width(), kapp->desktop()->height()*4/5);

//! @todo progress bar...

	QTimer::singleShot(50, this, SLOT(initLater()));
}

void KGVSimplePrintPreviewWindow::initLater()
{
	setFullWidth();
	updatePagesCount();
	goToPage(0);
}

KGVSimplePrintPreviewWindow::~KGVSimplePrintPreviewWindow()
{
}

void KGVSimplePrintPreviewWindow::slotPrintClicked()
{
	hide();
	emit printRequested();
	show();
	raise();
}

void KGVSimplePrintPreviewWindow::slotPageSetup()
{
	lower();
	emit pageSetupRequested();
}

void KGVSimplePrintPreviewWindow::slotZoomInClicked()
{
	//! @todo
}

void KGVSimplePrintPreviewWindow::slotZoomOutClicked()
{
	//! @todo
}

void KGVSimplePrintPreviewWindow::slotFirstClicked()
{
	goToPage(0);
}

void KGVSimplePrintPreviewWindow::slotPreviousClicked()
{
	goToPage(m_pageNumber-1);
}

void KGVSimplePrintPreviewWindow::slotNextClicked()
{
	goToPage(m_pageNumber+1);
}

void KGVSimplePrintPreviewWindow::slotLastClicked()
{
  goToPage(m_engine.pagesCount()-1);
}

void KGVSimplePrintPreviewWindow::slotRedraw()
{
//   std::cerr << "KGVSimplePrintPreviewWindow::slotRedraw" << std::endl;

  m_engine.clear();
  setFullWidth();
  updatePagesCount();
  m_pageNumber = 0;
  m_view->repaint(); //this will automatically paint a new page

  m_navToolbar->setItemEnabled(m_idNext, m_pageNumber < ((int)m_engine.pagesCount()-1));
  m_navToolbar->setItemEnabled(m_idLast, m_pageNumber < ((int)m_engine.pagesCount()-1));
  m_navToolbar->setItemEnabled(m_idPrevious, m_pageNumber > 0);
  m_navToolbar->setItemEnabled(m_idFirst, m_pageNumber > 0);
  static_cast<QLabel*>(m_navToolbar->getWidget(m_idPageNumberLabel))->setText(
    i18n("Page (number) of (total)", "Page %1 of %2").arg(m_pageNumber+1).arg(m_engine.pagesCount()));
}

void KGVSimplePrintPreviewWindow::goToPage(int pageNumber)
{
	if (pageNumber==m_pageNumber || pageNumber < 0 || pageNumber > ((int)m_engine.pagesCount()-1))
		return;
	m_pageNumber = pageNumber;

	m_view->repaint(); //this will automatically paint a new page
//	if (m_engine.eof())
//		m_pagesCount = pageNumber+1;

	m_navToolbar->setItemEnabled(m_idNext, pageNumber < ((int)m_engine.pagesCount()-1));
	m_navToolbar->setItemEnabled(m_idLast, pageNumber < ((int)m_engine.pagesCount()-1));
	m_navToolbar->setItemEnabled(m_idPrevious, pageNumber > 0);
	m_navToolbar->setItemEnabled(m_idFirst, pageNumber > 0);
	static_cast<QLabel*>(m_navToolbar->getWidget(m_idPageNumberLabel))->setText(
		i18n("Page (number) of (total)", "Page %1 of %2").arg(m_pageNumber+1).arg(m_engine.pagesCount()));
}

void KGVSimplePrintPreviewWindow::setFullWidth()
{
	m_scrollView->setFullWidth();
}

void KGVSimplePrintPreviewWindow::updatePagesCount()
{
	QPixmap pm(m_view->size()); //dbl buffered
	QPainter p(m_view);
	//p.begin(&pm, this);
////! @todo only for screen!
//	p.fillRect(pe->rect(), QBrush(white));
	m_engine.calculatePagesCount(p);
	p.end();
}

bool KGVSimplePrintPreviewWindow::event( QEvent * e )
{
	QEvent::Type t = e->type();
	if (t==QEvent::KeyPress) {
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);
		const int k = ke->key();
		bool ok = true;
		if (k==Qt::Key_Equal || k==Qt::Key_Plus)
			slotZoomInClicked();
		else if (k==Qt::Key_Minus)
			slotZoomOutClicked();
		else if (k==Qt::Key_Home)
			slotFirstClicked();
		else if (k==Qt::Key_End)
			slotLastClicked();
		else
			ok = false;

		if (ok) {
			ke->accept();
			return true;
		}
	}
	else if (t==QEvent::AccelOverride) {
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);
		const int k = ke->key();
		bool ok = true;
		if (k==Qt::Key_PageUp)
			slotPreviousClicked();
		else if (k==Qt::Key_PageDown)
			slotNextClicked();
		else
			ok = false;

		if (ok) {
			ke->accept();
			return true;
		}
	}
	return QWidget::event(e);
}


#include "simpleprintpreviewwindow.moc"
