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
#include "dotgraphview.h"
#include "pannerview.h"
#include "canvasnode.h"
#include "dotdefaults.h"

#include <math.h>

#include <kdebug.h>
#include <kconfig.h>

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

void GraphNode::updateWith(const GraphNode& node)
{
//   kDebug() ;
  GraphElement::updateWith(node);
  if (canvasNode())
  {
    canvasNode()->modelChanged();
  }
//   kDebug() << "done";
}

QTextStream& operator<<(QTextStream& s, const GraphNode& n)
{
  s << n.id() << "  ["
    << dynamic_cast<const GraphElement&>(n)
    <<"];"<<endl;
  return s;
}

