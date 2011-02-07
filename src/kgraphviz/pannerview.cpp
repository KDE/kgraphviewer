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

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#include "pannerview.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QGraphicsScene>
#include <QPainter>
#include <QMouseEvent>

#include <kdebug.h>
#include <klocale.h>

#include "dotgraphview.h"

using namespace KGraphViz;

class KGraphViz::PannerViewPrivate
{
public:
  PannerViewPrivate(DotGraphView* parent);
  ~PannerViewPrivate();

  inline DotGraphView* parent() const { return q_ptr; }

  Q_DECLARE_PUBLIC(DotGraphView)
  DotGraphView* const q_ptr;

  QPixmap m_backgroundPixmap;
  QRectF m_zoomRect;
  bool m_movingZoomRect;
  QPointF m_lastPos;
};

PannerViewPrivate::PannerViewPrivate(DotGraphView* parent)
  : q_ptr(parent)
  , m_movingZoomRect(false)
{
}

PannerViewPrivate::~PannerViewPrivate()
{
  kDebug();
}

PannerView::PannerView(DotGraphView * parent)
  : QGraphicsView(parent)
  , d_ptr(new PannerViewPrivate(parent))
{
  setScene(new QGraphicsScene);
  setCacheMode(QGraphicsView::CacheBackground);
  setRenderHints(QPainter::Antialiasing);

  // if there are ever graphic glitches to be found, remove this again
  setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing |
                       QGraphicsView::DontClipPainter |
                       QGraphicsView::DontSavePainterState);

  setToolTip(i18n("View of the complete graph. Click and drag to move the visible part."));
  setWhatsThis(i18n("<h1>View of the Complete Graph</h1>"
    "<p>Single clicking somewhere without the red square will move the center of the "
    "view to where the mouse was clicked.</p><p>Clicking and dragging within the red square "
    "will cause the view to follow the movement.</p>"));
}

PannerView::~PannerView()
{
  kDebug();
  
  delete d_ptr;
}

void PannerView::setZoomRect(const QRectF& rectangle)
{
  Q_D(PannerView);
//   kDebug() << "PannerView::setZoomRect " << r;
  if (rectangle == d->m_zoomRect) {
    return;
  }
  // get new zoom rect
  d->m_zoomRect = rectangle;
  const qreal q1 = mapToScene(15,0).x();
  const qreal d1 = mapToScene(10,0).x();
  if (!d->m_zoomRect.isValid() || d->m_zoomRect.width() < q1 || d->m_zoomRect.height() < q1)
  {
    double factor = ((double)d->m_zoomRect.width())/d->m_zoomRect.height();
    qreal newWidth, newHeight;
    if (factor > 1.0)
    {
      newWidth = q1;
      newHeight = newWidth/factor;
      if (newHeight < d1) newHeight = d1;
    }
    else
    {
      newHeight = q1;
      newWidth = newHeight*factor;
      if (newWidth < d1) newWidth = d1;
    }
    QRectF newRect = d->m_zoomRect;
    qreal newXPos = newRect.x() + (newRect.width()/2) - newWidth/2;
    newXPos = (newXPos<0)?0:newXPos;
    newRect.setX(newXPos);
    qreal newYPos = newRect.y() + (newRect.height()/2) -newHeight/2;
    newYPos = (newYPos<0)?0:newYPos;
    newRect.setY(newYPos);
    newRect.setWidth(newWidth);
    newRect.setHeight(newHeight);
    d->m_zoomRect = newRect;
  }
}

void PannerView::moveZoomRectTo(const QPointF& newPos, bool notify)
{
  Q_D(PannerView);
//   kDebug() << newPos << d->m_zoomRect;
  if (!d->m_zoomRect.isValid()) {
    return;
  }

  if (d->m_zoomRect.center() == newPos) {
    kDebug() << "Same position, don't do anything";
    return;
  }

  scene()->invalidate(d->m_zoomRect, QGraphicsScene::ForegroundLayer);
  d->m_zoomRect.moveCenter(newPos);
  scene()->invalidate(d->m_zoomRect, QGraphicsScene::ForegroundLayer);

  if (d->m_zoomRect.isValid() && notify) {
    d->m_lastPos = newPos;
	emit zoomRectMovedTo(newPos);
  }
}

void PannerView::drawForeground(QPainter * p, const QRectF & rect )
{
  kDebug();
  Q_D(const PannerView);
  if (d->m_zoomRect.isValid() && rect.intersects(d->m_zoomRect))
  {
    p->save();
    if (d->m_zoomRect.width() > 10 && d->m_zoomRect.height() > 10)
    {
      p->setPen(Qt::red);
      p->setOpacity(0.2);
      p->setBrush(Qt::red);

      // substract pen width, i.e. draw inside
      qreal penWidth = p->pen().widthF();
      p->drawRect(d->m_zoomRect.adjusted(-penWidth, -penWidth, -penWidth, -penWidth));
    }
    else
    {
      QBrush brush(Qt::red);
      p->fillRect(d->m_zoomRect, brush);
    }
    p->restore();
  }
}


void PannerView::drawBackground(QPainter* painter, const QRectF& rect)
{
  Q_D(const PannerView);
//   kDebug() << rect << viewport()->rect() << sceneRect() << d->m_backgroundPixmap.rect();
  
  painter->eraseRect(viewport()->rect());
  painter->drawPixmap(rect, d->m_backgroundPixmap, d->m_backgroundPixmap.rect());
}

void PannerView::updateBackground()
{
  Q_D(PannerView);
  const DotGraphView* view = d->parent();
  QPixmap pixmap(viewport()->rect().size());
  QPainter p(&pixmap);
  view->scene()->render(&p, pixmap.rect(), view->sceneRect(), Qt::IgnoreAspectRatio);
  setSceneRect(view->sceneRect());

  d->m_backgroundPixmap = pixmap;
  invalidateScene(QRectF(), QGraphicsScene::BackgroundLayer);
}

void PannerView::mousePressEvent(QMouseEvent* e)
{
  Q_D(PannerView);
  if (e->button() != Qt::LeftButton) {
    return;
  }
/*  kDebug() << "PannerView::mousePressEvent " 
              << mapToScene(e->pos()) << " / " << d->m_zoomRect << " / " << d->m_zoomRect.center() <<endl;*/
  moveZoomRectTo(mapToScene(e->pos()));
  d->m_movingZoomRect = true;
}

void PannerView::mouseMoveEvent(QMouseEvent* e)
{
  Q_D(const PannerView);
  if (!d->m_movingZoomRect) {
    return;
  }

//   kDebug() << "PannerView::mouseMoveEvent " << pos;
  moveZoomRectTo(mapToScene(e->pos()));
}

void PannerView::mouseReleaseEvent(QMouseEvent* e)
{
  Q_D(PannerView);
  if (e->button() != Qt::LeftButton) {
    return;
  }
  moveZoomRectTo(mapToScene(e->pos()));
//   kDebug() << "PannerView::mouseReleaseEvent " << pos;
  d->m_movingZoomRect = false;
  emit zoomRectMoveFinished();
}

void PannerView::contextMenuEvent(QContextMenuEvent* event)
{
  Q_D(PannerView);
  d->parent()->contextMenuEvent(event);
}

#include "pannerview.moc"
