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
#include "graphnode.h"
#include "graphsubgraph.h"
#include "canvasedge.h"
#include "support/dotdefaults.h"
#include "support/dotgrammar.h"

#include <graphviz/gvc.h>

#include <KDebug>

namespace KGraphViz
{

GraphEdge::GraphEdge() : 
    m_fromNode(0),
    m_toNode(0),
    m_dir(DOT_DEFAULT_EDGE_DIR)
{
//   kDebug() ;
}

GraphEdge::~GraphEdge()
{
  kDebug();
}

GraphEdge::GraphEdge(const GraphEdge& edge) :
  GraphElement(edge)
{
    m_fromNode = 0;
    m_toNode = 0;
    m_colors = edge.m_colors;
    m_dir = edge.m_dir;
    m_arrowheads = edge.m_arrowheads;
}

void GraphEdge::colors(const QString& cs)
{
  m_colors = QStringList::split(":", cs);
//   kDebug() << fromNode()->id() << " -> " << toNode()->id() << ": nb colors: " << m_colors.size();
}

const QString GraphEdge::color(uint i) 
{
  if (i >= (uint)m_colors.count() && m_attributes.find("color") != m_attributes.end())
  {
    colors(m_attributes["color"]);
  }
  if (i < (uint)m_colors.count())
  {
//     std::cerr << "edge color " << i << " is " << m_colors[i] << std::endl;
//     kDebug() << fromNode()->id() << " -> " << toNode()->id() << "color" << i << "is" << m_colors[i];
    return m_colors[i];
  }
  else
  {
//     kDebug() << fromNode()->id() << " -> " << toNode()->id() << "no edge color " << i << ". returning " << DOT_DEFAULT_EDGE_COLOR;
    return DOT_DEFAULT_EDGE_COLOR;
  }
}

void GraphEdge::updateWithEdge(const GraphEdge& edge)
{
  m_arrowheads = edge.arrowheads();
  m_colors = edge.colors();
  m_dir = edge.dir();
  m_fromNode = edge.m_fromNode;
  m_toNode = edge.m_toNode;

  GraphElement::updateWithElement(edge);
}

void GraphEdge::updateWithEdge(edge_t* edge)
{
  QList<QString> drawingAttributes;
  drawingAttributes << "_draw_" << "_ldraw_" << "_hdraw_" << "_tdraw_" << "_hldraw_" << "_tldraw_";
  importFromGraphviz(edge, drawingAttributes);
}

QTextStream& operator<<(QTextStream& s, const GraphEdge& e)
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

}

#include "graphedge.moc"
