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

class DotGraph;
class KTemporaryFile;


/**
 * GraphExporter
 *
 * Generates a graph file for "dot"
 */
class GraphExporter
{
public:
  GraphExporter(QString filename = QString::null);
  virtual ~GraphExporter();

  void reset(QString filename = QString::null);

  QString filename() { return _dotName; }

  void writeDot(const DotGraph* graph);


private:
  QString _dotName;
  KTemporaryFile* _tmpFile;
};


#endif



