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
#include "canvasnode.h"
#include "dotdefaults.h"

#include <iostream>

/*
 * Graph Edge
 */

GraphEdge::GraphEdge() : 
m_type(DOT_DEFAULT_EDGE_STYLE), 
m_colors(), 
m_dir(DOT_DEFAULT_EDGE_DIR),
m_z(1)
{
    _fromNode = _toNode = 0;
    _ce = 0;

    _visible = true;
    _lastFromCaller = true;
}

GraphEdge::~GraphEdge()
{
}

QString GraphEdge::prettyName()
{
  return label();
}


void GraphEdge::colors(const QString& cs)
{
  m_colors = QStringList::split(":", cs);
//   std::cerr << "edge nb colors: " << m_colors.size() << std::endl;
}

const QString GraphEdge::color(uint i) 
{
  if (i < m_colors.count())
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
