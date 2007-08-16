/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gaël de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/

#include <kconfig.h>

#include "graphedge.h"
#include "graphnode.h"
#include "canvasedge.h"
#include "dotdefaults.h"

#include <iostream>

/*
 * Graph Edge
 */

GraphEdge::GraphEdge() : 
    GraphElement(),
    m_colors(),
    m_dir(DOT_DEFAULT_EDGE_DIR)
{
  _fromNode = _toNode = 0;
  m_ce = 0;

  _visible = true;
  _lastFromCaller = true;
}

GraphEdge::~GraphEdge()
{
}

void GraphEdge::colors(const QString& cs)
{
  m_colors = QStringList::split(":", cs);
//   std::cerr << "edge nb colors: " << m_colors.size() << std::endl;
}

const QString GraphEdge::color(uint i) 
{
  if (i < (uint)m_colors.count())
  {
//     std::cerr << "edge color " << i << " is " << m_colors[i] << std::endl;
    return m_colors[i];
  }
  else
  {
//     std::cerr << "no edge color " << i << ". returning " << DOT_DEFAULT_EDGE_COLOR << std::endl;
    return DOT_DEFAULT_EDGE_COLOR;
  }
}

void GraphEdge::updateWith(const GraphEdge& edge)
{
  kDebug() << k_funcinfo;
  m_arrowheads = edge.arrowheads();
  m_colors = edge.colors();
  m_dir = edge.dir();
  GraphElement::updateWith(edge);
/*  if (m_ce)
  {
    m_ce->modelChanged();
  }*/
  kDebug() << k_funcinfo << "done";
}

QTextStream& operator<<(QTextStream& s, const GraphEdge& e)
{
  s << e.fromNode()->id() << " -> " << e.toNode()->id() << "  ["
    << dynamic_cast<const GraphElement&>(e) << "];" << endl;

  return s;
}
