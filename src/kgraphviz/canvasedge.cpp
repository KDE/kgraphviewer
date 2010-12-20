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

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


#include "canvasedge.h"
#include "canvasedge_p.h"

#include "canvaselement.h"
#include "dotgraphview.h"
#include "graphedge.h"
#include "support/dotdefaults.h"
#include "support/dot2qtconsts.h"
#include "support/fontscache.h"

#include <KDebug>
#include <KLocale>

#include <QPainter>

#include <math.h>
#include <iostream>

using namespace KGraphViz;

CanvasEdgePrivate::CanvasEdgePrivate()
{
}

CanvasEdgePrivate::~CanvasEdgePrivate()
{
}

CanvasEdge::CanvasEdge(DotGraphView* view,
                       GraphEdge* e,
                       QGraphicsScene* scene,
                       QGraphicsItem* parent) :
  CanvasElement(view, e, scene, parent),
  d_ptr(new CanvasEdgePrivate)
{
  kDebug() << "edge "  << edge()->fromNode()->id() << "->"  << edge()->toNode()->id() << CanvasEdge::gh();
  setBoundingRegionGranularity(0.9);
  setFont(*FontsCache::changeable().fromName(e->fontName()));

  kDebug() << "boundingRect computed: " << boundingRect();

  QString tipStr = i18n("%1 -> %2\nlabel='%3'",
      edge()->fromNode()->id(),edge()->toNode()->id(),e->label());
  setToolTip(tipStr);
} 

CanvasEdge::~CanvasEdge()
{
  delete d_ptr;
}

QPainterPath CanvasEdge::shape () const
{
  Q_D(const CanvasEdge);
//   kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id();
  if (d->m_boundingRegion.isEmpty()) {
    d->m_boundingRegion.addRegion(boundingRegion(QTransform()));
  }
  return d->m_boundingRegion;
  /*
  foreach (const DotRenderOp& dro, edge()->renderOperations())
  {
    if ( dro.renderop == "B" )
    {
      for (int splineNum = 0; splineNum < edge()->colors().count() || (splineNum==0 && edge()->colors().count()==0); splineNum++)
      {
        QPolygonF points(dro.integers[0]);
        for (int i = 0; i < dro.integers[0]; i++)
        {
          qreal nom = (dro.integers[2*dro.integers[0]]-dro.integers[2]);
          qreal denom = (dro.integers[2*dro.integers[0]-1]-dro.integers[1]);
          qreal diffX, diffY;
          if (nom == 0)
          {
            diffX = 0;
            diffY = 2*(edge()->colors().count()/2 - splineNum);
          }
          else if (denom ==0)
          {
            diffX = 2*(edge()->colors().count()/2 - splineNum);
            diffY = 0;
          }
          else
          {
            double pente = nom/denom;
            if (pente < 0)
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = edge()->colors().count()/2 + splineNum;
            }
            else
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = 2*(edge()->colors().count()/2 - splineNum);
            }
          }
          QPointF p(
              //NOTE: when uncommenting, fix nested comments in here:
              (dro.integers[2*i+1]/ *%m_wdhcf* /*scaleX()) +marginX() + diffX,
              (gh()-dro.integers[2*i+2]/ *%m_hdvcf* /)*scaleY() + marginY() + diffY
                  );
          points[i] = p;
        }
        path.moveTo(points[0]);
        for (int j = 0; j < (points.size()-1)/3; j++)
        {
          path.cubicTo(points[3*j + 1],points[3*j+1 + 1], points[3*j+2 + 1]);
        }
        for (int j = (points.size()-1)/3-3; j >= 0 ; j--)
        {
          path.cubicTo(points[3*j + 1],points[3*j+1 + 1], points[3*j+2 + 1]);
        }
      }
    }
  }
  return path;
  */
}



void CanvasEdge::paint(QPainter* p, const QStyleOptionGraphicsItem* option,
                   QWidget* widget)
{
  Q_UNUSED(option)
  Q_UNUSED(widget)

  if (boundingRect() == QRectF())
    return;

  /// computes the scaling of line width
  qreal widthScaleFactor = (scaleX()+scaleY())/2;
  if (widthScaleFactor < 1)
  {
    widthScaleFactor = 1;
  }

  if (edge()->style()=="invis")
  {
    return;
  }
  if (edge()->renderOperations().isEmpty())
  {
    if ((edge()->fromNode()->canvasElement()!=0)
      && (edge()->toNode()->canvasElement()!=0))
    {
      p->drawLine(
        edge()->fromNode()->canvasElement()->boundingRect().center()+edge()->fromNode()->canvasElement()->pos(),
        edge()->toNode()->canvasElement()->boundingRect().center()+edge()->toNode()->canvasElement()->pos());
    }
    return;
  }

  QColor lineColor = Dot2QtConsts::componentData().qtColor(edge()->color(0));
  QColor backColor;
  
  QList<QPointF> allPoints;
  foreach (const DotRenderOp& dro, edge()->renderOperations())
  {
    //     kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "renderop" << dro.renderop << "; selected:" << edge()->isSelected();
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
/*      if (m_hovered && m_view->highlighting())
      {
        c = c.lighter();
      }*/
      backColor = c;
//       kDebug() << "C" << dro.str.mid(0,7) << backColor;
    }
    else if ( dro.renderop == "T" )
    {
      const QString& str = dro.str;
    
      qreal stringWidthGoal = dro.integers[3] * scaleX();
      int fontSize = edge()->fontSize();
      QFont font = CanvasElement::font();
      font.setPointSize(fontSize);
      QFontMetrics fm(font);
      while (fm.width(str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        font.setPointSize(fontSize);
        fm = QFontMetrics(font);
      }
      p->save();
      p->setFont(font);
      
      p->setPen(Dot2QtConsts::componentData().qtColor(edge()->fontColor()));

      qreal x = (scaleX() *
                       (
                         (dro.integers[0])
                         + (((-dro.integers[2])*(fm.width(dro.str)))/2)
                         - ( (fm.width(dro.str))/2 )
                       )
                      )
                      + marginX();
      qreal y = ((gh() - (dro.integers[1]))*scaleY())+ marginY();
      QPointF point(x,y);
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawText" << edge()->fontColor() << point;

      p->drawText(point,str);
      p->restore();

      // save back
      setFont(font);
    }      
    else if (( dro.renderop == "p" ) || (dro.renderop == "P" ))
    {
      QPolygonF polygon(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        QPointF point(
            (int(dro.integers[2*i+1])/*%m_wdhcf*/)*scaleX() +marginX(),
            (int(gh()-dro.integers[2*i+2])/*%m_hdvcf*/)*scaleY() + marginY()
                );
        polygon[i] = point;
//         kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id()  << point;
        allPoints.append(point);
      }
      if (dro.renderop == "P" )
      {
        p->save();
        p->setBrush(lineColor);
        p->drawPolygon(polygon);
//         kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawPolygon" << edge()->color(0) << polygon;
        p->restore();
      }
      else
      {
        p->setBrush(Dot2QtConsts::componentData().qtColor("white"));
      }
      QPen pen(lineColor);
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth((int)(2 * widthScaleFactor));
      }
      else
      {
        pen.setWidth((int)(1 * widthScaleFactor));
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(edge()->style()));
      }
      p->save();
      p->setPen(pen);
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawPolyline" << edge()->color(0) << polygon;
      p->drawPolyline(polygon);
      p->restore();
    }
    else if (( dro.renderop == "e" ) || (dro.renderop == "E" ))
    {
      qreal w = scaleX() * dro.integers[2] * 2;
      qreal h = scaleY() *  dro.integers[3] * 2;
      qreal x = (marginX() + (dro.integers[0]/*%m_wdhcf*/)*scaleX()) - w/2;
      qreal y = ((gh() -  dro.integers[1]/*%m_hdvcf*/)*scaleY() + marginY()) - h/2;
      p->save();
      if (dro.renderop == "E" )
      {
        p->setBrush(lineColor);
      }
      else
      {
        p->setBrush(Dot2QtConsts::componentData().qtColor("white"));
      }
      QPen pen(lineColor);
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(int(2 * widthScaleFactor));
      }
      else
      {
        pen.setWidth(int(1 * widthScaleFactor));
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(edge()->style()));
      }
      p->setPen(pen);
      QRectF rect(x,y,w,h);
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawEllipse" << edge()->color(0) << rect;
      p->drawEllipse(rect);
      p->restore();
    }
    else if ( dro.renderop == "B" )
    {
      uint lineWidth = 1;
      QPen pen;
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(int(2 * widthScaleFactor));
      }
      else if (edge()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(edge()->style()));
      }
      if (edge()->style().left(12) == "setlinewidth")
      {
        bool ok;
        lineWidth = edge()->style().mid(12, edge()->style().length()-1-12).toInt(&ok);
        pen.setWidth(int(lineWidth * widthScaleFactor));
      }
      if (edge()->attributes().contains("penwidth"))
      {
        bool ok;
        lineWidth = edge()->attributes()["penwidth"].toInt(&ok);
        pen.setWidth(int(lineWidth * widthScaleFactor));
      }
      if (edge()->attributes().contains("color"))
      {
        kDebug() << "set edge color to " << QColor(edge()->attributes()["color"]).name();
        lineColor = QColor(edge()->attributes()["color"]);
      }
      for (int splineNum = 0; splineNum < edge()->colors().count() || (splineNum==0 && edge()->colors().count()==0); splineNum++)
      {
        QPolygonF points(dro.integers[0]);
        for (int i = 0; i < dro.integers[0]; i++)
        {
          // computing of diffX and diffY to draw parallel edges
          // when asked through the corresponding GraphViz feature
          qreal nom = (dro.integers[2*dro.integers[0]]-dro.integers[2]);
          qreal denom = (dro.integers[2*dro.integers[0]-1]-dro.integers[1]);
          qreal diffX, diffY;
          if (nom == 0)
          {
            diffX = 0;
            diffY = 2*(edge()->colors().count()/2 - splineNum);
          }
          else if (denom ==0)
          {
            diffX = 2*(edge()->colors().count()/2 - splineNum);
            diffY = 0;
          }
          else
          {
            double pente = nom/denom;
            if (pente < 0)
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = edge()->colors().count()/2 + splineNum;
            }
            else
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = 2*(edge()->colors().count()/2 - splineNum);
            }
          }
          QPointF p(
              (dro.integers[2*i+1]/*%m_wdhcf*/*scaleX()) +marginX() + diffX,
              (gh()-dro.integers[2*i+2]/*%m_hdvcf*/)*scaleY() + marginY() + diffY
                  );
          points[i] = p;
//           kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id()  << p;
          allPoints.append(p);
        }

//        kDebug() << "Setting pen color to " << edge()->color(splineNum);
        if (splineNum != 0)
          lineColor = Dot2QtConsts::componentData().qtColor(edge()->color(splineNum));
        pen.setColor(lineColor);
        p->save();
//         p->setBrush(Dot2QtConsts::componentData().qtColor(edge()->color(0)));
        p->setBrush(Qt::NoBrush);
        p->setPen(pen);
        QPainterPath path;
        path.moveTo(points[0]);
        for (int j = 0; j < (points.size()-1)/3; j++)
        {
          path.cubicTo(points[3*j + 1],points[3*j+1 + 1], points[3*j+2 + 1]);
        }
//         kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawPath" << edge()->color(splineNum) << points.first() << points.last();
        p->drawPath(path);
        p->restore();
      }
    }
  }
  if (edge()->isSelected())
  {
//     kDebug() << "draw square";
//     p->drawRect(m_boundingRect);
    qreal maxDist = 0;
    QPair<QPointF,QPointF> pointsPair;
    foreach(const QPointF& point1, allPoints)
    {
      foreach(const QPointF& point2, allPoints)
      {
        if (QLineF(point1, point2).length() > maxDist)
        {
          maxDist = QLineF(point1, point2).length();
          pointsPair = qMakePair(point1, point2);
        }
      }
    }
    if (maxDist>0)
    {
      p->save();
      //         p->setBrush(Dot2QtConsts::componentData().qtColor(edge()->color(0)));
      p->setBrush(Qt::black);
      p->setPen(Qt::black);
      p->drawRect(QRectF(pointsPair.first-QPointF(3,3),QSizeF(6,6)));
      p->drawRect(QRectF(pointsPair.second-QPointF(3,3),QSizeF(6,6)));
      p->restore();
    }
  }
}

void CanvasEdge::computeBoundingRect()
{
  Q_D(CanvasEdge);
  kDebug();
  //invalidate bounding region cache
  d->m_boundingRegion = QPainterPath();
  if (edge()->renderOperations().isEmpty())
  {
    if ((edge()->fromNode()->canvasElement()==0)
      || (edge()->toNode()->canvasElement()==0)
      || edge()->style()=="invis")
    {
      setBoundingRect(QRectF());
    }
    else
    {
      QRectF br(
      edge()->fromNode()->canvasElement()->boundingRect().center()+edge()->fromNode()->canvasElement()->pos(),
                edge()->toNode()->canvasElement()->boundingRect().center()+edge()->toNode()->canvasElement()->pos());
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() <<br;
      setBoundingRect(br);
    }
  }
  else
  {
    QPolygonF points;
    foreach (const DotRenderOp& dro, edge()->renderOperations())
    {
//       kDebug() << dro.renderop  << ", ";
      if ( (dro.renderop != "B") && (dro.renderop != "p") &&  (dro.renderop != "P") ) continue;
      uint previousSize = points.size();
      points.resize(previousSize+dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        QPointF p(
            ((dro.integers[2*i+1]/*%m_wdhcf*/)*scaleX()) +marginX(),
            ((gh()-dro.integers[2*i+2]/*%m_hdvcf*/)*scaleY()) + marginY()
                );
        points[previousSize+i] = p;
      }
    }
//     kDebug() << points.size() << "points";
    if (points.size() == 0) return;

    int len = points.count();
    QPolygonF a = points,  b = points;
    a.translate(-1, -1);
    b.translate(1, 1);
    a.resize(2*len);
    for (int i=0;i<len;i++)
    {
      a[len+i] = b[i];
    }
//     kDebug() << a.size() << "points";

    setBoundingRect(a.boundingRect());
  }
  kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "New bounding rect is:" << boundingRect();
}

GraphEdge* CanvasEdge::edge() const
{
  return dynamic_cast<GraphEdge*>(element());
}
