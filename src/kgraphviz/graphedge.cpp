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
#include "graphedge_p.h"

#include "graphnode.h"
#include "graphsubgraph.h"
#include "canvasedge.h"
#include "support/dotdefaults.h"
#include "support/dotgrammar.h"

#include <graphviz/gvc.h>

#include <KDebug>

using namespace KGraphViz;

GraphEdgePrivate::GraphEdgePrivate()
  : m_fromNode(0)
  , m_toNode(0)
{
}

GraphEdgePrivate::GraphEdgePrivate(const GraphEdgePrivate& other)
  : m_fromNode(other.m_fromNode)
  , m_toNode(other.m_toNode)
  , m_arrowHeads(other.m_arrowHeads)
{
}

GraphEdgePrivate::~GraphEdgePrivate()
{
  kDebug();
}

GraphEdge::GraphEdge()
  : d_ptr(new GraphEdgePrivate)
{
  kDebug();
}

GraphEdge::~GraphEdge()
{
  kDebug();

  delete d_ptr;
}

GraphEdge::GraphEdge(const GraphEdge& other)
  : GraphElement(other)
  , d_ptr(new GraphEdgePrivate(*other.d_ptr))
{
}

GraphEdge::GraphEdge(Agedge_t* edge)
  : d_ptr(new GraphEdgePrivate)
{
  kDebug();

  updateWithEdge(edge);
}

QList< DotRenderOp >& GraphEdge::arrowheads()
{
  Q_D(GraphEdge);
  return d->m_arrowHeads;
}

const QList< DotRenderOp >& GraphEdge::arrowheads() const
{
  Q_D(const GraphEdge);
  return d->m_arrowHeads;
}

const QStringList& GraphEdge::colors() const
{
  Q_D(const GraphEdge);
  return d->m_colors;
}

void GraphEdge::colors(const QString& cs)
{
  Q_D(GraphEdge);
  d->m_colors = QStringList::split(":", cs);
//   kDebug() << fromNode()->id() << " -> " << toNode()->id() << ": nb colors: " << d->m_colors.size();
}

GraphNode* GraphEdge::fromNode() const
{
  Q_D(const GraphEdge);
  return d->m_fromNode;
}

void GraphEdge::setFromNode(GraphNode* node)
{
  Q_D(GraphEdge);
  d->m_fromNode = node;
}

void GraphEdge::setToNode(GraphNode* node)
{
  Q_D(GraphEdge);
  d->m_toNode = node;
}

GraphNode* GraphEdge::toNode() const
{
  Q_D(const GraphEdge);
  return d->m_toNode;
}

const QString GraphEdge::color(uint i) 
{
  Q_D(const GraphEdge);
  if (i >= (uint)d->m_colors.count() && m_attributes.find("color") != m_attributes.end())
  {
    colors(m_attributes["color"]);
  }
  if (i < (uint)d->m_colors.count())
  {
//     std::cerr << "edge color " << i << " is " << d->m_colors[i] << std::endl;
//     kDebug() << fromNode()->id() << " -> " << toNode()->id() << "color" << i << "is" << d->m_colors[i];
    return d->m_colors[i];
  }
  else
  {
//     kDebug() << fromNode()->id() << " -> " << toNode()->id() << "no edge color " << i << ". returning " << DOT_DEFAULT_EDGE_COLOR;
    return DOT_DEFAULT_EDGE_COLOR;
  }
}

void GraphEdge::updateWithEdge(const GraphEdge& edge)
{
  Q_D(GraphEdge);
  d->m_arrowHeads = edge.arrowheads();
  d->m_colors = edge.colors();
  d->m_fromNode = edge.fromNode();
  d->m_toNode = edge.toNode();

  GraphElement::updateWithElement(edge);
}

void GraphEdge::updateWithEdge(edge_t* edge)
{
  QList<QString> drawingAttributes;
  drawingAttributes << "_draw_" << "_ldraw_" << "_hdraw_" << "_tdraw_" << "_hldraw_" << "_tldraw_";
  importFromGraphviz(edge, drawingAttributes);
}

QTextStream& KGraphViz::operator<<(QTextStream& s, const GraphEdge& e)
{
  QString srcLabel = e.fromNode()->id();
  if (dynamic_cast<const GraphSubgraph*>(e.fromNode()))
  {
    srcLabel = QString("subgraph ") + srcLabel;
  }
  QString tgtLabel = e.toNode()->id();
  if (dynamic_cast<const GraphSubgraph*>(e.toNode()))
  {
    tgtLabel = QString("subgraph ") + tgtLabel;
  }
  s << srcLabel << " -> " << tgtLabel << "  ["
    << dynamic_cast<const GraphElement&>(e) << "];" << endl;

  return s;
}

#include "graphedge.moc"
