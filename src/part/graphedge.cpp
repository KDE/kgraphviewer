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

#include "graphedge.h"
#include "canvasedge.h"
#include "dotdefaults.h"
#include "graphnode.h"
#include "graphsubgraph.h"
#include "kgraphviewerlib_debug.h"

namespace KGraphViewer
{
/*
 * Graph Edge
 */

GraphEdge::GraphEdge()
    : GraphElement()
    , m_fromNode(nullptr)
    , m_toNode(nullptr)
    , m_visible(true)
    , m_colors()
    , m_dir(DOT_DEFAULT_EDGE_DIR)
    , m_arrowheads()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) ;
}

GraphEdge::~GraphEdge()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) ;
}

GraphEdge::GraphEdge(const GraphEdge &edge)
    : GraphElement(edge)
{
    m_fromNode = nullptr;
    m_toNode = nullptr;
    m_visible = edge.m_visible;
    m_colors = edge.m_colors;
    m_dir = edge.m_dir;
    m_arrowheads = edge.m_arrowheads;
}

void GraphEdge::colors(const QString &cs)
{
    m_colors = cs.split(':');
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << fromNode()->id() << " -> " << toNode()->id() << ": nb colors: " << m_colors.size();
}

const QString GraphEdge::color(uint i)
{
    if (i >= (uint)m_colors.count() && m_attributes.find(KEY_COLOR) != m_attributes.end()) {
        colors(m_attributes[KEY_COLOR]);
    }
    if (i < (uint)m_colors.count()) {
        //     std::cerr << "edge color " << i << " is " << m_colors[i] << std::endl;
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << fromNode()->id() << " -> " << toNode()->id() << "color" << i << "is" << m_colors[i];
        return m_colors[i];
    } else {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << fromNode()->id() << " -> " << toNode()->id() << "no edge color " << i << ". returning " << DOT_DEFAULT_EDGE_COLOR;
        return DOT_DEFAULT_EDGE_COLOR;
    }
}

void GraphEdge::updateWithEdge(const GraphEdge &edge)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << id() << edge.id();
    m_arrowheads = edge.arrowheads();
    m_colors = edge.colors();
    m_dir = edge.dir();
    GraphElement::updateWithElement(edge);
    if (canvasEdge()) {
        canvasEdge()->computeBoundingRect();
        canvasEdge()->modelChanged();
    }
}

void GraphEdge::updateWithEdge(edge_t *edge)
{
    qCDebug(KGRAPHVIEWERLIB_LOG);
    DotRenderOpVec ops;
    // decrease mem peak
    setRenderOperations(ops);

    if (agget(edge, (char *)"_draw_")) {
        parse_renderop(agget(edge, (char *)"_draw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ops.size();
    }
    if (agget(edge, (char *)"_ldraw_")) {
        parse_renderop(agget(edge, (char *)"_ldraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ops.size();
    }
    if (agget(edge, (char *)"_hdraw_")) {
        parse_renderop(agget(edge, (char *)"_hdraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ops.size();
    }
    if (agget(edge, (char *)"_tdraw_")) {
        parse_renderop(agget(edge, (char *)"_tdraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ops.size();
    }
    if (agget(edge, (char *)"_hldraw_")) {
        parse_renderop(agget(edge, (char *)"_hldraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ops.size();
    }
    if (agget(edge, (char *)"_tldraw_")) {
        parse_renderop(agget(edge, (char *)"_tldraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ops.size();
    }
    setRenderOperations(ops);
    Agsym_t *attr = agnxtattr(agraphof(agtail(edge)), AGEDGE, nullptr);
    while (attr) {
        qCDebug(KGRAPHVIEWERLIB_LOG) /*<< edge->name*/ << ":" << attr->name << agxget(edge, attr);
        m_attributes[attr->name] = agxget(edge, attr);
        attr = agnxtattr(agraphof(agtail(edge)), AGEDGE, attr);
    }
}

QTextStream &operator<<(QTextStream &s, const GraphEdge &e)
{
    QString srcLabel = e.fromNode()->id();
    if (dynamic_cast<const GraphSubgraph *>(e.fromNode())) {
        srcLabel = QString("subgraph ") + srcLabel;
    }
    QString tgtLabel = e.toNode()->id();
    if (dynamic_cast<const GraphSubgraph *>(e.toNode())) {
        tgtLabel = QString("subgraph ") + tgtLabel;
    }
    s << srcLabel << " -> " << tgtLabel << "  [" << dynamic_cast<const GraphElement &>(e) << "];" << Qt::endl;

    return s;
}

}

#include "moc_graphedge.cpp"
