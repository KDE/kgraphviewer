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


#include "graphexporter.h"

#include "dotgraph.h"

#include <graphviz/gvc.h>

#include <kdebug.h>

namespace KGraphViz
{

GraphExporter::GraphExporter()
{
}

GraphExporter::~GraphExporter()
{
}

graph_t* GraphExporter::exportToGraphviz(const DotGraph* graph)
{
  kDebug() << graph;

  int type = graph->directed()
      ?(graph->strict()?AGDIGRAPHSTRICT:AGDIGRAPH)
      :(graph->strict()?AGRAPHSTRICT:AGRAPH);

  graph_t* agraph = agopen((graph->id()!="\"\"")
    ?graph->id().toUtf8().data()
    :QString("unnamed").toUtf8().data(), type);

  graph->exportToGraphviz(agraph);
  /// @TODO Subgraph are not represented as needed in DotGraph, so it is not
  /// possible to save them back : to be changed !
  GraphSubgraphMap::const_iterator sit;
  for ( sit = graph->subgraphs().begin();
  sit != graph->subgraphs().end(); ++sit )
  {
    const GraphSubgraph& s = **sit;
    graph_t* subgraph = agsubg(agraph, s.id().toUtf8().data());
    s.exportToGraphviz(subgraph);
  }
  
  GraphNodeMap::const_iterator nit;
  foreach (GraphNode* n, graph->nodes())
  {
    node_t* node = agnode(agraph, n->id().toUtf8().data());
    n->exportToGraphviz(node);
  }
  
  GraphEdgeMap::const_iterator eit;
  foreach (GraphEdge* e, graph->edges())
  {
    edge_t* edge = agedge(agraph, agnode(agraph, e->fromNode()->id().toUtf8().data()),
                          agnode(agraph, e->toNode()->id().toUtf8().data()));
    e->exportToGraphviz(edge);
  }

  return agraph;
}

}
