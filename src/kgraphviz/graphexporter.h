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

#ifndef KGRAPHVIZ_GRAPH_EXPORTER_H
#define KGRAPHVIZ_GRAPH_EXPORTER_H

class Agraph_t;

namespace KGraphViz
{

class DotGraph;

/**
 * GraphExporter
 *
 * Generates a graph file for "dot"
 */
class GraphExporter
{
public:
  static Agraph_t* exportToGraphviz(const DotGraph* graph);

private:
  GraphExporter();
  virtual ~GraphExporter();
};

}

#endif
