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

/*
 * Canvas Subgraph (subgraph node view)
 */

#ifndef CANVAS_SUBGRAPH_H
#define CANVAS_SUBGRAPH_H

#include <QGraphicsPolygonItem>
#include <qfont.h>
#include <qpen.h>
#include <qbrush.h>

#include "dotgrammar.h"

class GraphSubgraph;
class DotGraphView;
class QGraphicsScene;

class CanvasSubgraph: public QGraphicsPolygonItem
{
public:
  CanvasSubgraph(
      DotGraphView* v, 
      GraphSubgraph* s,
      QGraphicsScene* c,
      double scaleX, double scaleY, int xMargin, int yMargin, int gh,
      int wdhcf, int hdvcf);
  virtual ~CanvasSubgraph() {}
  
  GraphSubgraph* subgraph() { return m_subgraph; }

  QRectF rect() {return QGraphicsPolygonItem::boundingRect();}
  
    
protected:
  virtual void paint(QPainter&);
  
  double m_scaleX, m_scaleY; 
  int m_xMargin, m_yMargin, m_gh, m_wdhcf, m_hdvcf;
  GraphSubgraph* m_subgraph;
  DotGraphView* m_view;
  QFont* m_font;
  QPen m_pen;
  QBrush m_brush;
};

  
#endif



