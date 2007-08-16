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

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#ifndef CANVAS_EDGE_H
#define CANVAS_EDGE_H

#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QAbstractGraphicsShapeItem>
#include <QWidget>
#include <QMap>
#include <QFont>

#include "graphexporter.h"


class CanvasNode;
class CanvasEdge;
class GraphEdge;
class DotGraphView;

/*
 * Canvas Items:
 * - CanvasNode       (Rectangular Area)
 * - CanvasEdge       (Spline curve)
 * - CanvasEdgeLabel  (Label for edges)
 * - CanvasEdgeArrow  (Arrows at the end of the edge spline)
 */


class CanvasEdge : public QObject, public QAbstractGraphicsShapeItem
{
Q_OBJECT
public:
  CanvasEdge(DotGraphView* v, GraphEdge*,
             double scaleX, double scaleY, 
             int xMargin, int yMargin, int gh,
             int wdhcf, int hdvcf, QGraphicsItem* parent = 0);

  QRectF boundingRect() const;

  void paint(QPainter* p, const QStyleOptionGraphicsItem *option,
        QWidget *widget);

  GraphEdge* edge() { return m_edge; }
  const GraphEdge* edge() const { return m_edge; }
  
public Q_SLOTS:
  void modelChanged();

protected:
  void computeBoundingRect();

  private:
  double m_scaleX, m_scaleY; 
  int m_xMargin, m_yMargin, m_gh, m_wdhcf, m_hdvcf;
  GraphEdge* m_edge;
  QRectF m_boundingRect;
  QFont* m_font;
  DotGraphView* m_view;
};



#endif



