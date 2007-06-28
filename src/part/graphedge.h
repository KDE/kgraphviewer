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
 * Graph Edge
 */

#ifndef GRAPH_EDGE_H
#define GRAPH_EDGE_H

#include "canvasnode.h"
#include "graphelement.h"
#include "dotgrammar.h"
#include "dotrenderop.h"

#include "qstringlist.h"

class CanvasEdge;
class GraphNode;

class GraphEdge : public GraphElement
{
public:
  GraphEdge();
  ~GraphEdge();

  CanvasEdge* canvasEdge() { return _ce; }
  void setCanvasEdge(CanvasEdge* ce) { _ce = ce; }

  bool isVisible() { return _visible; }
  void setVisible(bool v) { _visible = v; }

  GraphNode* fromNode() { return _fromNode; }
  GraphNode* toNode() { return _toNode; }
  const GraphNode* fromNode() const { return _fromNode; }
  const GraphNode* toNode() const { return _toNode; }

  // has special cases for collapsed edges
  QString prettyName();

  void setCallerNode(GraphNode* n) { _fromNode = n; }
  void setCallingNode(GraphNode* n) { _toNode = n; }

  inline const QVector< QPair< float, float > >& edgePoints() const {return m_edgePoints;}
  inline QVector< QPair< float, float > >& edgePoints() {return m_edgePoints;}
  inline void edgePoints(const QVector< QPair< float, float > >& ep) {m_edgePoints = ep;}
  
  inline const QStringList& colors() const {return m_colors;}
  const QString color(uint i);
  void colors(const QString& cs); 
  
  inline void labelX(float x) {m_labelX = x;}
  inline void labelY(float y) {m_labelY = y;}
  inline float labelX() const {return m_labelX;}
  inline float labelY() const {return m_labelY;}
  
  inline const QString& dir() const {return m_dir;}
  inline void dir(const QString& dir) {m_dir = dir;}

  inline std::vector< DotRenderOp >&  arrowheads() {return m_arrowheads;}
  inline const std::vector< DotRenderOp >&  arrowheads() const {return m_arrowheads;}

private:
  // we have a _ce *and* _from/_to because for collapsed edges,
  // only _to or _from will be unequal NULL
  GraphNode *_fromNode, *_toNode;
  CanvasEdge* _ce;
  bool _visible;
  // for keyboard navigation: have we last reached this edge via a caller?
  bool _lastFromCaller;
  QStringList m_colors;
  QString m_dir;
  QVector< QPair< float, float > > m_edgePoints;
  float m_labelX, m_labelY;
  
  std::vector< DotRenderOp > m_arrowheads;
};


typedef std::multimap<QPair<GraphNode*, GraphNode*>, GraphEdge*> GraphEdgeMap;

QTextStream& operator<<(QTextStream& s, const GraphEdge& e);

#endif



