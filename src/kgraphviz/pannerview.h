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

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/

#ifndef KGRAPHVIZ_PANNER_VIEW_H
#define KGRAPHVIZ_PANNER_VIEW_H

#include <QGraphicsView>

namespace KGraphViz
{

class DotGraphView;
class PannerViewPrivate;

/**
 * A panner layed over a QCanvas
 */
class PannerView: public QGraphicsView
{
  Q_OBJECT

public:
  explicit PannerView(DotGraphView * parent);
  virtual ~PannerView();

  void updateBackground();

public Q_SLOTS:
  void setZoomRect(const QRectF& rectangle);
  void moveZoomRectTo(const QPointF& newPos, const bool notify = true);

Q_SIGNALS:
  void zoomRectMovedTo(QPointF newPos);
  void zoomRectMoveFinished();

protected:
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void drawForeground(QPainter * p, const QRectF & rect );
  virtual void drawBackground(QPainter* painter, const QRectF& rect);
  virtual void contextMenuEvent(QContextMenuEvent* event);

  PannerViewPrivate* const d_ptr;

private:
  Q_DECLARE_PRIVATE(PannerView)
};

}

#endif
