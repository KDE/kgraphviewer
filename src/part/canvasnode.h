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
 * Canvas Node (graph node view)
 */

#ifndef CANVAS_NODE_H
#define CANVAS_NODE_H

#include <qcanvas.h>
#include <qwidget.h>
#include <qmap.h>
#include <qfont.h>
#include <qpen.h>

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

  virtual QRect rect() = 0;
  
  static CanvasNode* dotShapedCanvasNode(const DotRenderOp& dro, 
                                  const DotRenderOpVec& dros,
                                  DotGraphView* v, 
                                  GraphNode* n,
                                  QCanvas* c,
                                  double scaleX, double scaleY, 
                                         int xMargin, int yMargin, int gh,
                                         int wdhcf, int hdvcf);
    
  inline bool hasFocus() const {return m_hasFocus;}
  inline void hasFocus(bool val) {m_hasFocus = val;}
  
protected:
  virtual void drawShape(QPainter&);
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

class CanvasPolygonalNode: public QCanvasPolygon, public CanvasNode
{
public:
  CanvasPolygonalNode(DotGraphView* v,GraphNode* n, const QPointArray& points, QCanvas* c);
  CanvasPolygonalNode(
              DotGraphView* v, 
              GraphNode* n,
              const DotRenderOp& dro,
              const DotRenderOpVec& dros,
                       QCanvas* c,
                       double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                       int wdhcf, int hdvcf);
  virtual ~CanvasPolygonalNode() {}
  
  QRect rect() {return QCanvasPolygon::boundingRect();}
//   QPointArray areaPoints() const;
  virtual void update();
  
protected:
  virtual void drawShape(QPainter&);
};

class CanvasEllipseNode: public QCanvasEllipse, public CanvasNode
{
public:
  CanvasEllipseNode(DotGraphView* v,GraphNode* n, int x, int y, int w, int h, QCanvas* c);
  CanvasEllipseNode(
                    DotGraphView* v, 
                    GraphNode* n,
                    const DotRenderOp& dro,
                    const DotRenderOpVec& dros,
                    QCanvas* c,
                    double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                    int wdhcf, int hdvcf);
  virtual ~CanvasEllipseNode() {}
  
  QRect rect() {return QCanvasEllipse::boundingRect();}
  
//   QPointArray areaPoints() const;
    
protected:
  virtual void drawShape(QPainter&);
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
                     QCanvas* c,
                     double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                     int wdhcf, int hdvcf);
  virtual ~CanvasHtmlNode();
  
  QRect rect() {return view()->contentsRect();}
  
protected:
//   virtual void drawShape(QPainter&);

public slots:
  void move(int x, int y);
  void zoomed(double factor);

private:
  double m_zoomFactor;
  int m_xMovedTo, m_yMovedTo;
};


#endif



