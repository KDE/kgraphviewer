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

#ifndef KGRAPHVIZ_CANVAS_ELEMENT_H
#define KGRAPHVIZ_CANVAS_ELEMENT_H

#include <QAbstractGraphicsShapeItem>

class QGraphicsScene;
class QFont;

namespace KGraphViz
{

class CanvasElementPrivate;
class GraphElement;
class DotGraphView;

class CanvasElement : public QAbstractGraphicsShapeItem
{
public:
  CanvasElement(
      DotGraphView* v, 
      GraphElement* s,
      QGraphicsScene* c,
      QGraphicsItem* parent = 0);

  virtual ~CanvasElement();

  GraphElement* element() const;

  virtual void paint(QPainter* p, const QStyleOptionGraphicsItem *option,
        QWidget *widget = 0 );

  virtual QRectF boundingRect() const;

  void initialize(qreal scaleX, qreal scaleY,
                  qreal marginX, qreal marginY,
                  qreal gh,
                  qreal wdhcf, qreal hdvcf);

  virtual void modelChanged();

protected:
  virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
  virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
  virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );

  virtual void computeBoundingRect();
  void setBoundingRect(const QRectF& rect);

  qreal gh() const;
  void setGh(qreal gh);
  QFont font() const;
  void setFont(const QFont& font);
  qreal scaleX() const;
  void setScaleX(qreal scaleX);
  qreal scaleY() const;
  void setScaleY(qreal scaleY);
  qreal marginX() const;
  void setMarginX(qreal marginX);
  qreal marginY() const;
  void setMarginY(qreal marginY);

  CanvasElementPrivate* const d_ptr;

private:
  Q_DECLARE_PRIVATE(CanvasElement)
};

}

#endif // KGRAPHVIZ_CANVAS_ELEMENT_H
