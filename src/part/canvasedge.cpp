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

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#include <iostream>
#include <qpainter.h>

#include "canvasedge.h"
#include "dotdefaults.h"
#include "dot2qtconsts.h"
#include "FontsCache.h"

//
// CanvasEdge
//

CanvasEdge::CanvasEdge(GraphEdge* e, QCanvas* c,
                       double scaleX, double scaleY, 
                       int xMargin, int yMargin, int gh,
                       int wdhcf, int hdvcf)
  : QCanvasPolygonalItem(c),  _edge(e), m_scaleX(scaleX),
    m_scaleY(scaleY), m_xMargin(xMargin), m_yMargin(yMargin),
    m_gh(gh), m_wdhcf(wdhcf), m_hdvcf(hdvcf),
    m_font(0)
{
  m_font = FontsCache::changeable().fromName(e->fontName().c_str());

//   std::cerr << "edge "  << e->fromNode()->id() << "->"  << e->toNode()->id() << std::endl;
  DotRenderOpVec::const_iterator it, it_end;
  it = edge()->renderOperations().begin(); 
  it_end = edge()->renderOperations().end();
  for (; it != it_end; it++)
  {
//     std::cerr << (*it).renderop  << ", ";
    if ( (*it).renderop != "B" ) continue;
    uint previousSize = m_points.size();
    m_points.resize(previousSize+(*it).integers[0]);
    for (int i = 0; i < (*it).integers[0]; i++)
    {
      QPoint p(
          int(((*it).integers[2*i+1]%m_wdhcf)*m_scaleX) +m_xMargin,
          int((m_gh-(*it).integers[2*i+2]%m_hdvcf)*m_scaleY) + m_yMargin
              );
      m_points[previousSize+i] = p;
    }
  }
//   std::cerr << std::endl;
  if (m_points.size() == 0) return;
  
  int minX = m_points[0].x(), minY = m_points[0].y();
  int maxX = minX, maxY = minY;
  int i;

  int len = m_points.count();
  for (i=1;i<len;i++) 
  {
    if (m_points[i].x() < minX) minX = m_points[i].x();
    if (m_points[i].y() < minY) minY = m_points[i].y();
    if (m_points[i].x() > maxX) maxX = m_points[i].x();
    if (m_points[i].y() > maxY) maxY = m_points[i].y();
  }
  QPointArray a = m_points.copy(),  b = m_points.copy();
  a.translate(-1, -1);
  b.translate(1, 1);
  a.resize(2*len);
  for (i=0;i<len;i++)
  {
    a[len+i] = b[i];
  }

  m_points = a;;
} 

void CanvasEdge::drawShape(QPainter& p)
{
  /// computes the scaling of line width
  int widthScaleFactor = int((m_scaleX+m_scaleY)/2);
  if (widthScaleFactor < 1)
  {
    widthScaleFactor = 1;
  }

  DotRenderOpVec::const_iterator it, it_end;
  it = edge()->renderOperations().begin(); 
  it_end = edge()->renderOperations().end();
  for (; it != it_end; it++)
  {
  
    if ( (*it).renderop == "T" )
    {
      QString str = QString::fromUtf8((*it).str.c_str());
    
      int stringWidthGoal = int((*it).integers[3] * m_scaleX);
      int fontSize = edge()->fontSize();
      m_font->setPointSize(fontSize);
      QFontMetrics fm(*m_font);
      while (fm.width(str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        m_font->setPointSize(fontSize);
        fm = QFontMetrics(*m_font);
      }
      p.save();
      p.setFont(*m_font);
      
      p.setPen(Dot2QtConsts::instance().qtColor(edge()->fontColor()));
      p.drawText(
          int((m_scaleX * 
          (
          ((*it).integers[0]) 
          + ((((*it).integers[2])*((*it).integers[3]))/2)
          - ( ((*it).integers[3])/2 )
          )
              )
          + m_xMargin ),
      int(((m_gh - ((*it).integers[1]))*m_scaleY)+ m_yMargin),
      str);
      p.restore();
    }      
    else if (( (*it).renderop == "p" ) || ((*it).renderop == "P" ))
    {
      QPointArray points((*it).integers[0]);
      for (int i = 0; i < (*it).integers[0]; i++)
      {
        QPoint p(
            int(((*it).integers[2*i+1]%m_wdhcf)*m_scaleX) +m_xMargin,
        int((m_gh-(*it).integers[2*i+2]%m_hdvcf)*m_scaleY) + m_yMargin
                );
        points[i] = p;
      }
      if ((*it).renderop == "P" )
      {
        p.save();
        p.setBrush(Dot2QtConsts::instance().qtColor(edge()->color(0)));
        p.drawPolygon(points);
        p.restore();
      }
      else
      {
        p.setBrush(Dot2QtConsts::instance().qtColor("white"));
      }
      QPen pen(Dot2QtConsts::instance().qtColor(edge()->color(0)));
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2 * widthScaleFactor);
      }
      else
      {
        pen.setWidth(1 * widthScaleFactor);
        pen.setStyle(Dot2QtConsts::instance().qtPenStyle(edge()->style()));
      }
      p.save();
      p.setPen(pen);
      p.drawPolyline(points);
      p.restore();
    }
    else if (( (*it).renderop == "e" ) || ((*it).renderop == "E" ))
    {
      double w = m_scaleX * (*it).integers[2] * 2;
      double h = m_scaleY *  (*it).integers[3] * 2;
      double x = (m_xMargin + ((*it).integers[0]%m_wdhcf)*m_scaleX) - w/2;
      double y = ((m_gh -  (*it).integers[1]%m_hdvcf)*m_scaleY + m_yMargin) - h/2;
      p.save();
      if ((*it).renderop == "E" )
      {
        p.setBrush(Dot2QtConsts::instance().qtColor(edge()->color(0)));
      }
      else
      {
        p.setBrush(Dot2QtConsts::instance().qtColor("white"));
      }
      QPen pen(Dot2QtConsts::instance().qtColor(edge()->color(0)));
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2 * widthScaleFactor);
      }
      else
      {
        pen.setWidth(1 * widthScaleFactor);
        pen.setStyle(Dot2QtConsts::instance().qtPenStyle(edge()->style()));
      }
      p.setPen(pen);
      p.drawEllipse(int(x),int(y),int(w),int(h));
      p.restore();
    }
    else if ( (*it).renderop == "B" )
    {
      uint lineWidth = 1;
      QPen pen;
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2 * widthScaleFactor);
      }
      else if (edge()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::instance().qtPenStyle(edge()->style()));
      }
      if (edge()->style().left(12) == "setlinewidth")
      {
        bool ok;
        lineWidth = edge()->style().mid(12, edge()->style().length()-1-12).toInt(&ok);
        pen.setWidth(lineWidth * widthScaleFactor);
      }
      for (uint splineNum = 0; splineNum < edge()->colors().count() || (splineNum==0 && edge()->colors().count()==0); splineNum++)
      {
        QPointArray points((*it).integers[0]);
        for (int i = 0; i < (*it).integers[0]; i++)
        {
          int nom = ((*it).integers[2*(*it).integers[0]]-(*it).integers[2]);
          int denom = ((*it).integers[2*(*it).integers[0]-1]-(*it).integers[1]);
          int diffX, diffY;
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
            double pente = ((double)nom)/denom;
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
          QPoint p(
              int(((*it).integers[2*i+1]%m_wdhcf)*m_scaleX) +m_xMargin + diffX,
              int((m_gh-(*it).integers[2*i+2]%m_hdvcf)*m_scaleY) + m_yMargin + diffY
                  );
          points[i] = p;
        }
        pen.setColor(Dot2QtConsts::instance().qtColor(edge()->color(splineNum)));
        p.save();
//         p.setBrush(Dot2QtConsts::instance().qtColor(edge()->color(0)));
        p.setPen(pen);
        for (uint j = 0; j < points.size()/3; j++)
        {
          p.drawCubicBezier(points, 3*j);
        }
        p.restore();
      }
    }      
  }
}


QPointArray CanvasEdge::areaPoints() const
{
  return m_points;
}

