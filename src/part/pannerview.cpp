/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gaël de Chalendar <kleag@free.fr>

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

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QGraphicsScene>
#include <QPainter>
#include <QMouseEvent>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
//#include <ktempfile.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>

#include "pannerview.h"

//
// PannerView
//
PannerView::PannerView(QWidget * parent, const char * name)
  : QGraphicsView(parent), m_drawContents(true)
{
  m_movingZoomRect = false;

  // why doesn't this avoid flicker ?
  // viewport()->setBackgroundMode(Qt::NoBackground);
  setBackgroundMode(Qt::NoBackground);

  setToolTip(i18n("View of The complete graph. Click and drag to move this visible part."));
  setWhatsThis(i18n("<h1>View of The complete graph</h1>"
    "<p>Single click out of the red square to move the center of his one at the "
    "mouse position.</p><p>Click in the red square and drag the mouse and the "
    "red square will follow the movement.</p>"));
}

void PannerView::setZoomRect(QRectF r)
{
  kDebug() << "PannerView::setZoomRect " << r;
  QRectF oldRect = m_zoomRect;
  m_zoomRect = r;
  qreal q = mapToScene(15,0).x();
  qreal d = mapToScene(10,0).x();
  if (!m_zoomRect.isValid() || m_zoomRect.width() < q || m_zoomRect.height() < q) 
  {
    double factor = ((double)m_zoomRect.width())/m_zoomRect.height();
    qreal newWidth, newHeight;
    if (factor > 1.0)
    {
      newWidth = q;
      newHeight = newWidth/factor;
      if (newHeight < d) newHeight = d;
    }
    else
    {
      newHeight = q;
      newWidth = newHeight*factor;
      if (newWidth < d) newWidth = d;
    }
    QRectF newRect = m_zoomRect;
    qreal newXPos = newRect.x() + (newRect.width()/2) - newWidth/2;
    newXPos = (newXPos<0)?0:newXPos;
    newRect.setX(newXPos);
    qreal newYPos = newRect.y() + (newRect.height()/2) -newHeight/2;
    newYPos = (newYPos<0)?0:newYPos;
    newRect.setY(newYPos);
    newRect.setWidth(newWidth);
    newRect.setHeight(newHeight);
    m_zoomRect = newRect;
  }
  updateSceneRect(oldRect);
  updateSceneRect(m_zoomRect);
}

void PannerView::drawForeground(QPainter * p, const QRectF & rect )
{
  kDebug() << "PannerView::drawForeground " << rect;
  if (m_zoomRect.isValid()) 
  {
    p->save();
    p->setPen(Qt::red);
    if (m_zoomRect.width() > 10 && m_zoomRect.height() > 10)
    {
      p->drawRect(m_zoomRect);
    }
    else
    {
      QBrush brush(Qt::red);
      p->fillRect(m_zoomRect, brush);
    }
    p->restore();
  }
}

void PannerView::mousePressEvent(QMouseEvent* e)
{
/*  kDebug() << "PannerView::mousePressEvent " 
              << mapToScene(e->pos()) << " / " << m_zoomRect << " / " << m_zoomRect.center() <<endl;*/
  QPointF pos = mapToScene(e->pos());
  if (m_zoomRect.isValid()) 
  {
//     if (!m_zoomRect.contains(pos))
      emit zoomRectMovedTo(pos);

    m_movingZoomRect = true;
    m_lastPos = pos;
  }
}

void PannerView::mouseMoveEvent(QMouseEvent* e)
{
  QPointF pos = mapToScene(e->pos());
//   kDebug() << "PannerView::mouseMoveEvent " << pos;
  if (m_movingZoomRect) {
    emit zoomRectMovedTo(pos);
    m_lastPos = pos;
  }
}

void PannerView::mouseReleaseEvent(QMouseEvent* e)
{
  QPointF pos = mapToScene(e->pos());
//   kDebug() << "PannerView::mouseReleaseEvent " << pos;
  m_movingZoomRect = false;
  emit zoomRectMoveFinished();
}

#include "pannerview.moc"
