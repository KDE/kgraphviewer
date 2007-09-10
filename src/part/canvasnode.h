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

/*
 * Canvas Node (graph node view)
 */

#ifndef CANVAS_NODE_H
#define CANVAS_NODE_H

#include <QAbstractGraphicsShapeItem>
#include <QPen>

#include <khtml_part.h>
#include <khtmlview.h>

#include "dotgrammar.h"

class GraphNode;
class DotGraphView;

class CanvasNode : public QObject, public QAbstractGraphicsShapeItem
{
  Q_OBJECT
public:
  CanvasNode(DotGraphView* v,GraphNode* n, QGraphicsItem* parent = 0);
  
  virtual ~CanvasNode() {}
  
  void initialize(qreal scaleX, qreal scaleY,
                  qreal xMargin, qreal yMargin, qreal gh,
                  qreal wdhcf, qreal hdvcf);

  GraphNode* node() { return m_node; }
  const GraphNode* node() const { return m_node; }

  inline bool hasFocus() const {return m_hasFocus;}
  inline void hasFocus(bool val) {m_hasFocus = val;}

  virtual void paint(QPainter* p, const QStyleOptionGraphicsItem *option,
        QWidget *widget = 0 );

  virtual QRectF boundingRect () const;
  void computeBoundingRect();

public Q_SLOTS:
  void modelChanged();
  
protected:
  virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
  virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
  virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

  qreal m_scaleX, m_scaleY;
  qreal m_xMargin, m_yMargin, m_gh, m_wdhcf, m_hdvcf;
  GraphNode* m_node;
  DotGraphView* m_view;
  bool m_hasFocus;
  QFont* m_font;
  QPen m_pen;
  QRectF m_boundingRect;
};

// class CanvasHtmlNode: public KHTMLPart, public CanvasNode
// {
//   Q_OBJECT
// public:
//   CanvasHtmlNode(
//                      DotGraphView* v, 
//                      GraphNode* n,
//                      const DotRenderOp& dro,
//                      const DotRenderOpVec& dros,
//                      QGraphicsScene* c,
//                      double scaleX, double scaleY, int xMargin, int yMargin, int gh,
//                      int wdhcf, int hdvcf);
//   virtual ~CanvasHtmlNode();
//   
//   QRect rect() {return view()->contentsRect();}
//   
// protected:
// //   virtual void paint(QPainter&);
// 
// public slots:
//   void move(int x, int y);
//   void zoomed(double factor);
// 
// private:
//   double m_zoomFactor;
//   int m_xMovedTo, m_yMovedTo;
// };


#endif



