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

/*
 * Graph Node
 */

#include "graphnode.h"

// lib
#include "dotgrammar.h"
#include "kgraphviewerlib_debug.h"

namespace KGraphViewer
{
//
// GraphNode
//

GraphNode::GraphNode()
    : GraphElement()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) ;
}

GraphNode::GraphNode(const GraphNode &gn)
    : GraphElement(gn)
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) ;
}

GraphNode::GraphNode(node_t *gn)
    : GraphElement()
{
    updateWithNode(gn);
}

void GraphNode::updateWithNode(const GraphNode &node)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << id() << node.id();
    GraphElement::updateWithElement(node);
    if (canvasNode()) {
        canvasNode()->computeBoundingRect();
        canvasNode()->modelChanged();
    }
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "done";
}

void GraphNode::updateWithNode(node_t *node)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << agnameof(node);
    m_attributes["id"] = agnameof(node);
    m_attributes["label"] = ND_label(node)->text;

    DotRenderOpVec ops;
    // decrease mem peak
    setRenderOperations(ops);

    if (agget(node, (char *)"_draw_")) {
        parse_renderop(agget(node, (char *)"_draw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "_draw_: element renderOperations size is now " << ops.size();
    }
    if (agget(node, (char *)"_ldraw_")) {
        parse_renderop(agget(node, (char *)"_ldraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "_ldraw_: element renderOperations size is now " << ops.size();
    }

    setRenderOperations(ops);

    Agsym_t *attr = agnxtattr(agraphof(node), AGNODE, nullptr);
    while (attr) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << agnameof(node) << ":" << attr->name << agxget(node, attr);
        m_attributes[attr->name] = agxget(node, attr);
        attr = agnxtattr(agraphof(node), AGNODE, attr);
    }
}

QTextStream &operator<<(QTextStream &s, const GraphNode &n)
{
    s << n.id() << "  [" << dynamic_cast<const GraphElement &>(n) << "];" << Qt::endl;
    return s;
}

}

#include "moc_graphnode.cpp"
