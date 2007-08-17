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
                                          double scaleX, double scaleY, 
                                          int xMargin, int yMargin, int gh,
                                          int wdhcf, int hdvcf
                                        )
  : QGraphicsPolygonItem(0,c), 
    m_scaleX(scaleX), m_scaleY(scaleY), 
    m_xMargin(xMargin), m_yMargin(yMargin), 
    m_gh(gh), m_wdhcf(wdhcf), m_hdvcf(hdvcf), 
    m_subgraph(gsubgraph), m_view(v), 
    m_font(0),
    m_pen(Dot2QtConsts::componentData().qtColor(gsubgraph->fontColor()))
{
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
  
  DotRenderOpVec::const_iterator it, it_end;
  it = gsubgraph->renderOperations().begin(); it_end = gsubgraph->renderOperations().end();
  for (; it != it_end; it++)
  {
    if ((*it).renderop != "T")
    {
      gsubgraph->setCanvasSubgraph(this);
      setZValue(gsubgraph->z());
    
      const DotRenderOp& dro = (*it);
      if (dro.integers.empty())
      {
        kError() << "Invalid empty render operations integers vector" << endl;
        break;
      }
      else if ( (dro.integers[0]-1)*2+2 >= dro.integers.size() )
      {
        kError() << "Invalid render operations vector. Says " << dro.integers[0] << "points but size is "<< dro.integers.size() << endl;
        break;
      }
      QPolygonF polygon(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        int x,y;
        x = (dro.integers[2*i+1] == wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]%wdhcf;
        y = (dro.integers[2*i+2] == hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]%hdvcf;
        {
      
        }
        QPointF p(
            x*scaleX +xMargin,
        (gh-y)*scaleY + yMargin
                );
/*        kDebug() << "    point: (" << dro.integers[2*i+1] << ","
            <<dro.integers[2*i+2]<< ") -> " << p << endl;*/
        polygon[i] = p;
      }
      setPolygon(polygon);
    }
    break;
  }
}


/** @todo handle multiple comma separated styles 
 * @todo implement styles diagonals, rounded
 */
void CanvasSubgraph::paint(QPainter& p)
{
/*  std::cerr << "CanvasSubgraph "<<subgraph()->id()<<" drawShape with style "
    << subgraph()->style() << ", brush color " << subgraph()->backColor() 
    << " and pen color " << subgraph()->lineColor() << std::endl;*/
  p.save();
  p.setPen(m_pen);
  p.setBrush(m_brush);

//   kError() << "subgraph()->style().left(12): " << subgraph()->style().left(12) << endl;
  p.drawPolygon(polygon());
  
  DotRenderOpVec::const_iterator it, it_end;
  it = subgraph()->renderOperations().begin(); 
  it_end = subgraph()->renderOperations().end();
  for (; it != it_end; it++)
  {
    if ( (*it).renderop == "T" )
    {
      // draw a label
/*      std::cerr << "Drawing a label " << (*it).integers[0] 
      << " " << (*it).integers[1] << " " << (*it).integers[2]
      << " " << (*it).integers[3] << " " << (*it).str 
      << " (" << node()->fontName() << ", " << node()->fontSize()
      << ", " << node()->fontColor() << ")" << std::endl;*/
      
      int stringWidthGoal = int((*it).integers[3] * m_scaleX);
      int fontSize = subgraph()->fontSize();
      m_font->setPointSize(fontSize);
      QFontMetrics fm(*m_font);
      while (fm.width((*it).str) > stringWidthGoal+10  && fontSize > 1)
      {
        fontSize--;
        m_font->setPointSize(fontSize);
        fm = QFontMetrics(*m_font);
      }
      p.setFont(*m_font);
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
                 (*it).str);
    }
  }
  p.restore();
}

