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

// lib
#include "dot2qtconsts.h"
#include "dotgraphview.h"
#include "FontsCache.h"
#include "graphelement.h"
#include "kgraphviewerlib_debug.h"
// KF
#include <KLocalizedString>
// Qt
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QMenu>

// comment out to get extended debug output during rendering
// #define RENDER_DEBUG 1

#ifndef RENDER_DEBUG
#define RENDER_DEBUG 0
#endif

namespace KGraphViewer
{
CanvasElement::CanvasElement(DotGraphView *v, GraphElement *gelement, QGraphicsScene *c, QGraphicsItem *parent)
    : QObject()
    , QAbstractGraphicsShapeItem(parent)
    , m_scaleX(0)
    , m_scaleY(0)
    , m_xMargin(0)
    , m_yMargin(0)
    , m_gh(0)
    , m_element(gelement)
    , m_view(v)
    , m_font(nullptr)
    , m_pen(Dot2QtConsts::componentData().qtColor(gelement->fontColor()))
    , m_popup(new QMenu())
    , m_hovered(false)
    , m_lastRenderOpRev(0)
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG);
    m_font = FontsCache::changeable().fromName(gelement->fontName());

    /*  qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating CanvasElement for "<<gelement->id();
      qCDebug(KGRAPHVIEWERLIB_LOG) << "    data: " << wdhcf << "," << hdvcf << "," << gh << ","
        << scaleX << "," << scaleY << "," << xMargin << "," << yMargin << endl;*/

    if (element()->style() == QLatin1String("bold")) {
        m_pen.setStyle(Qt::SolidLine);
        m_pen.setWidth(int(2 * ((m_scaleX + m_scaleY) / 2)));
    } else if (element()->style() != QLatin1String("filled")) {
        m_pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(m_element->style()));
        m_pen.setWidth(int((m_scaleX + m_scaleY) / 2));
        if (element()->style().left(12) == QLatin1String("setlinewidth")) {
            bool ok;
            uint lineWidth = element()->style().mid(13, m_element->style().length() - 1 - 13).toInt(&ok);
            m_pen.setWidth(lineWidth * int((m_scaleX + m_scaleY) / 2));
        }
    }
    if (m_element->style() == QLatin1String("filled")) {
        m_brush = Dot2QtConsts::componentData().qtColor(element()->backColor());
        //     QCanvasPolygon::drawShape(p);
    } else {
        m_brush = c->backgroundBrush();
    }

    // the message should be given (or possible to be given) by the part user
    QAction *removeElementAction = new QAction(i18n("Remove selected element(s)"), this);
    m_popup->addAction(removeElementAction);
    connect(removeElementAction, &QAction::triggered, this, &CanvasElement::slotRemoveElement);

    connect(this, &CanvasElement::selected, v, &DotGraphView::slotElementSelected);

    connect(this, &CanvasElement::elementContextMenuEvent, v, &DotGraphView::slotContextMenuEvent);

    setAcceptHoverEvents(true);

    connect(this, &CanvasElement::hoverEnter, v, qOverload<CanvasElement *>(&DotGraphView::slotElementHoverEnter));
    connect(this, &CanvasElement::hoverLeave, v, qOverload<CanvasElement *>(&DotGraphView::slotElementHoverLeave));
}

CanvasElement::~CanvasElement()
{
    delete m_popup;
}

void CanvasElement::modelChanged()
{
    qCDebug(KGRAPHVIEWERLIB_LOG); //<< id();
    m_pen = QPen(Dot2QtConsts::componentData().qtColor(m_element->fontColor()));
    m_font = FontsCache::changeable().fromName(m_element->fontName());
    prepareGeometryChange();
    computeBoundingRect();
}

void CanvasElement::initialize(qreal scaleX, qreal scaleY, qreal xMargin, qreal yMargin, qreal gh)
{
    Q_UNUSED(gh);
    //   qCDebug(KGRAPHVIEWERLIB_LOG);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    m_scaleX = scaleX;
    m_scaleY = scaleY;
    m_xMargin = xMargin;
    m_yMargin = yMargin;
    //   m_gh = gh;

    setZValue(m_element->z());

    computeBoundingRect();
}

QRectF CanvasElement::boundingRect() const
{
    return m_boundingRect;
}

void CanvasElement::computeBoundingRect()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << element();
    qCDebug(KGRAPHVIEWERLIB_LOG) << element()->id() << zValue();

    qreal adjust = 0.5;
    QRectF rect;
    if (element()->renderOperations().isEmpty()) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "no render operation";
        rect = QRectF(0, 0, (m_view->defaultNewElementPixmap().size().width()) * m_scaleX, (m_view->defaultNewElementPixmap().size().height()) * m_scaleY);
        m_boundingRect = rect;
    } else {
        DotRenderOpVec::const_iterator it, it_end;
        it = element()->renderOperations().constBegin();
        it_end = element()->renderOperations().constEnd();
        for (; it != it_end; it++) {
#if RENDER_DEBUG
            QString msg;
            QTextStream dd(&msg);
            dd << element()->id() << " an op: " << (*it).renderop << " ";
            for (int i : (*it).integers) {
                dd << i << " ";
            }
            dd << (*it).str;
            qCDebug(KGRAPHVIEWERLIB_LOG) << msg;
#endif

            if ((*it).renderop == QLatin1String("e") || (*it).renderop == QLatin1String("E")) {
                //         qCDebug(KGRAPHVIEWERLIB_LOG) << "integers[0]=" << (*it).integers[0] << ";
                qreal w = m_scaleX * (*it).integers[2] * 2;
                qreal h = m_scaleY * (*it).integers[3] * 2;
                qreal x = m_xMargin + (((*it).integers[0]) * m_scaleX) - w / 2;
                qreal y = ((m_gh - (*it).integers[1]) * m_scaleY) + m_yMargin - h / 2;
                m_boundingRect = QRectF(x - adjust, y - adjust, w + adjust, h + adjust);
                //         qCDebug(KGRAPHVIEWERLIB_LOG) << "'" << element()->id() << "' set rect for ellipse to " << rect;
            } else if ((*it).renderop == QLatin1String("p") || (*it).renderop == QLatin1String("P")) {
                QPolygonF polygon((*it).integers[0]);
                for (int i = 0; i < (*it).integers[0]; i++) {
                    qreal x, y;
                    x = (*it).integers[2 * i + 1];
                    y = (*it).integers[2 * i + 2];
                    {
                    }
                    QPointF p(x * m_scaleX + m_xMargin, (m_gh - y) * m_scaleY + m_yMargin);
                    polygon[i] = p;
                }
                m_boundingRect = polygon.boundingRect();
                //         qCDebug(KGRAPHVIEWERLIB_LOG) << "'" << element()->id() << "' set rect for polygon to " << rect;
            }
        }
    }
    setPos(0, 0);
}

/// TODO: optimize more!
void CanvasElement::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (m_lastRenderOpRev != element()->renderOperationsRevision()) {
        m_fontSizeCache.clear();
    }

    Q_UNUSED(option)
    Q_UNUSED(widget)
    /// computes the scaling of line width
    qreal widthScaleFactor = (m_scaleX + m_scaleY) / 2;
    if (widthScaleFactor < 1) {
        widthScaleFactor = 1;
    }

#if RENDER_DEBUG
    QString msg;
    QTextStream dd(&msg);
    for (const DotRenderOp &op : element()->renderOperations()) {
        dd << element()->id() << " an op: " << op.renderop << " ";
        for (int i : op.integers) {
            dd << i << " ";
        }
        dd << op.str << Qt::endl;
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << msg;
#endif

    if (element()->renderOperations().isEmpty() && m_view->isReadWrite()) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << element()->id() << ": no render operation. This should not happen.";
        return;
    }

    QListIterator<DotRenderOp> it(element()->renderOperations());
    //   it.toBack();

    QColor lineColor = Dot2QtConsts::componentData().qtColor(element()->lineColor());
    QColor backColor = Dot2QtConsts::componentData().qtColor(element()->backColor());
    if (m_hovered && m_view->highlighting()) {
        backColor = backColor.lighter();
    }

    const QPen oldPen = p->pen();
    const QBrush oldBrush = p->brush();
    const QFont oldFont = p->font();

    while (it.hasNext()) {
        const DotRenderOp &dro = it.next();
        if (dro.renderop == QLatin1String("c")) {
            QColor c(dro.str.mid(0, 7));
            bool ok;
            c.setAlpha(255 - dro.str.mid(8).toInt(&ok, 16));
            lineColor = c;
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "c" << dro.str.mid(0,7) << lineColor;
        } else if (dro.renderop == QLatin1String("C")) {
            QColor c(dro.str.mid(0, 7));
            bool ok;
            c.setAlpha(255 - dro.str.mid(8).toInt(&ok, 16));
            if (m_hovered && m_view->highlighting()) {
                c = c.lighter();
            }
            backColor = c;
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "C" << dro.str.mid(0,7) << backColor;
        } else if (dro.renderop == QLatin1String("e") || dro.renderop == QLatin1String("E")) {
            QPen pen = oldPen;
            qreal w = m_scaleX * dro.integers[2] * 2;
            qreal h = m_scaleY * dro.integers[3] * 2;
            qreal x = m_xMargin + (dro.integers[0] * m_scaleX) - w / 2;
            qreal y = ((m_gh - dro.integers[1]) * m_scaleY) + m_yMargin - h / 2;
            QRectF rect(x, y, w, h);
            pen.setColor(lineColor);
            auto attributeIt = element()->attributes().find(QStringLiteral("penwidth"));
            if (attributeIt != element()->attributes().constEnd()) {
                bool ok;
                const int lineWidth = attributeIt->toInt(&ok);
                pen.setWidth(int(lineWidth * widthScaleFactor));
            }

            p->setBrush(backColor);
            p->setPen(pen);

            //       qCDebug(KGRAPHVIEWERLIB_LOG) << element()->id() << "drawEllipse" << lineColor << backColor << rect;
            //       rect = QRectF(0,0,100,100);
            p->drawEllipse(rect);
        } else if (dro.renderop == QLatin1String("p") || dro.renderop == QLatin1String("P")) {
            //       std::cerr << "Drawing polygon for node '"<<element()->id()<<"': ";
            QPolygonF points(dro.integers[0]);
            for (int i = 0; i < dro.integers[0]; i++) {
                qreal x, y;
                x = dro.integers[2 * i + 1];
                y = dro.integers[2 * i + 2];
                QPointF p((x * m_scaleX) + m_xMargin, ((m_gh - y) * m_scaleY) + m_yMargin);
                /*        qCDebug(KGRAPHVIEWERLIB_LOG) << "    point: (" << dro.integers[2*i+1] << ","
                                  << dro.integers[2*i+2] << ") " */
                points[i] = p;
            }

            QPen pen = oldPen;
            pen.setColor(lineColor);
            if (element()->style() == QLatin1String("bold")) {
                pen.setStyle(Qt::SolidLine);
                pen.setWidth(2);
            }
            auto attributeIt = element()->attributes().find(QStringLiteral("penwidth"));
            if (attributeIt != element()->attributes().constEnd()) {
                bool ok;
                const int lineWidth = attributeIt->toInt(&ok);
                pen.setWidth(int(lineWidth * widthScaleFactor));
            } else if (element()->style() != QLatin1String("filled")) {
                pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(element()->style()));
            }
            if (element()->style().left(12) == QLatin1String("setlinewidth")) {
                bool ok;
                uint lineWidth = element()->style().mid(12, element()->style().length() - 1 - 12).toInt(&ok);
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
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << element()->id() << "drawPolygon" << points;
            p->drawPolygon(points);
            if (!element()->shapeFile().isEmpty()) {
                QPixmap pix(element()->shapeFile());
                if (!pix.isNull()) {
                    p->drawPixmap(int(points.boundingRect().left()), int(points.boundingRect().top()), pix);
                }
            }
        }
    }

    p->setBrush(oldBrush);
    p->setPen(oldPen);

    it.toFront();
    while (it.hasNext()) {
        const DotRenderOp &dro = it.next();
        if (dro.renderop == QLatin1String("c")) {
            QColor c(dro.str.mid(0, 7));
            bool ok;
            c.setAlpha(255 - dro.str.mid(8).toInt(&ok, 16));
            lineColor = c;
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "c" << dro.str.mid(0,7) << lineColor;
        } else if (dro.renderop == QLatin1String("C")) {
            QColor c(dro.str.mid(0, 7));
            bool ok;
            c.setAlpha(255 - dro.str.mid(8).toInt(&ok, 16));
            if (m_hovered && m_view->highlighting()) {
                c = c.lighter();
            }
            backColor = c;
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "C" << dro.str.mid(0,7) << backColor;
        } else if (dro.renderop == QLatin1String("L")) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "Label";
            QPolygonF points(dro.integers[0]);
            for (int i = 0; i < dro.integers[0]; i++) {
                qreal x, y;
                x = dro.integers[2 * i + 1];
                y = dro.integers[2 * i + 2];
                QPointF p((x * m_scaleX) + m_xMargin, ((m_gh - y) * m_scaleY) + m_yMargin);
                points[i] = p;
            }
            QPen pen(lineColor);
            if (element()->style() == QLatin1String("bold")) {
                pen.setStyle(Qt::SolidLine);
                pen.setWidth(2);
            } else if (element()->style() != QLatin1String("filled")) {
                pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(element()->style()));
            }
            p->setPen(pen);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << element()->id() << "drawPolyline" << points;
            p->drawPolyline(points);
        }
    }
    p->setPen(oldPen);

    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Drawing" << element()->id() << "labels";
    QString color = lineColor.name();
    it.toFront();
    uint num_T = 0;
    while (it.hasNext()) {
        const DotRenderOp &dro = it.next();
        if (dro.renderop == QLatin1String("c") || dro.renderop == QLatin1String("C")) {
            color = dro.str.mid(0, 7);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << dro.renderop << color;
        } else if (dro.renderop == QLatin1String("F")) {
            element()->setFontName(dro.str);
            element()->setFontSize(dro.integers[0]);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "F" << element()->fontName() << element()->fontColor() << element()->fontSize();
        } else if (dro.renderop == QLatin1String("T")) {
            ++num_T;
            // we suppose here that the color has been set just before
            element()->setFontColor(color);
            // draw a label
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "Drawing a label " << dro.integers[0]
            //       << " " << dro.integers[1] << " " << dro.integers[2]
            //       << " " << dro.integers[3] << " " << dro.str
            //         << " (" << element()->fontName() << ", " << element()->fontSize()
            //         << ", " << element()->fontColor() << ")";

            int fontWidth = 0;
            bool cacheValid = false;
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << element()->id() << " initial fontSize " << fontSize;
            if (m_lastRenderOpRev == element()->renderOperationsRevision()) {
                FontSizeCache::iterator cacheIt = m_fontSizeCache.find(num_T);
                if (cacheIt != m_fontSizeCache.end()) {
                    m_font->setPointSize(cacheIt->first);
                    fontWidth = cacheIt->second;
                    cacheValid = true;
                }
            }
            if (!cacheValid) {
                int stringWidthGoal = int(dro.integers[3] * m_scaleX);
                int fontSize = element()->fontSize();
                m_font->setPointSize(fontSize);

                QFontMetrics fm(*m_font);
                fontWidth = fm.horizontalAdvance(dro.str);
                while (fontWidth > stringWidthGoal && fontSize > 1) {
                    // use floor'ed extrapolated font size
                    fontSize = double(stringWidthGoal) / fontWidth * fontSize;
                    m_font->setPointSize(fontSize);
                    fm = QFontMetrics(*m_font);
                    fontWidth = fm.horizontalAdvance(dro.str);
                }
                m_fontSizeCache[num_T] = qMakePair(fontSize, fontWidth);
            }

            p->setFont(*m_font);
            QPen pen(m_pen);
            pen.setColor(element()->fontColor());
            p->setPen(pen);
            qreal x = (m_scaleX * ((dro.integers[0]) + (((-dro.integers[2]) * (fontWidth)) / 2) - ((fontWidth) / 2))) + m_xMargin;
            qreal y = ((m_gh - (dro.integers[1])) * m_scaleY) + m_yMargin;
            QPointF point(x, y);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << element()->id() << "drawText" << point << " " << fontSize;
            p->drawText(point, dro.str);
        }
    }

    if (element()->isSelected()) {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "element is selected: draw selection marks";
        p->setBrush(Qt::black);
        p->setPen(Qt::black);
        p->drawRect(QRectF(m_boundingRect.topLeft(), QSizeF(6, 6)));
        p->drawRect(QRectF(m_boundingRect.topRight() - QPointF(6, 0), QSizeF(6, 6)));
        p->drawRect(QRectF(m_boundingRect.bottomLeft() - QPointF(0, 6), QSizeF(6, 6)));
        p->drawRect(QRectF(m_boundingRect.bottomRight() - QPointF(6, 6), QSizeF(6, 6)));
    }

    p->setPen(oldPen);
    p->setBrush(oldBrush);
    p->setFont(oldFont);

    m_lastRenderOpRev = element()->renderOperationsRevision();
}

void CanvasElement::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << m_element->id() << boundingRect();
    if (m_view->isReadOnly()) {
        return;
    }
    if (m_view->editingMode() == DotGraphView::AddNewEdge) {
        m_view->createNewEdgeDraftFrom(this);
        return;
    } else if (m_view->editingMode() == DotGraphView::DrawNewEdge) {
        m_view->finishNewEdgeTo(this);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        m_element->setSelected(!m_element->isSelected());
        if (m_element->isSelected()) {
            Q_EMIT selected(this, event->modifiers());
        }
        update();
    } else if (event->button() == Qt::RightButton) {
        // opens the selected edge contextual menu and if necessary select the edge
        if (!m_element->isSelected()) {
            m_element->setSelected(true);
            Q_EMIT selected(this, event->modifiers());
            update();
        }

        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "opens the contextual menu";
        //     m_popup->exec(event->screenPos());
        Q_EMIT elementContextMenuEvent(m_element->id(), event->screenPos());
    }
}

void CanvasElement::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    //   qCDebug(KGRAPHVIEWERLIB_LOG) ;
}

void CanvasElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    //   qCDebug(KGRAPHVIEWERLIB_LOG) ;
}

void CanvasElement::slotRemoveElement()
{
    m_view->removeSelectedElements();
}

void CanvasElement::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    //   qCDebug(KGRAPHVIEWERLIB_LOG);
    m_hovered = true;
    update();
    Q_EMIT hoverEnter(this);
}

void CanvasElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    //   qCDebug(KGRAPHVIEWERLIB_LOG);
    m_hovered = false;
    update();
    Q_EMIT hoverLeave(this);
}

}

#include "moc_canvaselement.cpp"
