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

#include "pannerview.h"

// lib
#include "dotgraphview.h"
#include "kgraphviewerlib_debug.h"
// KF
#include <KLocalizedString>
// Qt
#include <QMouseEvent>

namespace KGraphViewer
{
//
// PannerView
//
PannerView::PannerView(DotGraphView *parent)
    : QGraphicsView(parent)
    , m_drawContents(true)
    , m_parent(parent)
{
    m_movingZoomRect = false;

    // why doesn't this avoid flicker ?
    // viewport()->setBackgroundMode(Qt::NoBackground);

    // if there are ever graphic glitches to be found, remove this again
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);

    setToolTip(i18n("View of the complete graph. Click and drag to move the visible part."));
    setWhatsThis(
        i18n("<h1>View of the Complete Graph</h1>"
             "<p>Single clicking somewhere without the red square will move the center of the "
             "view to where the mouse was clicked.</p><p>Clicking and dragging within the red square "
             "will cause the view to follow the movement.</p>"));
}

void PannerView::setZoomRect(QRectF r)
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "PannerView::setZoomRect " << r;
    if (r == m_zoomRect) {
        return;
    }
    scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);
    // get new zoom rect
    m_zoomRect = r;
    qreal q = mapToScene(15, 0).x();
    if (!m_zoomRect.isValid() || m_zoomRect.width() < q || m_zoomRect.height() < q) {
        double factor = ((double)m_zoomRect.width()) / m_zoomRect.height();
        qreal newWidth, newHeight;
        if (factor < 1.0) {
            newWidth = q;
            newHeight = newWidth / factor;
        } else {
            newHeight = q;
            newWidth = newHeight * factor;
        }
        qreal newXPos = m_zoomRect.x() + (m_zoomRect.width() - newWidth) / 2;
        qreal newYPos = m_zoomRect.y() + (m_zoomRect.height() - newHeight) / 2;
        m_zoomRect.setX(newXPos);
        m_zoomRect.setY(newYPos);
        m_zoomRect.setWidth(newWidth);
        m_zoomRect.setHeight(newHeight);
    }
    scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);
}

void PannerView::moveZoomRectTo(const QPointF &newPos, bool notify)
{
    if (!m_zoomRect.isValid()) {
        return;
    }

    if (m_zoomRect.center() == newPos) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "same pos, don't do anything";
        return;
    }

    scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);
    m_zoomRect.moveCenter(newPos);
    scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);

    if (m_zoomRect.isValid() && notify) {
        Q_EMIT zoomRectMovedTo(newPos);
        m_lastPos = newPos;
    }
}

void PannerView::drawForeground(QPainter *p, const QRectF &rect)
{
    if (m_zoomRect.isValid() && rect.intersects(m_zoomRect)) {
        p->save();
        if (m_zoomRect.width() > 10 && m_zoomRect.height() > 10) {
            p->setPen(Qt::red);
            // subtract pen width, i.e. draw inside
            qreal penWidth = p->pen().widthF();
            p->drawRect(m_zoomRect.adjusted(-penWidth, -penWidth, -penWidth, -penWidth));
        } else {
            QBrush brush(Qt::red);
            p->fillRect(m_zoomRect, brush);
        }
        p->restore();
    }
}

void PannerView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        return;
    }
    /*  qCDebug(KGRAPHVIEWERLIB_LOG) << "PannerView::mousePressEvent "
                  << mapToScene(e->pos()) << " / " << m_zoomRect << " / " << m_zoomRect.center() <<endl;*/
    moveZoomRectTo(mapToScene(e->pos()));
    m_movingZoomRect = true;
}

void PannerView::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_movingZoomRect) {
        return;
    }

    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "PannerView::mouseMoveEvent " << pos;
    moveZoomRectTo(mapToScene(e->pos()));
}

void PannerView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        return;
    }
    moveZoomRectTo(mapToScene(e->pos()));
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "PannerView::mouseReleaseEvent " << pos;
    m_movingZoomRect = false;
    Q_EMIT zoomRectMoveFinished();
}

void PannerView::contextMenuEvent(QContextMenuEvent *event)
{
    m_parent->contextMenuEvent(event);
}

}

#include "moc_pannerview.cpp"
