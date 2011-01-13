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

#include "canvaselement.h"
#include "canvaselement_p.h"

#include "dotgraphview.h"
#include "graphelement.h"
#include "support/dotdefaults.h"
#include "support/dot2qtconsts.h"
#include "support/fontscache.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <kdebug.h>
#include <klocale.h>
#include <KAction>

using namespace KGraphViz;

CanvasElementPrivate::CanvasElementPrivate() :
  m_scaleX(0), m_scaleY(0),
  m_xMargin(0), m_yMargin(0),
  m_gh(0),
  m_wdhcf(0), m_hdvcf(0),
  m_hovered(false)
{
}

CanvasElementPrivate::~CanvasElementPrivate()
{
}

CanvasElement::CanvasElement(DotGraphView* v,
                             GraphElement* gelement,
                             QGraphicsScene* c,
                             QGraphicsItem* parent)
  : QAbstractGraphicsShapeItem(parent)
  , d_ptr(new CanvasElementPrivate)
{
  Q_D(CanvasElement);
  d->m_element = gelement;
  d->m_view = v;
  d->m_pen = Dot2QtConsts::componentData().qtColor(gelement->fontColor());
  
//   kDebug();
  d->m_font = *FontsCache::changeable().fromName(gelement->fontName());
/*  kDebug() << "Creating CanvasElement for "<<gelement->id();
  kDebug() << "    data: " << wdhcf << "," << hdvcf << "," << gh << "," 
    << scaleX << "," << scaleY << "," << xMargin << "," << yMargin << endl;*/
  
  if (element()->style() == "bold")
  {
    d->m_pen.setStyle(Qt::SolidLine);
    d->m_pen.setWidth(int(2*((d->m_scaleX+d->m_scaleY)/2)));
  }
  else if (element()->style() != "filled")
  {
    d->m_pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(d->m_element->style()));
    d->m_pen.setWidth(int((d->m_scaleX+d->m_scaleY)/2));
    if (element()->style().left(12) == "setlinewidth")
    {
      bool ok;
      uint lineWidth = element()->style().mid(13, d->m_element->style().length()-1-13).toInt(&ok);
      d->m_pen.setWidth(lineWidth * int((d->m_scaleX+d->m_scaleY)/2));
    }
  }
  if (d->m_element->style() == "filled")
  {
    d->m_brush = Dot2QtConsts::componentData().qtColor(element()->backColor());
//     QCanvasPolygon::drawShape(p);
  }
  else
  {
    d->m_brush = c->backgroundBrush();
  }

  setAcceptHoverEvents ( true );
}

CanvasElement::~CanvasElement()
{
  kDebug() << element()->label();
  
  delete d_ptr;
}

GraphElement* CanvasElement::element() const
{
  Q_D(const CanvasElement);
  return d->m_element;
}

qreal CanvasElement::gh() const
{
  Q_D(const CanvasElement);
  return d->m_gh;
}
void CanvasElement::setGh(qreal gh)
{
  Q_D(CanvasElement);
  d->m_gh = gh;
}

qreal CanvasElement::marginX() const
{
  Q_D(const CanvasElement);
  return d->m_xMargin;
}
void CanvasElement::setMarginX(qreal marginX)
{
  Q_D(CanvasElement);
  d->m_xMargin = marginX;
}

qreal CanvasElement::marginY() const
{
  Q_D(const CanvasElement);
  return d->m_yMargin;
}
void CanvasElement::setMarginY(qreal marginY)
{
  Q_D(CanvasElement);
  d->m_yMargin = marginY;
}

qreal CanvasElement::scaleX() const
{
  Q_D(const CanvasElement);
  return d->m_scaleX;
}
void CanvasElement::setScaleX(qreal scaleX)
{
  Q_D(CanvasElement);
  d->m_scaleX = scaleX;
}

qreal CanvasElement::scaleY() const
{
  Q_D(const CanvasElement);
  return d->m_scaleY;
}
void CanvasElement::setScaleY(qreal scaleY)
{
  Q_D(CanvasElement);
  d->m_scaleY = scaleY;
}

QFont CanvasElement::font() const
{
  Q_D(const CanvasElement);
  return d->m_font;
}
void CanvasElement::setFont(const QFont& font)
{
  Q_D(CanvasElement);
  d->m_font = font;
}

void CanvasElement::setBoundingRect(const QRectF& rect)
{
  Q_D(CanvasElement);
  if (d->m_boundingRect == rect)
    return;

  prepareGeometryChange();
  d->m_boundingRect = rect;
}

void CanvasElement::modelChanged()
{
  Q_D(CanvasElement);
  kDebug() ;//<< id();
  d->m_pen = QPen(Dot2QtConsts::componentData().qtColor(d->m_element->fontColor()));
  d->m_font = *FontsCache::changeable().fromName(d->m_element->fontName());
  computeBoundingRect();
}

void CanvasElement::initialize(qreal scaleX, qreal scaleY,
                            qreal xMargin, qreal yMargin, qreal gh,
                            qreal wdhcf, qreal hdvcf)
{
  Q_D(CanvasElement);
  kDebug();
  setFlag(QGraphicsItem::ItemIsSelectable, true);

  d->m_scaleX = scaleX; d->m_scaleY = scaleY;
  d->m_xMargin = xMargin; d->m_yMargin = yMargin;
//   d->m_gh = gh;
  d->m_wdhcf = wdhcf; d->m_hdvcf = hdvcf;

  setZValue(d->m_element->z());

  computeBoundingRect();
}

QRectF CanvasElement::boundingRect() const
{
  Q_D(const CanvasElement);
  return d->m_boundingRect;
}

void CanvasElement::computeBoundingRect()
{
  Q_D(CanvasElement);
  kDebug() << element()->id() << zValue();
  
  qreal adjust = 0.5;
  QRectF rect;
  if (element()->renderOperations().isEmpty())
  {
    kDebug() << "no render operation";
    rect = QRectF(0,0,(d->m_view->defaultNewElementPixmap().size().width())*d->m_scaleX,(d->m_view->defaultNewElementPixmap().size().height())*d->m_scaleY);
    d->m_boundingRect = rect;
  }
  else
  {
    DotRenderOpVec::const_iterator it, it_end;
    it = element()->renderOperations().constBegin(); it_end = element()->renderOperations().constEnd();
    for (; it != it_end; it++)
    {
//       QString msg;
//       QTextStream dd(&msg);
//       dd << element()->id() << " an op: " << (*it).renderop << " ";
//       foreach (int i, (*it).integers)
//       {
//         dd << i << " ";
//       }
//       dd << (*it).str;
//       kDebug() << msg;

      if ((*it).renderop == "e" || (*it).renderop == "E")
      {
//         kDebug() << "integers[0]=" << (*it).integers[0] << "; d->m_wdhcf=" << d->m_wdhcf
//             << "(*it).integers[0]/*%d->m_wdhcf*/=" << (*it).integers[0]/*%d->m_wdhcf*/;
        qreal w = d->m_scaleX * (*it).integers[2] * 2;
        qreal h = d->m_scaleY * (*it).integers[3] * 2;
        qreal x = d->m_xMargin + (((*it).integers[0]/*%d->m_wdhcf*/)*d->m_scaleX) - w/2;
        qreal y = ((d->m_gh - (*it).integers[1]/*%d->m_hdvcf*/)*d->m_scaleY) + d->m_yMargin - h/2;
        d->m_boundingRect = QRectF(x - adjust,y - adjust, w + adjust, h + adjust);
//         kDebug() << "'" << element()->id() << "' set rect for ellipse to " << rect;
      }
      else if  ((*it).renderop == "p" || (*it).renderop == "P")
      {
        QPolygonF polygon((*it).integers[0]);
        for (int i = 0; i < (*it).integers[0]; i++)
        {
          qreal x,y;
          x = ((*it).integers[2*i+1] == d->m_wdhcf)?(*it).integers[2*i+1]:(*it).integers[2*i+1]/*%d->m_wdhcf*/;
          y = ((*it).integers[2*i+2] == d->m_hdvcf)?(*it).integers[2*i+2]:(*it).integers[2*i+2]/*%d->m_hdvcf*/;
          {

          }
          QPointF p(
                    x*d->m_scaleX +d->m_xMargin,
                    (d->m_gh-y)*d->m_scaleY + d->m_yMargin
                  );
          polygon[i] = p;
        }
        d->m_boundingRect = polygon.boundingRect();
//         kDebug() << "'" << element()->id() << "' set rect for polygon to " << rect;
      }
    }
  }
  setPos(0,0);
}

void CanvasElement::paint(QPainter* p,
                          const QStyleOptionGraphicsItem* option,
                          QWidget* widget)
{
  Q_UNUSED(option)
  Q_UNUSED(widget)

  Q_D(CanvasElement);

  /// computes the scaling of line width
  qreal widthScaleFactor = (d->m_scaleX+d->m_scaleY)/2;
  if (widthScaleFactor < 1)
  {
    widthScaleFactor = 1;
  }
  
  QString msg;
  QTextStream dd(&msg);
  foreach (const DotRenderOp &op, element()->renderOperations())
  {
    dd << element()->id() << " an op: " << op.renderop << " ";
    foreach (int i, op.integers)
    {
      dd << i << " ";
    }
    dd << op.str << endl;
  }
//   kDebug() << msg;

  if (element()->renderOperations().isEmpty() && !d->m_view->isReadOnly())
  {
    kError() << element()->id() << ": no render operation. This should not happen.";
    return;
  }

  QListIterator<DotRenderOp> it(element()->renderOperations());

  QColor lineColor(Dot2QtConsts::componentData().qtColor(element()->lineColor()));
  QColor backColor(Dot2QtConsts::componentData().qtColor(element()->backColor()));
  if (d->m_hovered && d->m_view->highlighting())
  {
    backColor = backColor.lighter(110);
  }
  
  while (it.hasNext())
  {
    const DotRenderOp& dro = it.next();
    if (dro.renderop == "c")
    {
      QColor c(dro.str.mid(0,7));
      bool ok;
      c.setAlpha(255-dro.str.mid(8).toInt(&ok,16));
      lineColor = c;
//       kDebug() << "c" << dro.str.mid(0,7) << lineColor;
    }
    else if (dro.renderop == "C")
    {
      QColor c(dro.str.mid(0,7));
      bool ok;
      c.setAlpha(255-dro.str.mid(8).toInt(&ok,16));
      if (d->m_hovered && d->m_view->highlighting())
      {
        c = c.lighter();
      }
      backColor = c;
//       kDebug() << "C" << dro.str.mid(0,7) << backColor;
    }
    else if (dro.renderop == "e" || dro.renderop == "E")
    {
      QPen pen = p->pen();
      qreal w = d->m_scaleX * dro.integers[2] * 2;
      qreal h = d->m_scaleY * dro.integers[3] * 2;
      qreal x = d->m_xMargin + ((dro.integers[0]/*%d->m_wdhcf*/)*d->m_scaleX) - w/2;
      qreal y = ((d->m_gh - dro.integers[1]/*%d->m_hdvcf*/)*d->m_scaleY) + d->m_yMargin - h/2;
      QRectF rect(x,y,w,h);
      p->save();
      p->setBrush(backColor);
      pen.setColor(lineColor);
      if (element()->attributes().contains("penwidth"))
      {
        bool ok;
        int lineWidth = element()->attributes()["penwidth"].toInt(&ok);
        pen.setWidth(int(lineWidth * widthScaleFactor));
      }
      p->setPen(pen);
      
//       kDebug() << element()->id() << "drawEllipse" << lineColor << backColor << rect;
//       rect = QRectF(0,0,100,100);
      p->drawEllipse(rect);
      p->restore();
    }
    else if(dro.renderop == "p" || dro.renderop == "P")
    {
//       std::cerr << "Drawing polygon for node '"<<element()->id()<<"': ";
      QPolygonF points(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        qreal x,y;
        x = (dro.integers[2*i+1] == d->m_wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]/*%d->m_wdhcf*/;
        y = (dro.integers[2*i+2] == d->m_hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]/*%d->m_hdvcf*/;
        QPointF p(
                  (x*d->m_scaleX) + d->m_xMargin,
                  ((d->m_gh-y)*d->m_scaleY) + d->m_yMargin
                );
/*        kDebug() << "    point: (" << dro.integers[2*i+1] << ","
                  << dro.integers[2*i+2] << ") " << d->m_wdhcf << "/" << d->m_hdvcf;*/
        points[i] = p;
      }
      p->save();

      QPen pen = p->pen();
      pen.setColor(lineColor);
      if (element()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      if (element()->attributes().contains("penwidth"))
      {
        bool ok;
        int lineWidth = element()->attributes()["penwidth"].toInt(&ok);
        pen.setWidth(int(lineWidth * widthScaleFactor));
      }
      else if (element()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(element()->style()));
      }
      if (element()->style().left(12) == "setlinewidth")
      {
        bool ok;
        uint lineWidth = element()->style().mid(12, element()->style().length()-1-12).toInt(&ok);
        pen.setWidth(lineWidth);
      }
      p->setPen(pen);
      p->setBrush(backColor);
/*      if (element()->style() == "filled")
      {
        p->setBrush(Dot2QtConsts::componentData().qtColor(element()->backColor()));
    //     QCanvasPolygon::paint(p);
      }
      else
      {
        p->setBrush(canvas()->backgroundColor());
      }*/
//       kDebug() << element()->id() << "drawPolygon" << points;
      p->drawPolygon(points);
      p->restore();
      if (!element()->shapeFile().isEmpty())
      {
        QPixmap pix(element()->shapeFile());
        if (!pix.isNull())
        {
          p->drawPixmap(int(points.boundingRect().left()), int(points.boundingRect().top()), pix);
        }
      }
    }

  }

  it.toFront();
  while (it.hasNext())
  {
    const DotRenderOp& dro = it.next();
    if (dro.renderop == "c")
    {
      QColor c(dro.str.mid(0,7));
      bool ok;
      c.setAlpha(255-dro.str.mid(8).toInt(&ok,16));
      lineColor = c;
//       kDebug() << "c" << dro.str.mid(0,7) << lineColor;
    }
    else if (dro.renderop == "C")
    {
      QColor c(dro.str.mid(0,7));
      bool ok;
      c.setAlpha(255-dro.str.mid(8).toInt(&ok,16));
      if (d->m_hovered && d->m_view->highlighting())
      {
        c = c.lighter();
      }
      backColor = c;
//       kDebug() << "C" << dro.str.mid(0,7) << backColor;
    }
    else if ( dro.renderop == "L" )
    {
//       kDebug() << "Label";
      QPolygonF points(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        qreal x,y;
        x = (dro.integers[2*i+1] == d->m_wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]/*%d->m_wdhcf*/;
        y = (dro.integers[2*i+2] == d->m_hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]/*%d->m_hdvcf*/;
        QPointF p(
                  (x*d->m_scaleX) +d->m_xMargin,
                  ((d->m_gh-y)*d->m_scaleY) + d->m_yMargin
                );
        points[i] = p;
      }
      p->save();
      QPen pen(lineColor);
      if (element()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      else if (element()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(element()->style()));
      }
      p->setPen(pen);
//       kDebug() << element()->id() << "drawPolyline" << points;
      p->drawPolyline(points);
      p->restore();
    }
  }

//   kDebug() << "Drawing" << element()->id() << "labels";
  QString color = lineColor.name();
  it.toFront();
  while (it.hasNext())
  {
    const DotRenderOp& dro = it.next();
    if (dro.renderop == "c" || dro.renderop == "C")
    {
      color = dro.str.mid(0,7);
//       kDebug() << dro.renderop << color;
    }
    else if (dro.renderop == "F")
    {
      element()->setFontName(dro.str);
      element()->setFontSize(dro.integers[0]);
//       kDebug() << "F" << element()->fontName() << element()->fontColor() << element()->fontSize();
    }
    else if ( dro.renderop == "T" )
    {
      // we suppose here that the color has been set just before
      element()->setFontColor(color);
      // draw a label
//       kDebug() << "Drawing a label " << dro.integers[0]
//       << " " << dro.integers[1] << " " << dro.integers[2]
//       << " " << dro.integers[3] << " " << dro.str
//         << " (" << element()->fontName() << ", " << element()->fontSize()
//         << ", " << element()->fontColor() << ")";

      int stringWidthGoal = int(dro.integers[3] * d->m_scaleX);
      int fontSize = element()->fontSize();
//       kDebug() << element()->id() << " initial fontSize " << fontSize;
      d->m_font.setPointSize(fontSize);
      QFontMetrics fm(d->m_font);
      while (fm.width(dro.str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        d->m_font.setPointSize(fontSize);
        fm = QFontMetrics(d->m_font);
      }
      p->save();
      p->setFont(d->m_font);
      QPen pen(d->m_pen);
      pen.setColor(element()->fontColor());
      p->setPen(pen);
      qreal x = (d->m_scaleX *
                       (
                         (dro.integers[0])
                         + (((-dro.integers[2])*(fm.width(dro.str)))/2)
                         - ( (fm.width(dro.str))/2 )
                       )
                      )
                      + d->m_xMargin;
      qreal y = ((d->m_gh - (dro.integers[1]))*d->m_scaleY)+ d->m_yMargin;
      QPointF point(x,y);
//       kDebug() << element()->id() << "drawText" << point << " " << fontSize;
      p->drawText(point, dro.str);
      p->restore();
    }
  }
  if (isSelected())
  {
//     kDebug() << "element is selected: draw selection marks";
    p->save();
    p->setBrush(Qt::black);
    p->setPen(Qt::black);
    p->drawRect(QRectF(d->m_boundingRect.topLeft(),QSizeF(6,6)));
    p->drawRect(QRectF(d->m_boundingRect.topRight()-QPointF(6,0),QSizeF(6,6)));
    p->drawRect(QRectF(d->m_boundingRect.bottomLeft()-QPointF(0,6),QSizeF(6,6)));
    p->drawRect(QRectF(d->m_boundingRect.bottomRight()-QPointF(6,6),QSizeF(6,6)));
    p->restore();
  }
}

void CanvasElement::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  Q_D(CanvasElement);
  kDebug() << d->m_element->id() << boundingRect();

  if (event->button() != Qt::LeftButton)
    return;

  if (d->m_view->editingMode() == DotGraphView::AddNewEdge)
  {
    d->m_view->createNewEdgeDraftFrom(this);
  }
  else if (d->m_view->editingMode() == DotGraphView::DrawNewEdge)
  {
    d->m_view->finishNewEdgeTo(this);
  }
}

void CanvasElement::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
  Q_UNUSED(event)
  Q_D(CanvasElement);
  kDebug() << "Element:" << element()->id();
  d->m_hovered = true;
  update();
}

void CanvasElement::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
  Q_UNUSED(event)
  Q_D(CanvasElement);
  kDebug() << "Element:" << element()->id();
  d->m_hovered = false;
  update();
}
