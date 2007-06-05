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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/

/*
 * Graph Exporter
 */

#ifndef GRAPH_EXPORTER_H
#define GRAPH_EXPORTER_H

#include "graphnode.h"
#include "graphedge.h"

#define DEFAULT_DETAILLEVEL  1
#define DEFAULT_LAYOUT       GraphOptions::TopDown

class KTempFile;


/* Graph Options Storage */
class GraphOptions 
{
public:
  enum Layout { TopDown, LeftRight, Circular};
  
  GraphOptions() : _detailLevel(DEFAULT_DETAILLEVEL),_layout(DEFAULT_LAYOUT) {}

  // implementation of getters
  virtual int detailLevel() { return _detailLevel; }
  virtual Layout layout() { return _layout; }

  static QString layoutString(Layout);
  static Layout layout(const QString&);
protected:
    int _detailLevel;
    Layout _layout;
};

/**
 * GraphExporter
 *
 * Generates a graph file for "dot"
 * Create an instance and
 */
class GraphExporter: public GraphOptions
{
public:
  GraphExporter(const QString& filename = QString::null);
  virtual ~GraphExporter();

  void reset(const QString& filename = QString::null);

  QString filename() { return _dotName; }

  // Set the object from which to get graph options for creation.
  // Default is this object itself (supply 0 for default)
  void setGraphOptions(GraphOptions* go = 0);

  // Create a subgraph with given limits/maxDepths
  void createGraph();

  // calls createGraph before dumping of not already created
  void writeDot();

  /* After CanvasEdges are attached to GraphEdges, we can
   * sort the incoming and outgoing edges of all nodes
   * regarding start/end points for keyboard navigation
   */
  void sortEdges();

private:
  QString _dotName;
  KTempFile* _tmpFile;
  bool _graphCreated;

  GraphOptions* _go;

  // optional graph attributes
  bool _useBox;

  // graph parts written to file
  GraphNodeMap _nodeMap;
  GraphEdgeMap _edgeMap;
};


#endif



