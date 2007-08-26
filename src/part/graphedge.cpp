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
#include "canvasedge.h"
#include "dotdefaults.h"

#include <kconfig.h>

/*
 * Graph Edge
 */

GraphEdge::GraphEdge() : 
    GraphElement(),
    m_fromNode(0),m_toNode(0),
    m_ce(0),m_visible(true),
    m_colors(),
    m_dir(DOT_DEFAULT_EDGE_DIR),
    m_arrowheads()
{
  kDebug() ;
}

GraphEdge::~GraphEdge()
{
  kDebug() ;
}

GraphEdge::GraphEdge(const GraphEdge& edge) :
  GraphElement(edge)
{
    m_fromNode = 0;
    m_toNode = 0;
    m_ce = 0;
    m_visible = edge.m_visible;
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
    kDebug() << fromNode()->id() << " -> " << toNode()->id() << "color" << i << "is" << m_colors[i];
    return m_colors[i];
  }
  else
  {
//     kDebug() << fromNode()->id() << " -> " << toNode()->id() << "no edge color " << i << ". returning " << DOT_DEFAULT_EDGE_COLOR;
    return DOT_DEFAULT_EDGE_COLOR;
  }
}

void GraphEdge::updateWith(const GraphEdge& edge)
{
  kDebug() ;
  m_arrowheads = edge.arrowheads();
  m_colors = edge.colors();
  m_dir = edge.dir();
  GraphElement::updateWith(edge);
/*  if (m_ce)
  {
    m_ce->modelChanged();
  }*/
  kDebug() << "done";
}

QTextStream& operator<<(QTextStream& s, const GraphEdge& e)
{
  s << e.fromNode()->id() << " -> " << e.toNode()->id() << "  ["
    << dynamic_cast<const GraphElement&>(e) << "];" << endl;

  return s;
}
