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

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <qtooltip.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qwhatsthis.h>
#include <qcanvas.h>
#include <qwmatrix.h>
#include <qpair.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstyle.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktempfile.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>

#include "dotgraphview.h"
#include "graphnode.h"
#include "canvasnode.h"
#include "dotdefaults.h"
#include "dot2qtconsts.h"
#include "FontsCache.h"

//
// CanvasNode
//

CanvasNode::CanvasNode(DotGraphView* v, GraphNode* n)
: m_node(n), m_view(v), m_hasFocus(false), 
  m_font(0),
  m_pen(Dot2QtConsts::instance().qtColor(n->fontColor()))

{
  m_font = FontsCache::changeable().fromName(n->fontName());
}

CanvasNode* CanvasNode::dotShapedCanvasNode(const DotRenderOp& dro, 
                                            const DotRenderOpVec& dros,
                                            DotGraphView* v, 
                                            GraphNode* n,
                                            QCanvas* c,
                                            double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                                            int wdhcf, int hdvcf)
{
  CanvasNode* res = 0;
/*  std::cerr << "dotShapedCanvasNode: creating node for " << n->label() 
            << " ; dro = " << dro.renderop << std::endl;*/
  if (n->label()[0] == '<' && n->shape() != "record")
  {
//     std::cerr << "dotShapedCanvasNode: creating CanvasHtmlNode" << std::endl;
    res = new CanvasHtmlNode(v, n, dro, dros, c,
                             scaleX, scaleY, xMargin, yMargin, gh,
                             wdhcf, hdvcf);
  }
  else if (dro.renderop == "e" || dro.renderop == "E")
  {
//   kdDebug() << "dotShapedCanvasNode: creating CanvasEllipseNode" << endl;
    res = new CanvasEllipseNode(v, n, dro, dros, c,
                                scaleX, scaleY, xMargin, yMargin, gh,
                                wdhcf, hdvcf);
  }
  else if(dro.renderop == "p" || dro.renderop == "P" || dro.renderop == "T")
  {
//     std::cerr << "dotShapedCanvasNode: creating CanvasPolygonalNode" << std::endl;
    res = new CanvasPolygonalNode(v, n, dro, dros, c,
                                  scaleX, scaleY, xMargin, yMargin, gh,
                                  wdhcf, hdvcf);
  }
  else if (dro.renderop == "L" || dro.renderop == "B"  || dro.renderop == "b")
  {
    kdWarning() << "xdot render operation '" << QString(dro.renderop.c_str()) << "' is currently "
      "not supported (ignored)." << endl;
  }
  else if (
             dro.renderop == "C" || dro.renderop == "c" 
          || dro.renderop == "F" || dro.renderop == "S")
  {
    kdWarning() << "xdot render operation '" << QString(dro.renderop.c_str()) << "' is currently "
      "not supported (ignored).\nUsually its value is handled through"
      " standard attributes." << endl;
  }
  else 
  {
    kdError() << "Error ! Unknown node shape '" << QString(dro.renderop.c_str()) << "' ; ignoring it." << endl;
    //    res = new CanvasPolygonalNode(v, n, dro, c);
  }
  if (res != 0)
  {
    res->m_scaleX = scaleX; 
    res->m_scaleY = scaleY; 
    res->m_xMargin = xMargin; 
    res->m_yMargin = yMargin;
    res->m_gh = gh;
    res->m_wdhcf = wdhcf;
    res->m_hdvcf = hdvcf;
  }
  return res;
}

void CanvasNode::drawShape(QPainter& p)
{
  DotRenderOpVec::const_reverse_iterator it, it_end;
  it = node()->renderOperations().rbegin(); 
  it_end = node()->renderOperations().rend();
  for (; it != it_end; it++)
  {
    const DotRenderOp& dro = (*it);
    if (dro.renderop == "e" || dro.renderop == "E")
    {
      int w = int(m_scaleX * dro.integers[2]) * 2;
      int h = int(m_scaleY * dro.integers[3]) * 2;
      int x = m_xMargin + int((dro.integers[0]%m_wdhcf)*m_scaleX) - w/2;
      int y = int((m_gh - dro.integers[1]%m_hdvcf)*m_scaleY) + m_yMargin - h/2;
      p.save();
      p.setBrush(Dot2QtConsts::instance().qtColor(node()->backColor()));
      p.setPen(Dot2QtConsts::instance().qtColor(node()->lineColor()));
      p.drawEllipse(x, y, w, h);
      p.restore();
    }
    else if(dro.renderop == "p" || dro.renderop == "P")
    {
//       std::cerr << "Drawing polygon for node '"<<node()->id()<<"': ";
      QPointArray points(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        int x,y;
        x = (dro.integers[2*i+1] == m_wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]%m_wdhcf;
        y = (dro.integers[2*i+2] == m_hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]%m_hdvcf;
        QPoint p(
                  int(x*m_scaleX) + m_xMargin,
                  int((m_gh-y)*m_scaleY) + m_yMargin
                );
/*        std::cerr << "    point: (" << dro.integers[2*i+1] << ","
                  << dro.integers[2*i+2] << ") " << m_wdhcf << "/" << m_hdvcf << std::endl;*/
        points[i] = p;
      }
      p.save();
      QPen pen(Dot2QtConsts::instance().qtColor(node()->lineColor()));
      if (node()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      else if (node()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::instance().qtPenStyle(node()->style()));
      }
      if (node()->style().left(12) == "setlinewidth")
      {
        bool ok;
        uint lineWidth = node()->style().mid(12, node()->style().length()-1-12).toInt(&ok);
        pen.setWidth(lineWidth);
      }
      p.setPen(pen);
      p.setBrush(Dot2QtConsts::instance().qtColor(node()->backColor()));
/*      if (node()->style() == "filled")
      {
        p.setBrush(Dot2QtConsts::instance().qtColor(node()->backColor()));
    //     QCanvasPolygon::drawShape(p);
      }
      else
      {
        p.setBrush(canvas()->backgroundColor());
      }*/
      p.drawPolygon(points);
      p.restore();
      if (!node()->shapeFile().isEmpty())
      {
        QPixmap pix(node()->shapeFile());
        if (!pix.isNull())
        {
          p.drawPixmap(points.boundingRect().left(), points.boundingRect().top(), pix);
        }
      }
    }

  }

  it = node()->renderOperations().rbegin(); 
  it_end = node()->renderOperations().rend();
  for (; it != it_end; it++)
  {
    const DotRenderOp& dro = (*it);
    if ( (*it).renderop == "L" )
    {
      QPointArray points(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        int x,y;
        x = (dro.integers[2*i+1] == m_wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]%m_wdhcf;
        y = (dro.integers[2*i+2] == m_hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]%m_hdvcf;
        QPoint p(
                  int(x*m_scaleX) +m_xMargin,
                  int((m_gh-y)*m_scaleY) + m_yMargin
                );
        points[i] = p;
      }
      p.save();
      QPen pen(Dot2QtConsts::instance().qtColor(node()->lineColor()));
      if (node()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      else if (node()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::instance().qtPenStyle(node()->style()));
      }
      p.setPen(pen);
      p.drawPolyline(points);
      p.restore();
    }
  }


  DotRenderOpVec::const_iterator itl, itl_end;
  itl = node()->renderOperations().begin(); 
  itl_end = node()->renderOperations().end();
  for (; itl != itl_end; itl++)
  {
    const DotRenderOp& dro = (*itl);
    if ( dro.renderop == "T" )
    {
      QString str = QString::fromUtf8(dro.str.c_str());
      // draw a label
/*      std::cerr << "Drawing a label " << (*it).integers[0] 
        << " " << (*it).integers[1] << " " << (*it).integers[2]
        << " " << (*it).integers[3] << " " << (*it).str 
        << " (" << node()->fontName() << ", " << node()->fontSize()
        << ", " << node()->fontColor() << ")" << std::endl;*/
      
      int stringWidthGoal = int(dro.integers[3] * m_scaleX);
      int fontSize = node()->fontSize();
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
      p.setPen(m_pen);
      p.drawText(
                  int((m_scaleX * 
                       (
                         (dro.integers[0]) 
                         + (((-dro.integers[2])*(dro.integers[3]))/2)
                         - ( (dro.integers[3])/2 )
                       )
                      )
                      + m_xMargin ),
                  int(((m_gh - (dro.integers[1]))*m_scaleY)+ m_yMargin),
                  str);
      p.restore();
    }
  }

}

CanvasPolygonalNode::CanvasPolygonalNode(DotGraphView* v,GraphNode* n, const QPointArray& points, QCanvas* c)
: QCanvasPolygon(c), CanvasNode(v, n)
{
  setPoints(points);
}

CanvasPolygonalNode::CanvasPolygonalNode(
                                          DotGraphView* v, 
                                          GraphNode* n,
                                          const DotRenderOp& dro,
                                          const DotRenderOpVec& dros,
                                          QCanvas* c,
                                          double scaleX, double scaleY, 
                                          int xMargin, int yMargin, int gh,
                                          int wdhcf, int hdvcf
                                        )
: QCanvasPolygon(c), CanvasNode(v, n)
{
  m_renderOperations = dros;

// // /*  kdDebug() << "Creating CanvasPolygonalNode for "<<n->id()<<" with " << dro.integers[0] << " points." << endl;
// //   kdDebug() << "    data: " << wdhcf << "," << hdvcf << "," << gh << "," 
// //     << scaleX << "," << scaleY << "," << xMargin << "," << yMargin << endl;*/
  m_scaleX = scaleX; m_scaleY = scaleY;
  m_xMargin = xMargin; m_yMargin = yMargin;
  m_gh = gh; m_wdhcf = wdhcf; m_hdvcf = hdvcf;
  QPointArray points(dro.integers[0]);
  for (int i = 0; i < dro.integers[0]; i++)
  {
    int x,y;
    x = (dro.integers[2*i+1] == wdhcf)?dro.integers[2*i+1]:dro.integers[2*i+1]%wdhcf;
    y = (dro.integers[2*i+2] == hdvcf)?dro.integers[2*i+2]:dro.integers[2*i+2]%hdvcf;
    {
      
    }
    QPoint p(
              int(x*scaleX) +xMargin,
              int((gh-y)*scaleY) + yMargin
            );
/*  kdDebug() << "    point: (" << dro.integers[2*i+1] << ","
      <<dro.integers[2*i+2]<< ") -> " << p << endl;*/
    points[i] = p;
  }
  setPoints(points);
}

/** @todo handle multiple comma separated styles 
 * @todo implement styles diagonals, rounded
 */
void CanvasPolygonalNode::drawShape(QPainter& p)
{
  CanvasNode::drawShape(p);
  
  // drawing of a colored line around the node if it has focus
/*  if (m_hasFocus)
  {
    p.save();
    QPen pen(Dot2QtConsts::instance().qtColor("blue"));
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawPolyline(points());
    p.restore();
  }*/
}

void CanvasPolygonalNode::update()
{
  QCanvasPolygon::update();
}

CanvasEllipseNode::CanvasEllipseNode(DotGraphView* v, GraphNode* n,
                                             int x, int y, int w, int h, QCanvas* c)
: QCanvasEllipse(w,h,c), CanvasNode(v, n)
{
  QCanvasEllipse::setX(x+w/2); QCanvasEllipse::setY(y+h/2);
}

CanvasEllipseNode::CanvasEllipseNode(
                   DotGraphView* v, 
                   GraphNode* n,
                   const DotRenderOp& dro,
                   const DotRenderOpVec& dros,
                   QCanvas* c,
                   double scaleX, double scaleY, int xMargin, int yMargin, int gh,
                                      int wdhcf, int hdvcf)
: QCanvasEllipse(c), CanvasNode(v, n)
{
/*  kdDebug() << "CanvasEllipseNode " << dro.integers.size() << " / " 
    << dro.integers[0] << " / " << dro.integers[1] << " / " 
    << dro.integers[2] << " / " << dro.integers[3] << endl; */
  m_renderOperations = dros;
  m_scaleX = scaleX; m_scaleY = scaleY;
  m_xMargin = xMargin; m_yMargin = yMargin;
  m_gh = gh; m_wdhcf = wdhcf; m_hdvcf = hdvcf;
  int w = int(scaleX * dro.integers[2]) * 2;
  int h = int(scaleY * dro.integers[3]) * 2;
  setSize(w,h);
  int x = xMargin + int((dro.integers[0]%wdhcf)*scaleX);
  int y = int((gh - dro.integers[1]%hdvcf)*scaleY) + yMargin;
  QCanvasEllipse::setX(x); QCanvasEllipse::setY(y);
}

void CanvasEllipseNode::drawShape(QPainter& p)
{
  
  CanvasNode::drawShape(p);
}

CanvasHtmlNode::CanvasHtmlNode(
                                          DotGraphView* v, 
                                          GraphNode* n,
                                          const DotRenderOp& dro,
                                          const DotRenderOpVec& dros,
                                          QCanvas* c,
                                          double scaleX, double scaleY, 
                                          int xMargin, int yMargin, int gh,
                                          int wdhcf, int hdvcf
                                        )
: KHTMLPart(v->viewport()), CanvasNode(v, n)
{
  m_renderOperations = dros;
//   std::cerr << "Creating "<<node()->id()<<" CanvasHtmlNode for" << n 
//     << " with label '" << n->label() << "'" << std::endl;

  QString myHTMLCode = n->label();
  myHTMLCode = myHTMLCode.mid(1, myHTMLCode.length() - 2);
//   std::cerr << "HTML = " << myHTMLCode << std::endl;
  begin(KURL(QString("file:") + QDir::currentDirPath() + "/index.html"));
  setAutoloadImages(true);
  write(myHTMLCode);
  kdDebug() << "HTML written." << endl;
  end();
  setStatusMessagesEnabled (false);
//   view()->setFrameShape ( QFrame::NoFrame );
//   view()->setFrameShadow ( QFrame::Plain );
//   view()->setLineWidth ( 0 );
//   view()->setMidLineWidth ( 0 );
  view()->setHScrollBarMode ( QScrollView::AlwaysOff );
  view()->setVScrollBarMode ( QScrollView::AlwaysOff );
  view()->setMarginWidth(0);
  view()->setMarginHeight(0);
  m_zoomFactor = m_view->zoom();
  view()->part()->setZoomFactor(int(m_zoomFactor*100));
  view()->move(int(n->x()*scaleX*m_zoomFactor), int((gh-n->y())*scaleY*m_zoomFactor));
  view()->setMinimumSize(int(n->w()*scaleX),int(n->h()*scaleY*m_zoomFactor));
  view()->setMaximumSize(int(n->w()*scaleX),int(n->h()*scaleY*m_zoomFactor));
  view()->adjustSize();
  KHTMLPart::show();
  connect(v, SIGNAL(contentsMoving ( int, int)), this, SLOT(move(int, int)));
  connect(v, SIGNAL(zoomed (double)), this, SLOT(zoomed(double)));
}

CanvasHtmlNode::~CanvasHtmlNode() 
{
  KHTMLPart::hide();
}

// void CanvasHtmlNode::drawShape(QPainter& p)
// {
//   view()->drawContents(&p);
// }

void CanvasHtmlNode::move(int x, int y)
{
//   std::cerr << "CanvasHtmlNode::move("<<x<<", "<<y<<")" << std::endl;
  m_xMovedTo = x; m_yMovedTo = y;
  view()->move(int((node()->x())*m_scaleX*m_zoomFactor - m_xMovedTo), int((m_gh-node()->y())*m_scaleY*m_zoomFactor) - m_yMovedTo);
//   view()->move(int(x*m_scaleX), int((m_gh-y)*m_scaleY));
}

void CanvasHtmlNode::zoomed(double factor)
{
  m_zoomFactor = factor;
  view()->part()->setZoomFactor(int(factor*100));
  view()->move(int(node()->x()*m_scaleX*m_zoomFactor - m_xMovedTo), int((m_gh-node()->y())*m_scaleY*m_zoomFactor - m_yMovedTo));
  view()->setMinimumSize(int(node()->w()*m_scaleX*m_zoomFactor),int(node()->h()*m_scaleY*m_zoomFactor));
  view()->setMaximumSize(int(node()->w()*m_scaleX*m_zoomFactor),int(node()->h()*m_scaleY*m_zoomFactor));
  view()->adjustSize();
}
