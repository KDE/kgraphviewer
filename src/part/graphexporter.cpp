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


/*
 * Callgraph View
 */

#include <stdlib.h>
#include <math.h>

#include <QFile>
#include <QTextStream>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktemporaryfile.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>

#include "dotgraphview.h"
#include "pannerview.h"
#include "canvasedge.h"

//
// GraphOptions
//

QString GraphOptions::layoutString(Layout l)
{
    if (l == Circular) return QString("Circular");
    if (l == LeftRight) return QString("LeftRight");
    return QString("TopDown");
}

GraphOptions::Layout GraphOptions::layout(QString s)
{
    if (s == QString("Circular")) return Circular;
    if (s == QString("LeftRight")) return LeftRight;
    return TopDown;
}



//
// GraphExporter
//

GraphExporter::GraphExporter(QString filename)
{
    _go = this;
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
  _graphCreated = false;
  _nodeMap.clear();
  _edgeMap.clear();

  if (_tmpFile) 
  {
    delete _tmpFile;
  }

  if (filename.isEmpty()) 
  {
    _tmpFile = new KTemporaryFile();
    _tmpFile->setSuffix(".dot");
    _dotName = _tmpFile->name();
    _useBox = true;
  }
  else 
  {
    _tmpFile = 0;
    _dotName = filename;
    _useBox = false;
  }
}



void GraphExporter::setGraphOptions(GraphOptions* go)
{
    if (go == 0) go = this;
    _go = go;
}

void GraphExporter::createGraph()
{
  //  buildGraph(f, 0, true, 1.0); // down to callings
}

void GraphExporter::writeDot()
{

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

  if (!_graphCreated) createGraph();

  /* Generate dot format...
   * When used for the DotGraphView (in contrast to "Export Callgraph..."),
   * the labels are only dummy placeholders to reserve space for our own
   * drawings.
   */

  *stream << "digraph \"callgraph\" {\n";

  if (_go->layout() == LeftRight) {
      *stream << QString("  rankdir=LR;\n");
  }
  else if (_go->layout() == Circular) {
    *stream << QString("  overlap=false;\n  splines=true;\n");
  }

  GraphNodeMap::Iterator nit;
  for ( nit = _nodeMap.begin();
        nit != _nodeMap.end(); ++nit ) 
  {
    GraphNode* n = *nit;


    *stream << n->label();
    if (_useBox) 
    {
      // make label 3 lines for DotGraphView
      *stream << QString("shape=box,label=\"%1\"];\n")
        .arg(n->label());
    }
    else
      *stream << QString("label=\"%1\"];\n")
      .arg(n->label());
  }

  GraphEdgeMap::iterator eit;
  for ( eit = _edgeMap.begin(); eit != _edgeMap.end(); ++eit ) 
  {
    GraphEdge* e = (*eit).second;

    GraphNode& from = *(e->fromNode());
    GraphNode& to   = *(e->toNode());

    e->setCallerNode(&from);
    e->setCallingNode(&to);

    *stream << QString("  F%1 -> F%2 [weight=1")
      .arg(from.label())
      .arg(to.label());

    *stream << QString(",label=\"%1\"").arg(e->label());

    *stream << QString("];\n");

  }

  // clear edges here completely.
  // Visible edges are inserted again on parsing in DotGraphView::refresh
//   for ( nit = _nodeMap.begin(); nit != _nodeMap.end(); ++nit ) 
//   {
//       GraphNode* n = *nit;
//   }

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

