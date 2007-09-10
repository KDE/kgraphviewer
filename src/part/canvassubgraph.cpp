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


#include "canvassubgraph.h"
#include "dotgraphview.h"
#include "graphsubgraph.h"
#include "dotdefaults.h"
#include "dot2qtconsts.h"
#include "FontsCache.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QPainter>
#include <QGraphicsScene>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
//#include <ktempfile.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>

//
// CanvasNode
//



CanvasSubgraph::CanvasSubgraph(
                              DotGraphView* v,
                              GraphSubgraph* gsubgraph,
                              QGraphicsScene* c,
                              QGraphicsItem* parent
                            )
  : QObject(), QAbstractGraphicsShapeItem(parent), 
    m_subgraph(gsubgraph), m_view(v),
    m_font(0),
    m_pen(Dot2QtConsts::componentData().qtColor(gsubgraph->fontColor()))
{
  kDebug();
  m_font = FontsCache::changeable().fromName(gsubgraph->fontName());
/*  kDebug() << "Creating CanvasSubgraph for "<<gsubgraph->id();
  kDebug() << "    data: " << wdhcf << "," << hdvcf << "," << gh << "," 
    << scaleX << "," << scaleY << "," << xMargin << "," << yMargin << endl;*/
  
  if (subgraph()->style() == "bold")
  {
    m_pen.setStyle(Qt::SolidLine);
    m_pen.setWidth(int(2*((m_scaleX+m_scaleY)/2)));
  }
  else if (subgraph()->style() != "filled")
  {
    m_pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(m_subgraph->style()));
    m_pen.setWidth(int((m_scaleX+m_scaleY)/2));
    if (subgraph()->style().left(12) == "setlinewidth")
    {
      bool ok;
      uint lineWidth = subgraph()->style().mid(13, subgraph()->style().length()-1-13).toInt(&ok);
      m_pen.setWidth(lineWidth * int((m_scaleX+m_scaleY)/2));
    }
  }
  if (m_subgraph->style() == "filled")
  {
    m_brush = Dot2QtConsts::componentData().qtColor(subgraph()->backColor());
//     QCanvasPolygon::drawShape(p);
  }
  else
  {
    m_brush = c->backgroundBrush();
  }
  
}

void CanvasSubgraph::initialize(qreal scaleX, qreal scaleY,
                            qreal xMargin, qreal yMargin, qreal gh,
                            qreal wdhcf, qreal hdvcf)
{
  kDebug();
  setFlag(QGraphicsItem::ItemIsMovable, true);

  m_scaleX = scaleX; m_scaleY = scaleY;
  m_xMargin = xMargin; m_yMargin = yMargin;
  m_gh = gh; m_wdhcf = wdhcf; m_hdvcf = hdvcf;

  setZValue(m_subgraph->z());

  computeBoundingRect();
}

QRectF CanvasSubgraph::boundingRect () const
{
  return m_boundingRect;
}

void CanvasSubgraph::computeBoundingRect()
{
  kDebug() << subgraph()->id() << zValue();

  qreal adjust = 0.5;
  QRectF rect;
  if (subgraph()->renderOperations().isEmpty())
  {
    kDebug() << "no render operation";
    rect = QRectF(0,0,(m_view->defaultNewElementPixmap().size().width())*m_scaleX,(m_view->defaultNewElementPixmap().size().height())*m_scaleY);
    m_boundingRect = rect;
  }
  else
  {
    DotRenderOpVec::const_iterator it, it_end;
    it = subgraph()->renderOperations().begin(); it_end = subgraph()->renderOperations().end();
    for (; it != it_end; it++)
    {
      QString msg;
      QTextStream dd(&msg);
      dd << subgraph()->id() << " an op: " << (*it).renderop << " ";
      foreach (int i, (*it).integers)
      {
        dd << i << " ";
      }
      dd << (*it).str;
      kDebug() << msg;

      if ((*it).renderop == "e" || (*it).renderop == "E")
      {
        kDebug() << "integers[0]=" << (*it).integers[0] << "; m_wdhcf=" << m_wdhcf
            << "(*it).integers[0]/*%m_wdhcf*/=" << (*it).integers[0]/*%m_wdhcf*/;
        qreal w = m_scaleX * (*it).integers[2] * 2;
        qreal h = m_scaleY * (*it).integers[3] * 2;
        qreal x = m_xMargin + (((*it).integers[0]/*%m_wdhcf*/)*m_scaleX) - w/2;
        qreal y = ((m_gh - (*it).integers[1]/*%m_hdvcf*/)*m_scaleY) + m_yMargin - h/2;
        m_boundingRect = QRectF(x - adjust,y - adjust, w + adjust, h + adjust);
        kDebug() << "'" << subgraph()->id() << "' set rect for ellipse to " << rect;
      }
      else if  ((*it).renderop == "p" || (*it).renderop == "P")
      {
        QPolygonF polygon((*it).integers[0]);
        for (int i = 0; i < (*it).integers[0]; i++)
        {
          qreal x,y;
          x = ((*it).integers[2*i+1] == m_wdhcf)?(*it).integers[2*i+1]:(*it).integers[2*i+1]/*%m_wdhcf*/;
          y = ((*it).integers[2*i+2] == m_hdvcf)?(*it).integers[2*i+2]:(*it).integers[2*i+2]/*%m_hdvcf*/;
          {

          }
          QPointF p(
                    x*m_scaleX +m_xMargin,
                    (m_gh-y)*m_scaleY + m_yMargin
                  );
          polygon[i] = p;
        }
        m_boundingRect = polygon.boundingRect();
        kDebug() << "'" << subgraph()->id() << "' set rect for polygon to " << rect;
      }
    }
  }
  kDebug() << subgraph()->id() << "new bounding rect is:" << m_boundingRect;
}

void CanvasSubgraph::paint(QPainter* p, const QStyleOptionGraphicsItem *option,
QWidget *widget)
{
  Q_UNUSED(option)
  Q_UNUSED(widget)
  QString msg;
  QTextStream dd(&msg);
  foreach (DotRenderOp op, subgraph()->renderOperations())
  {
    dd << subgraph()->id() << " an op: " << op.renderop << " ";
    foreach (int i, op.integers)
    {
      dd << i << " ";
    }
    dd << op.str << endl;
  }
//   kDebug() << msg;

  if (subgraph()->renderOperations().isEmpty())
  {
//     kDebug() << "drawPixmap";
    p->drawPixmap(QPointF(0,0),m_view->defaultNewElementPixmap());
    return;
  }
  QListIterator<DotRenderOp> it(subgraph()->renderOperations());
  it.toBack();
  while (it.hasPrevious())
  {
    const DotRenderOp& dro = it.previous();
    if (dro.renderop == "e" || dro.renderop == "E")
    {
      qreal w = m_scaleX * dro.integers[2] * 2;
      qreal h = m_scaleY * dro.integers[3] * 2;
      qreal x = m_xMargin + ((dro.integers[0]/*%m_wdhcf*/)*m_scaleX) - w/2;
      qreal y = ((m_gh - dro.integers[1]/*%m_hdvcf*/)*m_scaleY) + m_yMargin - h/2;
      QRectF rect(x,y,w,h);
      p->save();
      p->setBrush(Dot2QtConsts::componentData().qtColor(subgraph()->backColor()));
      p->setPen(Dot2QtConsts::componentData().qtColor(subgraph()->lineColor()));
//       kDebug() << subgraph()->id() << "drawEllipse" << subgraph()->backColor() << rect;
      p->drawEllipse(rect);
      p->restore();
    }
    else if(dro.renderop == "p" || dro.renderop == "P")
    {
//       std::cerr << "Drawing polygon for node '"<<subgraph()->id()<<"': ";
      QPolygonF points(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        qreal x,y;
        x = (dro.integers[2*i+1] == m_wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]/*%m_wdhcf*/;
        y = (dro.integers[2*i+2] == m_hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]/*%m_hdvcf*/;
        QPointF p(
                  (x*m_scaleX) + m_xMargin,
                  ((m_gh-y)*m_scaleY) + m_yMargin
                );
/*        kDebug() << "    point: (" << dro.integers[2*i+1] << ","
                  << dro.integers[2*i+2] << ") " << m_wdhcf << "/" << m_hdvcf;*/
        points[i] = p;
      }
      p->save();
      QPen pen(Dot2QtConsts::componentData().qtColor(subgraph()->lineColor()));
      if (subgraph()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      else if (subgraph()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(subgraph()->style()));
      }
      if (subgraph()->style().left(12) == "setlinewidth")
      {
        bool ok;
        uint lineWidth = subgraph()->style().mid(12, subgraph()->style().length()-1-12).toInt(&ok);
        pen.setWidth(lineWidth);
      }
      p->setPen(pen);
      p->setBrush(Dot2QtConsts::componentData().qtColor(subgraph()->backColor()));
/*      if (subgraph()->style() == "filled")
      {
        p->setBrush(Dot2QtConsts::componentData().qtColor(subgraph()->backColor()));
    //     QCanvasPolygon::paint(p);
      }
      else
      {
        p->setBrush(canvas()->backgroundColor());
      }*/
      p->drawPolygon(points);
      p->restore();
      if (!subgraph()->shapeFile().isEmpty())
      {
        QPixmap pix(subgraph()->shapeFile());
        if (!pix.isNull())
        {
          p->drawPixmap(int(points.boundingRect().left()), int(points.boundingRect().top()), pix);
        }
      }
    }

  }

  it.toBack();
  while (it.hasPrevious())
  {
    const DotRenderOp& dro = it.previous();
    if ( dro.renderop == "L" )
    {
//       kDebug() << "Label";
      QPolygonF points(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        qreal x,y;
        x = (dro.integers[2*i+1] == m_wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]/*%m_wdhcf*/;
        y = (dro.integers[2*i+2] == m_hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]/*%m_hdvcf*/;
        QPointF p(
                  (x*m_scaleX) +m_xMargin,
                  ((m_gh-y)*m_scaleY) + m_yMargin
                );
        points[i] = p;
      }
      p->save();
      QPen pen(Dot2QtConsts::componentData().qtColor(subgraph()->lineColor()));
      if (subgraph()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      else if (subgraph()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(subgraph()->style()));
      }
      p->setPen(pen);
      kDebug() << subgraph()->id() << "drawPolyline" << points;
      p->drawPolyline(points);
      p->restore();
    }
  }


  DotRenderOpVec::const_iterator itl, itl_end;
  itl = subgraph()->renderOperations().begin();
  itl_end = subgraph()->renderOperations().end();
  for (; itl != itl_end; itl++)
  {
    const DotRenderOp& dro = (*itl);
    if ( dro.renderop == "T" )
    {
      // draw a label
/*      kDebug() << "Drawing a label " << (*it).integers[0]
        << " " << (*it).integers[1] << " " << (*it).integers[2]
        << " " << (*it).integers[3] << " " << (*it).str
        << " (" << subgraph()->fontName() << ", " << subgraph()->fontSize()
        << ", " << subgraph()->fontColor() << ")";*/

      int stringWidthGoal = int(dro.integers[3] * m_scaleX);
      int fontSize = subgraph()->fontSize();
      m_font->setPointSize(fontSize);
      QFontMetrics fm(*m_font);
      while (fm.width(dro.str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        m_font->setPointSize(fontSize);
        fm = QFontMetrics(*m_font);
      }
      p->save();
      p->setFont(*m_font);
      p->setPen(m_pen);
      qreal x = (m_scaleX *
                       (
                         (dro.integers[0])
                         + (((-dro.integers[2])*(fm.width(dro.str)))/2)
                         - ( (fm.width(dro.str))/2 )
                       )
                      )
                      + m_xMargin;
      qreal y = ((m_gh - (dro.integers[1]))*m_scaleY)+ m_yMargin;
      QPointF point(x,y);
//       kDebug() << subgraph()->id() << "drawText" << point;
      p->drawText(point, dro.str);
      p->restore();
    }
  }
//   p->drawRect(QRectF(pos(),boundingRect().bottomRight()));
}

/** @todo handle multiple comma separated styles 
 * @todo implement styles diagonals, rounded
 */
// void CanvasSubgraph::paint(QPainter& p)
// {
//   kDebug();
// /*  std::cerr << "CanvasSubgraph "<<subgraph()->id()<<" drawShape with style "
//     << subgraph()->style() << ", brush color " << subgraph()->backColor() 
//     << " and pen color " << subgraph()->lineColor() << std::endl;*/
//   p.save();
//   p.setPen(m_pen);
//   p.setBrush(m_brush);
// 
// //   kError() << "subgraph()->style().left(12): " << subgraph()->style().left(12) << endl;
//   p.drawPolygon(polygon());
//   
//   DotRenderOpVec::const_iterator it, it_end;
//   it = subgraph()->renderOperations().begin(); 
//   it_end = subgraph()->renderOperations().end();
//   for (; it != it_end; it++)
//   {
//     if ( (*it).renderop == "T" )
//     {
//       // draw a label
// /*      std::cerr << "Drawing a label " << (*it).integers[0] 
//       << " " << (*it).integers[1] << " " << (*it).integers[2]
//       << " " << (*it).integers[3] << " " << (*it).str 
//       << " (" << subgraph()->fontName() << ", " << subgraph()->fontSize()
//       << ", " << subgraph()->fontColor() << ")" << std::endl;*/
//       
//       int stringWidthGoal = int((*it).integers[3] * m_scaleX);
//       int fontSize = subgraph()->fontSize();
//       m_font->setPointSize(fontSize);
//       QFontMetrics fm(*m_font);
//       while (fm.width((*it).str) > stringWidthGoal+10  && fontSize > 1)
//       {
//         fontSize--;
//         m_font->setPointSize(fontSize);
//         fm = QFontMetrics(*m_font);
//       }
//       p.setFont(*m_font);
//       p.drawText(
//           int((m_scaleX * 
//           (
//           ((*it).integers[0]) 
//           + ((((*it).integers[2])*((*it).integers[3]))/2)
//           - ( ((*it).integers[3])/2 )
//           )
//               )
//           + m_xMargin ),
//       int(((m_gh - ((*it).integers[1]))*m_scaleY)+ m_yMargin),
//                  (*it).str);
//     }
//   }
//   p.restore();
// }

#include "canvassubgraph.moc"
