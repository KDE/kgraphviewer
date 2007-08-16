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


#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <ktemporaryfile.h>
#include <kapplication.h>

#include "graphexporter.h"
#include "dotgraph.h"

GraphExporter::GraphExporter()
{
}

GraphExporter::~GraphExporter()
{
}

QString GraphExporter::writeDot(const DotGraph* graph, const QString& fileName)
{
  kDebug() << k_funcinfo;

  QString actualFileName = fileName;
  if (fileName.isEmpty())
  {
    KTemporaryFile tempFile;
    tempFile.setSuffix(".dot");
    actualFileName = tempFile.name();
  }

  QFile f(actualFileName);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    kError() << k_funcinfo << "Unable to open for writing " << actualFileName << endl;
    exit(2);
  }
  QTextStream stream(&f);

  stream << "digraph \""<<graph->id()<<"\" {\n";

  stream << "graph [" << *graph <<"]" << endl;

  /// @TODO Subgraph are not represented as needed in DotGraph, so it is not
  /// possible to save them back : to be changed !
  kDebug() << k_funcinfo << "writing subgraphs";
  GraphSubgraphMap::const_iterator sit;
  for ( sit = graph->subgraphs().begin();
  sit != graph->subgraphs().end(); ++sit )
  {
    const GraphSubgraph& s = **sit;
    (stream) << s;
  }

  kDebug() << k_funcinfo << "writing nodes";
  GraphNodeMap::const_iterator nit;
  for ( nit = graph->nodes().begin();
        nit != graph->nodes().end(); ++nit )
  {
    (stream) << **nit;
  }

  kDebug() << k_funcinfo << "writing edges";
  GraphEdgeMap::const_iterator eit;
  for ( eit = graph->edges().begin();
        eit != graph->edges().end(); ++eit )
  {
    stream << **eit;
  }

  stream << "}\n";

  f.close();
  return actualFileName;
}

