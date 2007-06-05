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

#include "simpleprintingcommand.h"
#include "simpleprintingsettings.h"
#include "simpleprintingpagesetup.h"
#include "simpleprintpreviewwindow.h"

// #include <core/keximainwindow.h>
// #include <kexiutils/utils.h>
// #include <kexi_version.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <kurllabel.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kprinter.h>
#include <kpushbutton.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <qlabel.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qslider.h>

#include <iostream>

KGVSimplePrintingCommand::KGVSimplePrintingCommand(
	DotGraphView* mainWin, int objectId, QObject* parent)
 : QObject(parent, "KGVSimplePrintCommand")
 , m_previewEngine(0)
 , m_graphView(mainWin)
 , m_objectId(objectId)
 , m_previewWindow(0)
 , m_printPreviewNeedsReloading(true)
 , m_pageSetupDialog(0)
 , m_settings(new KGVSimplePrintingSettings(KGVSimplePrintingSettings::load()))
{
	connect(this, SIGNAL(showPageSetupRequested()), 
		this, SLOT(slotShowPageSetupRequested()));
}

KGVSimplePrintingCommand::~KGVSimplePrintingCommand()
{
	delete m_previewWindow;
	delete m_previewEngine;
	delete m_settings;
}

bool KGVSimplePrintingCommand::init(const QString& aTitleText)
{
  if (!m_previewEngine)
    m_previewEngine = new KGVSimplePrintingEngine(m_settings, this);

  bool backToPage0 = true;
  QString titleText(aTitleText.stripWhiteSpace());
  if (!m_previewWindow) 
  {
    backToPage0 = false;
    QString errorMessage;
    if (!m_previewEngine->init(
      *m_graphView, titleText, errorMessage)) {
      if (!errorMessage.isEmpty())
        KMessageBox::sorry(m_graphView, errorMessage, i18n("Print Preview")); 
      return false;
    }
    m_previewWindow = new KGVSimplePrintPreviewWindow(
      *m_previewEngine, "", 0, 
      Qt::WStyle_Customize|Qt::WStyle_NormalBorder|Qt::WStyle_Title|
      Qt::WStyle_SysMenu|Qt::WStyle_MinMax|Qt::WStyle_ContextHelp);
    connect(m_previewWindow, SIGNAL(printRequested()), this, SLOT(print()));
    connect(m_previewWindow, SIGNAL(pageSetupRequested()), this, SLOT(slotShowPageSetupRequested()));
//     KDialog::centerOnScreen(m_previewWindow);
    m_printPreviewNeedsReloading = false;
  }
  return true;
}

bool KGVSimplePrintingCommand::print(const QString& aTitleText)
{
  init(aTitleText);
  m_previewEngine->clear();

	//setup printing
	KPrinter printer;
	printer.setOrientation( m_settings->pageLayout.orientation == PG_PORTRAIT 
		? KPrinter::Portrait : KPrinter::Landscape );
	printer.setPageSize( 
		(KPrinter::PageSize)KgvPageFormat::printerPageSize( m_settings->pageLayout.format ) );
// #endif

	printer.setFullPage(true);
	QString docName( aTitleText );
	printer.setDocName( docName );
	printer.setCreator("kgraphviewer");
	if ( !printer.setup( m_graphView ) ) {
		return true;
	}

	// now we have final settings

  //! @todo get printer.pageOrder() (for reversed order requires improved engine)
	QPainter painter;

	if (!painter.begin(&printer)) 
  {
    //! @todo msg
		return false;
	}
	m_previewEngine->calculatePagesCount(painter);

	uint loops, loopsPerPage;
	QValueList<int> pagesToPrint;
	int fromPage = 0;
	// on !win32 print QPrinter::numCopies() times (the OS does not perform buffering)
	pagesToPrint = printer.pageList();
	kdDebug() << pagesToPrint << endl;
	if (pagesToPrint.isEmpty()) 
  {
		fromPage = 0;
		for (int i = 0; i<(int)m_previewEngine->pagesCount(); i++) 
    {
//       std::cerr << "Page " << i << " has to be printed" << std::endl;
			pagesToPrint.append(i);
		}
	}
	else
		fromPage = pagesToPrint.first();
	if (printer.collate()==KPrinter::Collate) 
  {
		//collation: p1, p2,..pn; p1, p2,..pn; ......; p1, p2,..pn
		loops = printer.numCopies();
		loopsPerPage = 1;
	}
	else 
  {
		//no collation: p1, p1, ..., p1; p2, p2, ..., p2; ......; pn, pn,..pn
		loops = 1; 
		loopsPerPage = printer.numCopies();
	}
  //! @todo also look at printer.pageSet() option : all/odd/even pages
// #endif
	// now, total number of printed pages is printer.numCopies()*printer.pageList().count()

// 	kdDebug() << "printing..." << endl;
	bool firstPage = true;
	for (uint copy = 0;copy < loops; copy++) 
  {
// 		kdDebug() << "copy " << (copy+1) << " of " << loops << endl;
//     std::cerr << "fromPage = " << fromPage << " ; eof = " << m_previewEngine->eof() << std::endl;
		uint pageNumber = fromPage;
		QValueList<int>::ConstIterator pagesIt = pagesToPrint.constBegin();
		for(;(int)pageNumber == fromPage || !m_previewEngine->eof(); ++pageNumber) 
    {
// 			std::cerr << "printing..." << std::endl;
			if (pagesIt == pagesToPrint.constEnd()) //no more pages to print
				break;
			if ((int)pageNumber < *pagesIt) 
      { //skip pages without printing (needed for computation)
				m_previewEngine->paintPage(pageNumber, painter, false);
				continue;
			}
			if (*pagesIt < (int)pageNumber) 
      { //sanity
				++pagesIt;
				continue;
			}
			for (uint onePageCounter = 0; onePageCounter < loopsPerPage; onePageCounter++) 
      {
				if (!firstPage)
					printer.newPage();
				else
					firstPage = false;
// 				std::cerr << "page #" << pageNumber << std::endl;
				m_previewEngine->paintPage(pageNumber, painter);
			}
			++pagesIt;
		}
	}
// 	kdDebug() << "end of printing." << endl;

	// stop painting, this will automatically send the print data to the printer
	if (!painter.end())
		return false;

// 	if (!m_previewEngine->done())
// 		return false;

	return true;
}

bool KGVSimplePrintingCommand::showPrintPreview(const QString& aTitleText, bool reload)
{
  init(aTitleText);
  
  if (reload)
    m_printPreviewNeedsReloading = true;
  
  if (m_printPreviewNeedsReloading) 
  {//dirty
    m_previewEngine->clear();
    //! @todo progress bar...
    m_previewEngine->setTitleText( aTitleText );
    m_previewWindow->setFullWidth();
    m_previewWindow->updatePagesCount();
    m_printPreviewNeedsReloading = false;
    m_previewWindow->goToPage(0);
  }
  m_previewWindow->show();
  m_previewWindow->raise();
  return true;
}

void KGVSimplePrintingCommand::hidePageSetup()
{
  if (m_pageSetupDialog != 0)
  {
    m_pageSetupDialog->hide();
  }
}

void KGVSimplePrintingCommand::hidePrintPreview()
{
  if (m_previewWindow != 0)
  {
    m_previewWindow->hide();
  }
}

void KGVSimplePrintingCommand::slotShowPageSetupRequested()
{
  if (m_pageSetupDialog == 0)
  {
    m_pageSetupDialog = new QDialog(0,"glurp",false);
    QMap<QString,QString> map;
    map["action"]=="pageSetup";
    map["title"]==m_graphView->dotFileName();
    QVBoxLayout *lyr = new QVBoxLayout(m_pageSetupDialog);
    KGVSimplePrintingPageSetup* sppsb = new KGVSimplePrintingPageSetup(this, m_graphView, m_pageSetupDialog, &map);
    if (m_previewWindow != 0)
    {
      connect(sppsb,SIGNAL(needsRedraw()),m_previewWindow, SLOT(slotRedraw()));
    }
    lyr->addWidget(sppsb);
    m_pageSetupDialog->show();
  }
  else
  {
    m_pageSetupDialog->show();
  }
  m_pageSetupDialog->raise();
}

void KGVSimplePrintingCommand::showPageSetup(const QString& aTitleText)
{
  init(aTitleText);
  emit showPageSetupRequested();
}
#include "simpleprintingcommand.moc"
