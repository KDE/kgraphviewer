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
 * Graph Edge
 */

#ifndef GRAPH_EDGE_H
#define GRAPH_EDGE_H

#include "canvasnode.h"
#include "dotgrammar.h"

#include "qstringlist.h"

class CanvasEdge;
class CanvasNode;
class GraphNode;

class GraphEdge
{
public:
  GraphEdge();
  ~GraphEdge();

  inline const QString& label() {return m_label;}
  inline void label(const QString& label) {m_label = label;}
  
  CanvasEdge* canvasEdge() { return _ce; }
  void setCanvasEdge(CanvasEdge* ce) { _ce = ce; }

  bool isVisible() { return _visible; }
  void setVisible(bool v) { _visible = v; }

  GraphNode* fromNode() { return _fromNode; }
  GraphNode* toNode() { return _toNode; }

  // has special cases for collapsed edges
  QString prettyName();

  void setCallerNode(GraphNode* n) { _fromNode = n; }
  void setCallingNode(GraphNode* n) { _toNode = n; }

  inline const QValueVector< QPair< float, float > >& edgePoints() const {return m_edgePoints;}
  inline QValueVector< QPair< float, float > >& edgePoints() {return m_edgePoints;}
  inline void edgePoints(const QValueVector< QPair< float, float > >& ep) {m_edgePoints = ep;}
  
  inline const QStringList& colors() {return m_colors;}
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

  void fontName(const std::string& theValue) {m_fontName = theValue;}
  const std::string& fontName() const {return m_fontName;}

  void fontSize(unsigned int theValue) {m_fontSize = theValue;}
  unsigned int fontSize() const {return m_fontSize;}
  
  inline const QString& fontColor() const {return m_fontColor;}
  inline void fontColor(const QString& fontColor) {m_fontColor = fontColor;}
  
  void z(unsigned int theValue) {m_z = theValue;}
  unsigned int z() const {return m_z;}
  
  inline const QString& style() const {return m_style;}
  inline void style(const QString& s) {m_style = s;}
  
  inline DotRenderOpVec& renderOperations() {return m_renderOperations;};
  inline const DotRenderOpVec& renderOperations() const {return m_renderOperations;};
  inline void renderOperations(DotRenderOpVec& drov) {m_renderOperations = drov;};
  
  
private:
  // we have a _ce *and* _from/_to because for collapsed edges,
  // only _to or _from will be unequal NULL
  GraphNode *_fromNode, *_toNode;
  CanvasEdge* _ce;
  bool _visible;
  // for keyboard navigation: have we last reached this edge via a caller?
  bool _lastFromCaller;
  QString m_label;
  QString m_type;
  QStringList m_colors;
  QString m_dir;
  QString m_style;
  QValueVector< QPair< float, float > > m_edgePoints;
  float m_labelX, m_labelY;
  std::string m_fontName;
  unsigned int m_fontSize;
  QString m_fontColor;
  unsigned int m_z;
  
  std::vector< DotRenderOp > m_arrowheads;
  
  DotRenderOpVec m_renderOperations;
  
};


typedef std::multimap<QPair<GraphNode*, GraphNode*>, GraphEdge*> GraphEdgeMap;

#endif



