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

/*
 * Subgraph model
 */

#ifndef GRAPH_SUBGRAPH_H
#define GRAPH_SUBGRAPH_H

#include <QMap>
#include <QTextStream>

#include "dotgrammar.h"
#include "graphelement.h"
#include "dotrenderop.h"

class CanvasSubgraph;

/**
 * Colors and styles are dot names
 */
class GraphSubgraph : public GraphElement
{
public:
  GraphSubgraph();
  
  virtual ~GraphSubgraph() {}  
  
  CanvasSubgraph* canvasSubgraph() { return m_cs; }
  void setCanvasSubgraph(CanvasSubgraph* cs) { m_cs = cs; }

  bool isVisible() const { return m_visible; }
  void setVisible(bool v) { m_visible = v; }

 private:
  CanvasSubgraph* m_cs;
  bool m_visible;

};

typedef QMap<QString, GraphSubgraph*> GraphSubgraphMap;

QTextStream& operator<<(QTextStream& s, const GraphSubgraph& s);

#endif



