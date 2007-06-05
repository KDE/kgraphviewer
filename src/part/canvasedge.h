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

#ifndef CANVAS_EDGE_H
#define CANVAS_EDGE_H

#include <qcanvas.h>
#include <qwidget.h>
#include <qmap.h>
#include <qfont.h>

#include "graphexporter.h"


class KTempFile;
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


class CanvasEdge : public QCanvasPolygonalItem
{
public:
  CanvasEdge(GraphEdge*, QCanvas*,
             double scaleX, double scaleY, 
             int xMargin, int yMargin, int gh,
             int wdhcf, int hdvcf);

  void drawShape(QPainter&);

  GraphEdge* edge() { return _edge; }
  
  QPointArray areaPoints() const;
    

  private:
  double m_scaleX, m_scaleY; 
  int m_xMargin, m_yMargin, m_gh, m_wdhcf, m_hdvcf;
  GraphEdge* _edge;
  QPointArray m_points;
  QFont* m_font;
};



#endif



