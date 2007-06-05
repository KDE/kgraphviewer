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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
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
//Added by qt3to4:
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>

KGVSimplePrintPreviewView::KGVSimplePrintPreviewView(
  KGVSimplePrintPreviewWindow *window)
 : QWidget(),
  m_window(window)
{
  kDebug() << "KGVSimplePrintPreviewView" << endl;
/*  resize(300,400);
  resizeContents(200, 400);*/
}

void KGVSimplePrintPreviewView::paintEvent( QPaintEvent *pe )
{
  kDebug() << "KGVSimplePrintPreviewView::paintEvent" << endl;
  Q_UNUSED(pe);
  QPainter p(m_window);
  p.setRenderHint(QPainter::Antialiasing);
//   p.fillRect(pe->rect(), QBrush(Qt::white));//pe->rect(), QBrush(white));
  if (m_window->currentPage()>=0)
    m_window->m_engine.paintPage(m_window->currentPage(), p);
//    emit m_window->paintingPageRequested(m_window->currentPage(), p);
}

#define KGVSimplePrintPreviewScrollView_MARGIN KDialog::marginHint()

KGVSimplePrintPreviewScrollView::KGVSimplePrintPreviewScrollView(
  KGVSimplePrintPreviewWindow *window) : QScrollArea(window), m_window(window)
{
//      this->settings = settings;
  m_view = new KGVSimplePrintPreviewView(m_window);

/*      int widthMM = KgvPageFormat::width( 
    settings.pageLayout.format, settings.pageLayout.orientation);
  int heightMM = KgvPageFormat::height( 
    settings.pageLayout.format, settings.pageLayout.orientation);
//      int constantHeight = 400;
//      m_view->resize(constantHeight * widthMM / heightMM, constantHeight ); //keep aspect
*/
  setWidget(m_view);
}

void KGVSimplePrintPreviewScrollView::paintEvent( QPaintEvent *pe )
{
  kDebug() << "KGVSimplePrintPreviewScrollView::paintEvent" << endl;
  QScrollArea::paintEvent(pe);
  ((KGVSimplePrintPreviewView*)widget())->paintEvent(pe);
}

// void KGVSimplePrintPreviewScrollView::resizeEvent( QResizeEvent *re )
// {
//   QScrollArea::resizeEvent(re);
// //  kDebug() << re->size().width() << " " << re->size().height() << endl;
// //  kDebug() << contentsWidth() << " " << contentsHeight() << endl;
// //  kDebug() << m_view->width() << " " << m_view->height() << endl;
//   setUpdatesEnabled(false);
//   if (re->size().width() > (m_view->width()+2*KGVSimplePrintPreviewScrollView_MARGIN)
//     || re->size().height() > (m_view->height()+2*KGVSimplePrintPreviewScrollView_MARGIN)) 
//   {
//     resizeContents(
//         QMAX(re->size().width(), m_view->width()+2*KGVSimplePrintPreviewScrollView_MARGIN),
//         QMAX(re->size().height(), m_view->height()+2*KGVSimplePrintPreviewScrollView_MARGIN));
//       
//     int vscrbarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
//     moveChild(m_view, (contentsWidth() - vscrbarWidth - m_view->width())/2, 
//       (contentsHeight() - m_view->height())/2);
//   }
//   setUpdatesEnabled(true);
// }

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
  m_view->resize((int)constantWidth, (int)heightForWidth); //keep aspect
/*  resizeContents(int(m_view->width() + 2*KGVSimplePrintPreviewScrollView_MARGIN), 
    int(m_view->height() + 2*KGVSimplePrintPreviewScrollView_MARGIN));*/
/*  moveChild(m_view, (contentsWidth()-m_view->width())/2, 
    (contentsHeight()-m_view->height())/2);*/
  viewport()->setUpdatesEnabled(true);
  resize(size()+QSize(1,1)); //to update pos.
//   m_view->enablePainting = true;
  m_view->repaint();
}

// void KGVSimplePrintPreviewScrollView::setContentsPos(int x, int y)
// {
// //  kDebug() << "############" << x << " " << y << " " << contentsX()<< " " <<contentsY() << endl;
//   if (x<0 || y<0) //to avoid endless loop on Linux
//     return;
//   QScrollArea::setContentsPos(x,y);
// }

#include "simpleprintpreviewwindow_p.moc"
