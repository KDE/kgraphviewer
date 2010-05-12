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
 * Graph Node model
 */

#ifndef GRAPH_NODE_H
#define GRAPH_NODE_H

#include <QVector>
#include <QList>
#include <QMap>
#include <QtCore/QTextStream>

#include <graphviz/gvc.h>

#include "dotrenderop.h"
#include "dotgrammar.h"
#include "graphelement.h"
#include "canvaselement.h"
#include "canvasnode.h"

namespace KGraphViewer
{
  
class CanvasNode;

/**
 * Colors and styles are dot names
 */
class GraphNode : public GraphElement
{
//   Q_OBJECT
public:
  GraphNode();
  GraphNode(const GraphNode& gn);
  GraphNode(node_t* gn);
  
  virtual ~GraphNode() {}  
  
  inline CanvasNode* canvasNode() { return dynamic_cast<CanvasNode*>(canvasElement()); }
  inline const CanvasNode* canvasNode() const { return dynamic_cast<const CanvasNode*>(canvasElement()); }
  inline void setCanvasNode(CanvasNode* cn) { setCanvasElement((CanvasElement*)cn); }

  virtual void updateWithNode(const GraphNode& node);
  virtual void updateWithNode(node_t* node);

  
private:
};

/** A map associating the ids of a graph's nodes to these nodes */
typedef QMap<QString, GraphNode*> GraphNodeMap;

QTextStream& operator<<(QTextStream& s, const GraphNode& n);

}

#endif



