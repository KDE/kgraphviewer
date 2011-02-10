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

#ifndef KGRAPHVIZ_GRAPH_EDGE_H
#define KGRAPHVIZ_GRAPH_EDGE_H

#include "graphelement.h"

#include <QMap>

class Agedge_t;

namespace KGraphViz
{

class GraphEdgePrivate;
class GraphNode;

class KGRAPHVIZ_EXPORT GraphEdge : public GraphElement
{
public:
  GraphEdge();
  GraphEdge(const GraphEdge& edge);
  GraphEdge(Agedge_t* edge);

  virtual ~GraphEdge();

  GraphNode* fromNode() const;
  void setFromNode(GraphNode* node);

  GraphNode* toNode() const;
  void setToNode(GraphNode* node);

  QList<DotRenderOp>& arrowheads();
  const QList<DotRenderOp>& arrowheads() const;

  virtual void updateWithEdge(const GraphEdge& edge);
  virtual void updateWithEdge(Agedge_t* edge);

protected:
  GraphEdgePrivate* const d_ptr;

private:
  Q_DECLARE_PRIVATE(GraphEdge);
};

/** A map associating the bounds nodes of a graph's edges to these edges */
typedef QMap<QString, GraphEdge*> GraphEdgeMap;

KGRAPHVIZ_EXPORT QTextStream& operator<<(QTextStream& s, const GraphEdge& e);
}

#endif
