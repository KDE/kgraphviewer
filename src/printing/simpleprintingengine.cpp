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

#include "simpleprintingengine.h"
#include "simpleprintingsettings.h"

// #include <core/keximainwindow.h>
// #include <kexiutils/utils.h>

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <kurllabel.h>
#include <kdebug.h>
#include <kconfig.h>

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qpaintdevicemetrics.h>

#include <kprinter.h>

#include <iostream>
#include <math.h>

KGVSimplePrintingEngine::KGVSimplePrintingEngine(
                                    KGVSimplePrintingSettings* settings, 
                                    QObject* parent) : 
    QObject(parent, "KGVSimplePrintingEngine"), m_settings(settings), m_pdm(0), 
    m_data(0)
{
  clear(); 
}

KGVSimplePrintingEngine::~KGVSimplePrintingEngine()
{
  done();
}

bool KGVSimplePrintingEngine::init(DotGraphView& data, const QString& titleText, QString& errorMessage)
{
	errorMessage = QString();
	done();
	m_headerText = titleText; 

  m_data = &data;
  m_eof = false;
  
  m_painting.resize(m_data->canvas()->size());
  QPainter p(&m_painting);
  m_data->canvas()->drawArea( m_data->canvas()->rect(), &p );
  
	return true;
}

bool KGVSimplePrintingEngine::done()
{
	bool result = true;
	m_data = 0;
	m_pagesCount = 0;
	m_paintInitialized = false;
	return result;
}

void KGVSimplePrintingEngine::clear()
{
	m_eof = false;
	m_pagesCount = 0;
	m_paintInitialized = false;
}

void KGVSimplePrintingEngine::paintPage(int pageNumber, QPainter& painter, bool paint)
{
	std::cerr << "KGVSimplePrintingEngine::paintPage" << std::endl;
	uint offset = 0;

	uint y = 0;

  if (pageNumber <= m_pagesCount)
  {
    m_eof = false;
  }
	const bool printer = painter.device()->devType() == QInternal::Printer;

	int w = 0, h = 0;
	m_pdm = QPaintDeviceMetrics( painter.device() );
	
	if (dynamic_cast<QWidget*>(painter.device())) {
		w = dynamic_cast<QWidget*>(painter.device())->width();
		h = dynamic_cast<QWidget*>(painter.device())->height();
	}
	else if (dynamic_cast<QPixmap*>(painter.device())) {
		w = dynamic_cast<QPixmap*>(painter.device())->width();
		h = dynamic_cast<QPixmap*>(painter.device())->height();
	}
	else {//KPrinter...
		w = m_pdm.widthMM();
		h = m_pdm.heightMM();
	}

	if (!m_paintInitialized) 
  {
		m_paintInitialized = true;

		double widthMM = KgvPageFormat::width( 
			m_settings->pageLayout.format, m_settings->pageLayout.orientation);
		double heightMM = KgvPageFormat::height( 
			m_settings->pageLayout.format, m_settings->pageLayout.orientation);

		m_dpiY = m_pdm.logicalDpiY();
		m_dpiX = m_pdm.logicalDpiX();
#ifdef Q_WS_WIN //fix for 120dpi
		if (!printer) 
    {
//			m_dpiY = 96;
//			m_dpiX = 96;
			m_dpiY = 86;
			m_dpiX = 86;
		}
#endif
		int pdWidthMM = m_pdm.widthMM();
		int pdHeightMM = m_pdm.heightMM();

		double screenF;
//		if (printer)
			screenF = 1.0;
//		else
//			screenF = (double)96.0/120.0;

		leftMargin = POINT_TO_INCH(m_settings->pageLayout.ptLeft)*m_dpiX* screenF;
		rightMargin = POINT_TO_INCH(m_settings->pageLayout.ptRight)*m_dpiX* screenF;
		topMargin = POINT_TO_INCH(m_settings->pageLayout.ptTop)*m_dpiY* screenF;
		bottomMargin = POINT_TO_INCH(m_settings->pageLayout.ptBottom)*m_dpiY* screenF;
		
		m_fx = widthMM / (pdWidthMM * screenF);
		m_fy = heightMM / (pdHeightMM * screenF);

	//screen only
	//	painter.fillRect(QRect(0,0,w,h), QBrush(white));
		m_pageWidth = int( m_fx*(double)m_pdm.width() - leftMargin - rightMargin);
		m_pageHeight = int( m_fy*(double)m_pdm.height() - topMargin - bottomMargin);

//! @todo add setting
		m_mainFont = m_settings->pageTitleFont; 
		if(!printer) 
    {
			int pixelSize = int( POINT_TO_INCH(m_mainFont.pointSizeFloat())*m_dpiX );
			m_mainFont.setPixelSize(pixelSize);
		}
		painter.setFont(m_mainFont);

		m_dateTimeText = KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(),
			true, false);
		m_dateTimeWidth = painter.fontMetrics().width(m_dateTimeText+"   ");
		m_mainLineSpacing = painter.fontMetrics().lineSpacing();
		m_footerHeight = m_mainLineSpacing * 2; //2 lines
		painter.setFont(m_mainFont);
		m_headerTextRect = painter.fontMetrics().boundingRect(
			(int)leftMargin, (int)topMargin,
			m_pageWidth - m_dateTimeWidth,
			m_pageHeight, Qt::AlignAuto|Qt::WordBreak, m_headerText);
		m_headerTextRect.setRight(m_headerTextRect.right()+10);
		m_headerTextRect.setWidth(
			QMIN(int(m_pageWidth - m_dateTimeWidth), m_headerTextRect.width()));

		painter.setFont(m_mainFont);
  }
  //screen only
  if(!printer) 
  {
    painter.setWindow(0, 0, int((double)w*m_fx), int((double)h*m_fy));
  }
  
  //paint header
  painter.setFont(m_mainFont);
  if (paint) 
  {
    painter.drawText(m_headerTextRect, Qt::AlignAuto|Qt::WordBreak, m_headerText);
  }
  painter.setFont(m_mainFont);
  if (paint) 
  {
    if (m_settings->addDateAndTime)
    {
      painter.drawText(
          (int)leftMargin + m_pageWidth - m_dateTimeWidth, (int)topMargin, 
          m_dateTimeWidth, m_headerTextRect.height(), 
          Qt::AlignRight, m_dateTimeText);
    }

    //footer
    if (m_settings->addPageNumbers)
    {
      QString pageNumString;
      if (m_pagesCount>0)
      {
        pageNumString = i18n("Page (number) of (total)", "Page %1 of %2")
          .arg(pageNumber+1).arg(m_pagesCount);
      }
      else
      {
        pageNumString = i18n("Page %1").arg(pageNumber+1);
      }
      painter.drawText((int)leftMargin, 
        (int)topMargin + m_pageHeight - m_mainLineSpacing, 
        m_pageWidth, m_mainLineSpacing,
        Qt::AlignRight | Qt::AlignBottom, pageNumString);
    }
  }

  w = m_pageWidth;
  h = m_pageHeight;
  y = (int)topMargin ;
  if (m_settings->addDateAndTime)
  {
    h -= (m_headerTextRect.height() + 1);
    y += (m_headerTextRect.height() );
  }
  if (m_settings->addTableBorders)
  {
    w -= 2; h -= 2;
  }
  if (m_settings->addPageNumbers)
  {
    h -= (m_mainLineSpacing*3/2 + 1);
  }

  std::cerr << "(w, h) = (" << w << ", " << h <<")" << std::endl;
  if ( ( (m_settings->fitToOnePage) || 
    (m_painting.width()<=w && m_painting.height()<=h) )
       && !m_eof)
  {
    std::cerr << "single-page printing" << std::endl;
    if (paint)
    {
      QImage img = m_painting.convertToImage();
      QPixmap pix;
      pix.convertFromImage(img.smoothScale(w, h, QImage::ScaleMin));
      painter.drawPixmap((int)leftMargin, y, pix);
    }
    m_eof = true;
  }
  else if (m_settings->horizFitting != 0 || m_settings->vertFitting != 0)
  {
    std::cerr << "fitted multi-pages printing page " << pageNumber << std::endl;
    int nbTilesByRow = (int)(ceil((double)m_painting.width())/w) + 1;
    std::cerr << "  nb tiles by row = " << nbTilesByRow << std::endl;

    int tileWidth = w;
    int tileHeight = h;
  
    if (m_settings->horizFitting != 0)
    {
      tileWidth = int(ceil(((double)m_painting.width())/m_settings->horizFitting));
      nbTilesByRow = m_settings->horizFitting;
    }
    if (m_settings->vertFitting != 0)
    {
      tileHeight = int(ceil(((double)m_painting.height())/m_settings->vertFitting));
    }
    std::cerr << "  tile size = "<<tileWidth<<"/"<<tileHeight<<"" << std::endl;
    int rowNum = pageNumber / nbTilesByRow;
    int columnNum = pageNumber % nbTilesByRow;
    int x1, y1, x2, y2;
    x1 = int(tileWidth * columnNum);
    x2 = int(tileWidth * (columnNum+1));
    y1 = int(tileHeight * rowNum);
    y2 = int(tileHeight * (rowNum+1));
    std::cerr << "(x1, y1, x2, 2) = ("<<x1<<","<<y1<<","<<x2<<","<<y2<<")" << std::endl;
    std::cerr << "painting size = ("<<m_painting.width()<<"/"<<m_painting.height()<<")" << std::endl;
    if (paint)
    {
      QImage::ScaleMode scaleMode = QImage::ScaleFree;
      if (m_settings->chainedFittings)
      {
        scaleMode = QImage::ScaleMin;
      }
      QImage img = m_painting.convertToImage();
      QImage toPaint = img.copy(x1, y1, (x2-x1+1), (y2-y1+1));
      QPixmap pix;
      if (m_settings->horizFitting == 0)
      {
        pix.convertFromImage(toPaint.smoothScale(img.width(), h, scaleMode));
      }
      else if (m_settings->vertFitting == 0)
      {
        pix.convertFromImage(toPaint.smoothScale(w, img.height(), scaleMode));
      }
      else
      {
        pix.convertFromImage(toPaint.smoothScale(w, h, scaleMode));
      }
      painter.drawPixmap((int)leftMargin, y, pix);
    }
    if ( x2 >= m_painting.width() && y2 >= m_painting.height() )
    {
      m_eof = true;
    }
  }
  else
  {
    std::cerr << "multi-pages printing page " << pageNumber << std::endl;
    int nbTilesByRow = (int)(((double)m_painting.width())/w) + 1;
    int rowNum = pageNumber / nbTilesByRow;
    int columnNum = pageNumber % nbTilesByRow;
    int x1, y1, x2, y2;
    x1 = int(w * columnNum);
    x2 = int(w * (columnNum+1));
    y1 = int(h * rowNum);
    y2 = int(h * (rowNum+1));
    std::cerr << "(x1, y1, x2, 2) = ("<<x1<<","<<y1<<","<<x2<<","<<y2<<")" << std::endl;
    if (paint)
    {
      QImage img = m_painting.convertToImage();
      QImage toPaint = img.copy(x1, y1, (x2-x1+1), (y2-y1+1));
      QPixmap pix;
      pix.convertFromImage(toPaint);
      painter.drawPixmap((int)leftMargin, y, pix);
    }
    if ( x2 >= m_painting.width() && y2 >= m_painting.height() )
    {
      m_eof = true;
    }
  }

  if (m_settings->addTableBorders) 
  {
    int y1 = (int)topMargin ;
    if (m_settings->addDateAndTime)
    {
      y1 += (m_headerTextRect.height());
    }
    int y2 = (int)topMargin + m_pageHeight;
    if (m_settings->addPageNumbers)
    {
      y2 -= (m_headerTextRect.height() /*+ m_mainLineSpacing*3/2*/ + 1);
    }
    if (paint)
    {
      painter.drawLine((int)leftMargin, y1, (int)leftMargin + m_pageWidth-1, y1);
      painter.drawLine((int)leftMargin + m_pageWidth-1, y1, (int)leftMargin + m_pageWidth-1, y2);
      painter.drawLine((int)leftMargin + m_pageWidth-1, y2, (int)leftMargin, y2);
      painter.drawLine((int)leftMargin, y2, (int)leftMargin, y1);
    }
  }

}

void KGVSimplePrintingEngine::calculatePagesCount(QPainter& painter)
{
//   std::cerr << "KGVSimplePrintingEngine::calculatePagesCount" << std::endl;
	if (m_eof || !m_data) {
		m_pagesCount = 0;
		return;
	}

	uint pageNumber = 0;
  if (m_settings->fitToOnePage)
  {
    m_pagesCount = 1;
  }
  else
  {
    for(;!m_eof; ++pageNumber) {
      paintPage(pageNumber, painter, false /* !paint */);
    }
    m_pagesCount = pageNumber;
  }
}

void KGVSimplePrintingEngine::setTitleText(const QString& titleText)
{
	m_headerText = titleText;
}

uint KGVSimplePrintingEngine::maxHorizFit() const
{
  uint w = m_pageWidth;
  if (m_settings->addTableBorders)
  {
    w -= 2; 
  }
//   std::cerr << "maxHorizFit: " << m_painting.width() << " / " << w
//     << " = " << m_painting.width()/w << std::endl;
  return (uint)ceil(((double)m_painting.width())/w) + 1;
}

uint KGVSimplePrintingEngine::maxVertFit() const
{
  uint h = m_pageHeight;
  if (m_settings->addDateAndTime)
  {
    h -= (m_headerTextRect.height() + 1);
  }
  if (m_settings->addTableBorders)
  {
    h -= 2;
  }
  if (m_settings->addPageNumbers)
  {
    h -= (m_mainLineSpacing*3/2 + 1);
  }
//   std::cerr << "maxVertFit: " << m_painting.height() << " / " << h 
//     << " = " << m_painting.height()/h << std::endl;
  return (uint)ceil(((double)m_painting.height()))/h + 1;
}

#include "simpleprintingengine.moc"
