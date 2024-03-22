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

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/

#ifndef CANVAS_EDGE_H
#define CANVAS_EDGE_H

// Qt
#include <QObject>
#include <QAbstractGraphicsShapeItem>

struct DotRenderOp;

/*
 * Canvas Items:
 * - CanvasNode       (Rectangular Area)
 * - CanvasEdge       (Spline curve)
 * - CanvasEdgeLabel  (Label for edges)
 * - CanvasEdgeArrow  (Arrows at the end of the edge spline)
 */
namespace KGraphViewer
{
class DotGraphView;
class GraphEdge;

class CanvasEdge : public QObject, public QAbstractGraphicsShapeItem
{
    Q_OBJECT
public:
    explicit CanvasEdge(DotGraphView *v, GraphEdge *, qreal scaleX, qreal scaleY, qreal xMargin, qreal yMargin, qreal gh, qreal wdhcf, qreal hdvcf, QGraphicsItem *parent = nullptr);

    ~CanvasEdge() override;

    QRectF boundingRect() const override;

    QPainterPath shape() const override;

    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    inline GraphEdge *edge()
    {
        return m_edge;
    }
    inline const GraphEdge *edge() const
    {
        return m_edge;
    }

    inline void setGh(qreal gh)
    {
        m_gh = gh;
    }

    void computeBoundingRect();

Q_SIGNALS:
    void selected(CanvasEdge *, Qt::KeyboardModifiers);
    void edgeContextMenuEvent(const QString &, const QPoint &);
    void hoverEnter(CanvasEdge *);
    void hoverLeave(CanvasEdge *);

public Q_SLOTS:
    void modelChanged();
    void slotRemoveEdge();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QPainterPath pathForSpline(int splineNum, const DotRenderOp &dro) const;
    qreal distance(const QPointF &point1, const QPointF &point2);

    qreal m_scaleX, m_scaleY;
    qreal m_xMargin, m_yMargin, m_gh, m_wdhcf, m_hdvcf;
    GraphEdge *m_edge;
    QRectF m_boundingRect;
    QFont *m_font;
    DotGraphView *m_view;
    QMenu *m_popup;
    mutable QPainterPath m_shape;
};

}

#endif
