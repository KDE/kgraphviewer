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

#include "simpleprintpreviewwindow_p.h"
#include "simpleprintingengine.h"
#include "simpleprintingsettings.h"

KGVSimplePrintPreviewView::KGVSimplePrintPreviewView(
  QWidget *parent, KGVSimplePrintPreviewWindow *window)
 : QWidget(parent, "KGVSimplePrintPreviewView", WStaticContents)//|WNoAutoErase)
 , m_window(window)
{
  enablePainting = false;
//      resize(300,400);
//      resizeContents(200, 400);
}

void KGVSimplePrintPreviewView::paintEvent( QPaintEvent *pe )
{
//   std::cerr << "KGVSimplePrintPreviewView::paintEvent" << std::endl;
  Q_UNUSED(pe);
  if (!enablePainting)
    return;
  QPixmap pm(size()); //dbl buffered
  QPainter p;
  p.begin(&pm, this);
//! @todo only for screen!
  p.fillRect(QRect(QPoint(0,0),pm.size()), QBrush(white));//pe->rect(), QBrush(white));
  if (m_window->currentPage()>=0)
    m_window->m_engine.paintPage(m_window->currentPage(), p);
//    emit m_window->paintingPageRequested(m_window->currentPage(), p);
  p.end();
  bitBlt(this, 0, 0, &pm);
}

#define KGVSimplePrintPreviewScrollView_MARGIN KDialogBase::marginHint()

KGVSimplePrintPreviewScrollView::KGVSimplePrintPreviewScrollView(
  KGVSimplePrintPreviewWindow *window)
 : QScrollView(window, "scrollview", WStaticContents|WNoAutoErase)
 , m_window(window)
{
//      this->settings = settings;
  widget = new KGVSimplePrintPreviewView(viewport(), m_window);

/*      int widthMM = KgvPageFormat::width( 
    settings.pageLayout.format, settings.pageLayout.orientation);
  int heightMM = KgvPageFormat::height( 
    settings.pageLayout.format, settings.pageLayout.orientation);
//      int constantHeight = 400;
//      widget->resize(constantHeight * widthMM / heightMM, constantHeight ); //keep aspect
*/
  addChild(widget);
}

void KGVSimplePrintPreviewScrollView::resizeEvent( QResizeEvent *re )
{
  QScrollView::resizeEvent(re);
//  kdDebug() << re->size().width() << " " << re->size().height() << endl;
//  kdDebug() << contentsWidth() << " " << contentsHeight() << endl;
//  kdDebug() << widget->width() << " " << widget->height() << endl;
  setUpdatesEnabled(false);
  if (re->size().width() > (widget->width()+2*KGVSimplePrintPreviewScrollView_MARGIN)
    || re->size().height() > (widget->height()+2*KGVSimplePrintPreviewScrollView_MARGIN)) {
    resizeContents(
      QMAX(re->size().width(), widget->width()+2*KGVSimplePrintPreviewScrollView_MARGIN),
      QMAX(re->size().height(), widget->height()+2*KGVSimplePrintPreviewScrollView_MARGIN));
    int vscrbarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    moveChild(widget, (contentsWidth() - vscrbarWidth - widget->width())/2, 
      (contentsHeight() - widget->height())/2);
  }
  setUpdatesEnabled(true);
}

void KGVSimplePrintPreviewScrollView::setFullWidth()
{
  viewport()->setUpdatesEnabled(false);
  double widthMM = KgvPageFormat::width( 
    m_window->settings()->pageLayout.format, 
    m_window->settings()->pageLayout.orientation);
  double heightMM = KgvPageFormat::height( 
    m_window->settings()->pageLayout.format, m_window->settings()->pageLayout.orientation);
//  int constantWidth = m_window->width()- KGVSimplePrintPreviewScrollView_MARGIN*6;
  double constantWidth = width()- KGVSimplePrintPreviewScrollView_MARGIN*6;
  double heightForWidth = constantWidth * heightMM / widthMM;
//  heightForWidth = QMIN(kapp->desktop()->height()*4/5, heightForWidth);
  constantWidth = heightForWidth * widthMM / heightMM;
  widget->resize((int)constantWidth, (int)heightForWidth); //keep aspect
  resizeContents(int(widget->width() + 2*KGVSimplePrintPreviewScrollView_MARGIN), 
    int(widget->height() + 2*KGVSimplePrintPreviewScrollView_MARGIN));
  moveChild(widget, (contentsWidth()-widget->width())/2, 
    (contentsHeight()-widget->height())/2);
  viewport()->setUpdatesEnabled(true);
  resize(size()+QSize(1,1)); //to update pos.
  widget->enablePainting = true;
  widget->repaint();
}

void KGVSimplePrintPreviewScrollView::setContentsPos(int x, int y)
{
//  kdDebug() << "############" << x << " " << y << " " << contentsX()<< " " <<contentsY() << endl;
  if (x<0 || y<0) //to avoid endless loop on Linux
    return;
  QScrollView::setContentsPos(x,y);
}

#include "simpleprintpreviewwindow_p.moc"
