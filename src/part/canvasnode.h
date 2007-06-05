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
 * Canvas Node (graph node view)
 */

#ifndef CANVAS_NODE_H
#define CANVAS_NODE_H

#include <QGraphicsPolygonItem>
#include <QGraphicsEllipseItem>
#include <QWidget>
#include <QMap>
#include <QFont>
#include <QPen>
//Added by qt3to4:
#include <QPolygonF>

#include <khtml_part.h>
#include <khtmlview.h>

#include "dotgrammar.h"

class GraphNode;
class DotGraphView;

class CanvasNode
{
public:
  CanvasNode(DotGraphView* v,GraphNode* n);
  
  virtual ~CanvasNode() {}
  
  GraphNode* node() { return m_node; }

//   virtual QRect rect() = 0;
  
  static CanvasNode* dotShapedCanvasNode(const DotRenderOp& dro, 
                                  const DotRenderOpVec& dros,
                                  DotGraphView* v, 
                                  GraphNode* n,
                                  QGraphicsScene* c,
                                  double scaleX, double scaleY, 
                                         int xMargin, int yMargin, int gh,
                                         int wdhcf, int hdvcf);
    
  inline bool hasFocus() const {return m_hasFocus;}
  inline void hasFocus(bool val) {m_hasFocus = val;}
  
protected:
  virtual void paint(QPainter* p, const QStyleOptionGraphicsItem *option,
QWidget *widget);

  virtual void update() {}
  
  double m_scaleX, m_scaleY; 
  int m_xMargin, m_yMargin, m_gh, m_wdhcf, m_hdvcf;
  GraphNode* m_node;
  DotGraphView* m_view;
  bool m_hasFocus;
  DotRenderOpVec m_renderOperations;
  QFont* m_font;
  QPen m_pen;
};

class CanvasPolygonalNode: public QGraphicsPolygonItem, public CanvasNode
{
public:
  CanvasPolygonalNode(DotGraphView* v,GraphNode* n, const QPolygonF& points, QGraphicsScene* c);
  CanvasPolygonalNode(
              DotGraphView* v, 
              GraphNode* n,
              const DotRenderOp& dro,
              const DotRenderOpVec& dros,
                       QGraphicsScene* c,
                       double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                       int wdhcf, int hdvcf);
  virtual ~CanvasPolygonalNode() {}
  
//   QRect rect() {return QGraphicsPolygonItem::polygon().boundingRect().toRect();}
  virtual void update();
  
protected:
  virtual void paint(QPainter* p, const QStyleOptionGraphicsItem *option,
QWidget *widget);
};

class CanvasEllipseNode: public QGraphicsEllipseItem, public CanvasNode
{
public:
  CanvasEllipseNode(DotGraphView* v,GraphNode* n, int x, int y, int w, int h, QGraphicsScene* c);
  CanvasEllipseNode(
                    DotGraphView* v, 
                    GraphNode* n,
                    const DotRenderOp& dro,
                    const DotRenderOpVec& dros,
                    QGraphicsScene* c,
                    double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                    int wdhcf, int hdvcf);
  virtual ~CanvasEllipseNode() {}
  
//   QRect rect() {return QGraphicsEllipseItem::rect().toRect();}
  
//   QPointArray areaPoints() const;
    
protected:
  virtual void paint(QPainter* p, const QStyleOptionGraphicsItem *option,
  QWidget *widget);
};

class CanvasHtmlNode: public KHTMLPart, public CanvasNode
{
  Q_OBJECT
public:
  CanvasHtmlNode(
                     DotGraphView* v, 
                     GraphNode* n,
                     const DotRenderOp& dro,
                     const DotRenderOpVec& dros,
                     QGraphicsScene* c,
                     double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                     int wdhcf, int hdvcf);
  virtual ~CanvasHtmlNode();
  
  QRect rect() {return view()->contentsRect();}
  
protected:
//   virtual void paint(QPainter&);

public slots:
  void move(int x, int y);
  void zoomed(double factor);

private:
  double m_zoomFactor;
  int m_xMovedTo, m_yMovedTo;
};


#endif



