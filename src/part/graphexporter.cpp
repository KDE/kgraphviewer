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

GraphExporter::GraphExporter(QString filename)
{
    _tmpFile = 0;
  reset(filename);
}


GraphExporter::~GraphExporter()
{
  if (_tmpFile) {
    delete _tmpFile;
  }
}


void GraphExporter::reset( QString filename)
{
  kDebug() << k_funcinfo << filename << endl;

  if (_tmpFile) 
  {
    delete _tmpFile;
  }

  if (filename.isEmpty()) 
  {
    _tmpFile = new KTemporaryFile();
    _tmpFile->setSuffix(".dot");
    _dotName = _tmpFile->name();
  }
  else 
  {
    _tmpFile = 0;
    _dotName = filename;
  }
}



void GraphExporter::writeDot(const DotGraph* graph)
{
  kDebug() << k_funcinfo << endl;
  QFile* file = 0;
  QTextStream* stream = 0;

  if (_tmpFile)
    stream = new QTextStream(_tmpFile);
  else {
    file = new QFile(_dotName);
    if ( !file->open( QIODevice::WriteOnly ) ) {
      kError() << "Can't write dot file '" << _dotName << "'" << endl;
      return;
    }
    stream = new QTextStream(file);
  }


  *stream << "digraph \""<<graph->id()<<"\" {\n";

  *stream << "graph [" << *graph <<"]" << endl;

  /// @TODO Subgraph are not represented as needed in DotGraph, so it is not
  /// possible to save them back as needed: to be changed !
  GraphSubgraphMap::const_iterator sit;
  for ( sit = graph->subgraphs().begin();
  sit != graph->subgraphs().end(); ++sit )
  {
    const GraphSubgraph& s = **sit;
    
    (*stream) << s;
  }
  
  GraphNodeMap::const_iterator nit;
  for ( nit = graph->nodes().begin();
        nit != graph->nodes().end(); ++nit )
  {
    const GraphNode& n = **nit;

    (*stream) << n;
  }

  GraphEdgeMap::const_iterator eit;
  for ( eit = graph->edges().begin();
        eit != graph->edges().end(); ++eit )
  {
    const GraphEdge& e = *((*eit).second);

    *stream << e;

  }

  *stream << "}\n";

  if (_tmpFile) 
  {
    _tmpFile->close();
  }
  else 
  {
    file->close();
    delete file;
    delete stream;
  }
}

