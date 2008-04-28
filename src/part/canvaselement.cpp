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
#include "dotgraphview.h"
#include "graphelement.h"
#include "dotdefaults.h"
#include "dot2qtconsts.h"
#include "FontsCache.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
//#include <ktempfile.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <KAction>

CanvasElement::CanvasElement(
                              DotGraphView* v,
                              GraphElement* gelement,
                              QGraphicsScene* c,
                              QGraphicsItem* parent
                            )
  : QObject(), QAbstractGraphicsShapeItem(parent), 
    m_element(gelement), m_view(v),
    m_font(0),
    m_pen(Dot2QtConsts::componentData().qtColor(gelement->fontColor())),
    m_popup(new QMenu())
{
//   kDebug();
  m_font = FontsCache::changeable().fromName(gelement->fontName());
/*  kDebug() << "Creating CanvasElement for "<<gelement->id();
  kDebug() << "    data: " << wdhcf << "," << hdvcf << "," << gh << "," 
    << scaleX << "," << scaleY << "," << xMargin << "," << yMargin << endl;*/
  
  if (element()->style() == "bold")
  {
    m_pen.setStyle(Qt::SolidLine);
    m_pen.setWidth(int(2*((m_scaleX+m_scaleY)/2)));
  }
  else if (element()->style() != "filled")
  {
    m_pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(m_element->style()));
    m_pen.setWidth(int((m_scaleX+m_scaleY)/2));
    if (element()->style().left(12) == "setlinewidth")
    {
      bool ok;
      uint lineWidth = element()->style().mid(13, m_element->style().length()-1-13).toInt(&ok);
      m_pen.setWidth(lineWidth * int((m_scaleX+m_scaleY)/2));
    }
  }
  if (m_element->style() == "filled")
  {
    m_brush = Dot2QtConsts::componentData().qtColor(element()->backColor());
//     QCanvasPolygon::drawShape(p);
  }
  else
  {
    m_brush = c->backgroundBrush();
  }
  
  // the message should be given (or possible to be given) by the part user
  KAction* removeElementAction = new KAction(i18n("Remove selected element(s)"), this);
  m_popup->addAction(removeElementAction);
  connect(removeElementAction,SIGNAL(triggered(bool)),this,SLOT(slotRemoveElement()));

  connect(this, SIGNAL(selected(CanvasElement*, Qt::KeyboardModifiers)), v, SLOT(slotElementSelected(CanvasElement*, Qt::KeyboardModifiers)));

}

CanvasElement::~CanvasElement()
{
  delete m_popup;
}

void CanvasElement::modelChanged()
{
  kDebug() ;//<< id();
  m_pen = QPen(Dot2QtConsts::componentData().qtColor(m_element->fontColor()));
  m_font = FontsCache::changeable().fromName(m_element->fontName());
  prepareGeometryChange();
  computeBoundingRect();
}

void CanvasElement::initialize(qreal scaleX, qreal scaleY,
                            qreal xMargin, qreal yMargin, qreal gh,
                            qreal wdhcf, qreal hdvcf)
{
//   kDebug();
  setFlag(QGraphicsItem::ItemIsMovable, true);

  m_scaleX = scaleX; m_scaleY = scaleY;
  m_xMargin = xMargin; m_yMargin = yMargin;
  m_gh = gh; m_wdhcf = wdhcf; m_hdvcf = hdvcf;

  setZValue(m_element->z());

  computeBoundingRect();
}

QRectF CanvasElement::boundingRect () const
{
  return m_boundingRect;
}

void CanvasElement::computeBoundingRect()
{
//   kDebug() << element();
//   kDebug() << element()->id() << zValue();
  
  qreal adjust = 0.5;
  QRectF rect;
  if (element()->renderOperations().isEmpty())
  {
    kDebug() << "no render operation";
    rect = QRectF(0,0,(m_view->defaultNewElementPixmap().size().width())*m_scaleX,(m_view->defaultNewElementPixmap().size().height())*m_scaleY);
    m_boundingRect = rect;
  }
  else
  {
    DotRenderOpVec::const_iterator it, it_end;
    it = element()->renderOperations().begin(); it_end = element()->renderOperations().end();
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
//         kDebug() << "integers[0]=" << (*it).integers[0] << "; m_wdhcf=" << m_wdhcf
//             << "(*it).integers[0]/*%m_wdhcf*/=" << (*it).integers[0]/*%m_wdhcf*/;
        qreal w = m_scaleX * (*it).integers[2] * 2;
        qreal h = m_scaleY * (*it).integers[3] * 2;
        qreal x = m_xMargin + (((*it).integers[0]/*%m_wdhcf*/)*m_scaleX) - w/2;
        qreal y = ((m_gh - (*it).integers[1]/*%m_hdvcf*/)*m_scaleY) + m_yMargin - h/2;
        m_boundingRect = QRectF(x - adjust,y - adjust, w + adjust, h + adjust);
//         kDebug() << "'" << element()->id() << "' set rect for ellipse to " << rect;
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
//         kDebug() << "'" << element()->id() << "' set rect for polygon to " << rect;
      }
    }
  }
  setPos(0,0);
}

void CanvasElement::paint(QPainter* p, const QStyleOptionGraphicsItem *option,
QWidget *widget)
{
  Q_UNUSED(option)
  Q_UNUSED(widget)
  QString msg;
  QTextStream dd(&msg);
  foreach (DotRenderOp op, element()->renderOperations())
  {
    dd << element()->id() << " an op: " << op.renderop << " ";
    foreach (int i, op.integers)
    {
      dd << i << " ";
    }
    dd << op.str << endl;
  }
//   kDebug() << msg;

  if (element()->renderOperations().isEmpty() && m_view->isReadWrite())
  {
//     kDebug() << "drawPixmap";
    p->drawPixmap(QPointF(0,0),m_view->defaultNewElementPixmap());
    return;
  }

  QListIterator<DotRenderOp> it(element()->renderOperations());
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
      p->setBrush(Dot2QtConsts::componentData().qtColor(element()->backColor()));
      p->setPen(Dot2QtConsts::componentData().qtColor(element()->lineColor()));
//       kDebug() << element()->id() << "drawEllipse" << element()->backColor() << rect;
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
      QPen pen(Dot2QtConsts::componentData().qtColor(element()->lineColor()));
      if (element()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(2);
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
      p->setBrush(Dot2QtConsts::componentData().qtColor(element()->backColor()));
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
      QPen pen(Dot2QtConsts::componentData().qtColor(element()->lineColor()));
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


  DotRenderOpVec::const_iterator itl, itl_end;
  itl = element()->renderOperations().begin();
  itl_end = element()->renderOperations().end();
  for (; itl != itl_end; itl++)
  {
    const DotRenderOp& dro = (*itl);
    if ( dro.renderop == "T" )
    {
      // draw a label
//       kDebug() << "Drawing a label " << (*itl).integers[0]
//         << " " << (*itl).integers[1] << " " << (*itl).integers[2]
//         << " " << (*itl).integers[3] << " " << (*itl).str
//         << " (" << element()->fontName() << ", " << element()->fontSize()
//         << ", " << element()->fontColor() << ")";

      int stringWidthGoal = int(dro.integers[3] * m_scaleX);
      int fontSize = element()->fontSize();
//       kDebug() << element()->id() << " initial fontSize " << fontSize;
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
//       kDebug() << element()->id() << "drawText" << point << " " << fontSize;
      p->drawText(point, dro.str);
      p->restore();
    }
  }
  if (element()->isSelected())
  {
//     kDebug() << "element is selected: draw selection marks";
    p->save();
    p->setBrush(Qt::black);
    p->setPen(Qt::black);
    p->drawRect(QRectF(m_boundingRect.topLeft(),QSizeF(6,6)));
    p->drawRect(QRectF(m_boundingRect.topRight()-QPointF(6,0),QSizeF(6,6)));
    p->drawRect(QRectF(m_boundingRect.bottomLeft()-QPointF(0,6),QSizeF(6,6)));
    p->drawRect(QRectF(m_boundingRect.bottomRight()-QPointF(6,6),QSizeF(6,6)));
    p->restore();
}
}

void CanvasElement::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  kDebug() << m_element->id() << boundingRect();
  if (m_view->isReadOnly())
  {
    return;
  }
  if (m_view->editingMode() == DotGraphView::AddNewEdge)
  {
    m_view->createNewEdgeDraftFrom(this);
    return;
  }
  else if (m_view->editingMode() == DotGraphView::DrawNewEdge)
  {
    m_view->finishNewEdgeTo(this);
    return;
  }
  if (event->button() == Qt::LeftButton)
  {
    m_element->setSelected(!m_element->isSelected());
    if (m_element->isSelected())
    {
      emit(selected(this,event->modifiers()));
    }
    update();
  }
  else if (event->button() == Qt::RightButton)
  {
    // opens the selected edge contextual menu and if necessary select the edge
    if (!m_element->isSelected())
    {
      m_element->setSelected(true);
      emit(selected(this,event->modifiers()));
      update();
    }
    kDebug() << "opens the contextual menu";

    m_popup->exec(event->screenPos());
  }
}




void CanvasElement::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED(event)
//   kDebug() ;
}

void CanvasElement::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  Q_UNUSED(event)
//   kDebug() ;
}

void CanvasElement::slotRemoveElement()
{
  kDebug();
  m_view->removeSelectedElements();
}

#include "canvaselement.moc"
