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

#include "canvasnode.h"
#include "dotgraphview.h"
#include "graphnode.h"
#include "support/dotdefaults.h"
#include "support/dotgrammar.h"

#include <math.h>

#include <graphviz/gvc.h>

#include <kdebug.h>

namespace KGraphViz
{
  
//
// GraphNode
//

GraphNode::GraphNode() :
    GraphElement()
{
//   kDebug() ;
}

GraphNode::GraphNode(const GraphNode& gn) :
GraphElement(gn)
{
  //   kDebug() ;
}

GraphNode::GraphNode(node_t* gn) : GraphElement()
{
  kDebug();

  updateWithNode(gn);
}

GraphNode::~GraphNode()
{
}

void GraphNode::updateWithNode(const GraphNode& node)
{
  GraphElement::updateWithElement(node);
}

double GraphNode::width() const
{
  bool ok = false;
  double result = m_attributes.value("width").toDouble(&ok);
  return (ok ? result : -1.0);
}

void GraphNode::updateWithNode(node_t* node)
{
  kDebug() << node->name;
  m_attributes["id"] = node->name;
  m_attributes["label"] = ND_label(node)->text;

  QList<QString> drawingAttributes;
  drawingAttributes << "_draw_" << "_ldraw_";
  importFromGraphviz(node, drawingAttributes);
}

QTextStream& operator<<(QTextStream& s, const GraphNode& n)
{
  s << n.id() << " ["
    << dynamic_cast<const GraphElement&>(n)
    <<"];"<<endl;
  return s;
}

}

#include "graphnode.moc"
