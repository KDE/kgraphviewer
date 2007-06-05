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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#ifndef PANNER_VIEW_H
#define PANNER_VIEW_H

#include <qcanvas.h>
#include <qwidget.h>


/**
 * A panner laid over a QCanvas
 */
class PannerView: public QCanvasView
{
  Q_OBJECT

public:
  PannerView(QWidget * parent = 0, const char * name = 0);

  void setZoomRect(QRect r);

  inline void setDrawingEnabled(bool val) {m_drawContents = val;}

signals:
  void zoomRectMoved(int dx, int dy);
  void zoomRectMoveFinished();

protected:
  void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent(QMouseEvent*);
  void contentsMouseReleaseEvent(QMouseEvent*);
  void drawContents(QPainter * p, int clipx, int clipy, int clipw, int cliph);

  QRect _zoomRect;
  bool _movingZoomRect;
  QPoint _lastPos;
  bool m_drawContents;
};

#endif



