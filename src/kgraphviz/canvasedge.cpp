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

#include "graphnode.h"
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

void CanvasEdge::computeBoundingRect()
{
  Q_D(CanvasEdge);
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
