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


/*
 * Callgraph View
 */

#include "canvasnode.h"
#include "dotgraphview.h"
#include "graphnode.h"
#include "dotdefaults.h"
#include "dot2qtconsts.h"
#include "FontsCache.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QGraphicsScene>
#include <QMatrix>
#include <QPainter>
#include <QStyle>
#include <QPolygonF>
#include <QPixmap>

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

CanvasNode::CanvasNode(DotGraphView* v, GraphNode* n, QGraphicsItem* parent)
: QObject(), QAbstractGraphicsShapeItem(parent), 
  m_node(n), m_view(v), m_hasFocus(false), 
  m_font(0),
  m_pen(Dot2QtConsts::componentData().qtColor(n->fontColor()))

{
  qDebug() << k_funcinfo << n->id();
  m_font = FontsCache::changeable().fromName(n->fontName());
  connect(n,SIGNAL(changed()),this,SLOT(modelChanged()));

  QString tipStr;
  QString id = n->id();
  QString label = n->label();
  tipStr = i18n("id='%1'\nlabel='%2'",id,label);
  qDebug() << "CanvasEllipseNode setToolTip " << tipStr;
  setToolTip(tipStr);
}

void CanvasNode::initialize(qreal scaleX, qreal scaleY,
                            qreal xMargin, qreal yMargin, qreal gh,
                            qreal wdhcf, qreal hdvcf)
{
  setFlag(QGraphicsItem::ItemIsMovable, true);

  m_scaleX = scaleX; m_scaleY = scaleY;
  m_xMargin = xMargin; m_yMargin = yMargin;
  m_gh = gh; m_wdhcf = wdhcf; m_hdvcf = hdvcf;

  setZValue(m_node->z());

  computeBoundingRect();
}

QRectF CanvasNode::boundingRect () const
{
  return m_boundingRect;
}

void CanvasNode::computeBoundingRect()
{
  qDebug() << k_funcinfo << node()->id();

  qreal adjust = 0.5;
  QRectF rect;
  if (node()->renderOperations().isEmpty())
  {
    rect = QRectF(0,0,(m_view->defaultNewElementPixmap().size().width())*m_scaleX,(m_view->defaultNewElementPixmap().size().height())*m_scaleY);
    m_boundingRect = rect;
  }
  else
  {
    DotRenderOpVec::const_iterator it, it_end;
    it = node()->renderOperations().begin(); it_end = node()->renderOperations().end();
    for (; it != it_end; it++)
    {
      QString msg;
      QTextStream dd(&msg);
      dd << k_funcinfo << node()->id() << " an op: " << (*it).renderop << " ";
      foreach (int i, (*it).integers)
      {
        dd << i << " ";
      }
      dd << (*it).str;
      qDebug() << msg;

      if ((*it).renderop == "e" || (*it).renderop == "E")
      {
        qDebug() << k_funcinfo << "integers[0]=" << (*it).integers[0] << "; m_wdhcf=" << m_wdhcf
            << "(*it).integers[0]/*%m_wdhcf*/=" << (*it).integers[0]/*%m_wdhcf*/;
        qreal w = m_scaleX * (*it).integers[2] * 2;
        qreal h = m_scaleY * (*it).integers[3] * 2;
        qreal x = m_xMargin + (((*it).integers[0]/*%m_wdhcf*/)*m_scaleX) - w/2;
        qreal y = ((m_gh - (*it).integers[1]/*%m_hdvcf*/)*m_scaleY) + m_yMargin - h/2;
        m_boundingRect = QRectF(x - adjust,y - adjust, w + adjust, h + adjust);
        qDebug() << k_funcinfo << "'" << node()->id() << "' set rect for ellipse to " << rect;
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
        qDebug() << k_funcinfo << "'" << node()->id() << "' set rect for polygon to " << rect;
      }
    }
  }
  qDebug() << k_funcinfo << node()->id() << "new bounding rect is:" << m_boundingRect;
}

void CanvasNode::modelChanged()
{
  kDebug() << k_funcinfo << node()->id();
  m_pen = QPen(Dot2QtConsts::componentData().qtColor(m_node->fontColor()));
  m_font = FontsCache::changeable().fromName(m_node->fontName());
  computeBoundingRect();
  prepareGeometryChange();
  setPos(0,0);
}

void CanvasNode::paint(QPainter* p, const QStyleOptionGraphicsItem *option,
QWidget *widget)
{
  Q_UNUSED(option)
  Q_UNUSED(widget)
  QString msg;
  QTextStream dd(&msg);
  foreach (DotRenderOp op, node()->renderOperations())
  {
    dd << k_funcinfo << node()->id() << " an op: " << op.renderop << " ";
    foreach (int i, op.integers)
    {
      dd << i << " ";
    }
    dd << op.str << endl;
  }
  qDebug() << msg;

  if (node()->renderOperations().isEmpty())
  {
//     kDebug() << k_funcinfo << "drawPixmap";
    p->drawPixmap(QPointF(0,0),m_view->defaultNewElementPixmap());
    return;
  }
  QListIterator<DotRenderOp> it(node()->renderOperations());
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
      p->setBrush(Dot2QtConsts::componentData().qtColor(node()->backColor()));
      p->setPen(Dot2QtConsts::componentData().qtColor(node()->lineColor()));
      qDebug() << k_funcinfo << node()->id() << "drawEllipse" << node()->backColor() << rect;
      p->drawEllipse(rect);
      p->restore();
    }
    else if(dro.renderop == "p" || dro.renderop == "P")
    {
//       std::cerr << "Drawing polygon for node '"<<node()->id()<<"': ";
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
/*        kDebug() << k_funcinfo << "    point: (" << dro.integers[2*i+1] << ","
                  << dro.integers[2*i+2] << ") " << m_wdhcf << "/" << m_hdvcf;*/
        points[i] = p;
      }
      p->save();
      QPen pen(Dot2QtConsts::componentData().qtColor(node()->lineColor()));
      if (node()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      else if (node()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(node()->style()));
      }
      if (node()->style().left(12) == "setlinewidth")
      {
        bool ok;
        uint lineWidth = node()->style().mid(12, node()->style().length()-1-12).toInt(&ok);
        pen.setWidth(lineWidth);
      }
      p->setPen(pen);
      p->setBrush(Dot2QtConsts::componentData().qtColor(node()->backColor()));
/*      if (node()->style() == "filled")
      {
        p->setBrush(Dot2QtConsts::componentData().qtColor(node()->backColor()));
    //     QCanvasPolygon::paint(p);
      }
      else
      {
        p->setBrush(canvas()->backgroundColor());
      }*/
      p->drawPolygon(points);
      p->restore();
      if (!node()->shapeFile().isEmpty())
      {
        QPixmap pix(node()->shapeFile());
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
      kDebug() << k_funcinfo << "Label";
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
      QPen pen(Dot2QtConsts::componentData().qtColor(node()->lineColor()));
      if (node()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
      }
      else if (node()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(node()->style()));
      }
      p->setPen(pen);
      qDebug() << k_funcinfo << node()->id() << "drawPolyline" << points;
      p->drawPolyline(points);
      p->restore();
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
      // draw a label
/*      kDebug() << "Drawing a label " << (*it).integers[0]
        << " " << (*it).integers[1] << " " << (*it).integers[2]
        << " " << (*it).integers[3] << " " << (*it).str 
        << " (" << node()->fontName() << ", " << node()->fontSize()
        << ", " << node()->fontColor() << ")";*/
      
      int stringWidthGoal = int(dro.integers[3] * m_scaleX);
      int fontSize = node()->fontSize();
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
      qDebug() << k_funcinfo << node()->id() << "drawText" << point;
      p->drawText(point, dro.str);
      p->restore();
    }
  }
//   p->drawRect(QRectF(pos(),boundingRect().bottomRight()));
}

void CanvasNode::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED(event)
  kDebug() << k_funcinfo << m_node->id() << boundingRect();
  if (m_view->editingMode() == DotGraphView::AddNewEdge)
  {
    m_view->createNewEdgeDraftFrom(this);
  }
  else if (m_view->editingMode() == DotGraphView::DrawNewEdge)
  {
    m_view->finishNewEdgeTo(this);
  }
}

void CanvasNode::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED(event)
//   kDebug() << k_funcinfo;
}

void CanvasNode::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED(event)
//   kDebug() << k_funcinfo;
}

// CanvasHtmlNode::CanvasHtmlNode(
//                                           DotGraphView* v, 
//                                           GraphNode* n,
//                                           const DotRenderOp& dro,
//                                           const DotRenderOpVec& dros,
//                                           QGraphicsScene* c,
//                                           double scaleX, double scaleY, 
//                                           int xMargin, int yMargin, int gh,
//                                           int wdhcf, int hdvcf
//                                         )
// : KHTMLPart(v->viewport()), CanvasNode(v, n)
// {
//   m_renderOperations = dros;
// //   kDebug() << "Creating "<<node()->id()<<" CanvasHtmlNode for" << n
// //     << " with label '" << n->label() << "'";
// 
//   QString myHTMLCode = n->label();
//   myHTMLCode = myHTMLCode.mid(1, myHTMLCode.length() - 2);
// //   kDebug() << "HTML = " << myHTMLCode;
//   begin(KUrl(QString("file:") + QDir::currentPath() + "/index.html"));
//   setAutoloadImages(true);
//   write(myHTMLCode);
//   kDebug() << "HTML written.";
//   end();
//   setStatusMessagesEnabled (false);
// //   view()->setFrameShape ( QFrame::NoFrame );
// //   view()->setFrameShadow ( QFrame::Plain );
// //   view()->setLineWidth ( 0 );
// //   view()->setMidLineWidth ( 0 );
// //   view()->setHScrollBarMode ( Q3ScrollView::AlwaysOff );
// //   view()->setVScrollBarMode ( Q3ScrollView::AlwaysOff );
//   view()->setMarginWidth(0);
//   view()->setMarginHeight(0);
//   m_zoomFactor = m_view->zoom();
//   view()->part()->setZoomFactor(int(m_zoomFactor*100));
//   view()->move(int(n->x()*scaleX*m_zoomFactor), int((gh-n->y())*scaleY*m_zoomFactor));
//   view()->setMinimumSize(int(n->w()*scaleX),int(n->h()*scaleY*m_zoomFactor));
//   view()->setMaximumSize(int(n->w()*scaleX),int(n->h()*scaleY*m_zoomFactor));
//   view()->adjustSize();
//   KHTMLPart::show();
//   CanvasHtmlNode::connect(v, SIGNAL(contentsMoving ( int, int)), this, SLOT(move(int, int)));
//   CanvasHtmlNode::connect(v, SIGNAL(zoomed (double)), this, SLOT(zoomed(double)));
// }
// 
// CanvasHtmlNode::~CanvasHtmlNode() 
// {
//   KHTMLPart::hide();
// }
// 
// // void CanvasHtmlNode::paint(QPainter& p)
// // {
// //   view()->drawContents(&p);
// // }
// 
// void CanvasHtmlNode::move(int x, int y)
// {
// //   kDebug() << "CanvasHtmlNode::move("<<x<<", "<<y<<")";
//   m_xMovedTo = x; m_yMovedTo = y;
//   view()->move(int((node()->x())*m_scaleX*m_zoomFactor - m_xMovedTo), int((m_gh-node()->y())*m_scaleY*m_zoomFactor) - m_yMovedTo);
// //   view()->move(int(x*m_scaleX), int((m_gh-y)*m_scaleY));
// }
// 
// void CanvasHtmlNode::zoomed(double factor)
// {
//   m_zoomFactor = factor;
//   view()->part()->setZoomFactor(int(factor*100));
//   view()->move(int(node()->x()*m_scaleX*m_zoomFactor - m_xMovedTo), int((m_gh-node()->y())*m_scaleY*m_zoomFactor - m_yMovedTo));
//   view()->setMinimumSize(int(node()->w()*m_scaleX*m_zoomFactor),int(node()->h()*m_scaleY*m_zoomFactor));
//   view()->setMaximumSize(int(node()->w()*m_scaleX*m_zoomFactor),int(node()->h()*m_scaleY*m_zoomFactor));
//   view()->adjustSize();
// }

#include "canvasnode.moc"
