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
 * Graph Node model
 */

#ifndef GRAPH_NODE_H
#define GRAPH_NODE_H

#include <vector>
#include <qptrlist.h>
#include <qmap.h>

#include "dotgrammar.h"

class CanvasNode;
class GraphEdge;

// sorts according start/end position of a call arc
// this depends on attached CanvasEdge's !
class GraphEdgeList: public QPtrList<GraphEdge>
{
 public:
    GraphEdgeList();
    void setSortCallerPos(bool b) { _sortCallerPos = b; }

 protected:
    int compareItems ( Item item1, Item item2 );

 private:
    bool _sortCallerPos;
};



/**
 * Colors and styles are dot names
 */
class GraphNode
{
public:
  GraphNode();
  
  virtual ~GraphNode() {}  
  
  inline void id(const QString& id) {m_id = id;}
  inline void x(double x) {m_x = x;}
  inline void y(double y) {m_y = y;}
  inline void w(double w) {m_w = w;}
  inline void h(double h) {m_h = h;}
  inline void style(const QString& ls) {m_style = ls;}
  inline void shape(const QString& lc) {m_shape= lc;}
  inline void lineColor(const QString& nt) {m_lineColor = nt;}
  inline void backColor(const QString& nc) {m_backColor = nc;}
  
  inline const QString& id() const {return m_id;}
  inline double x() const {return m_x;}
  inline double y() const {return m_y;}
  inline double w() const {return m_w;}
  inline double h() const {return m_h;}
  inline const QString& style() const {return m_style;}
  inline const QString& shape() const {return m_shape;}
  inline const QString& lineColor() const {return m_lineColor;}
  inline const QString& backColor() const {return m_backColor;}
  
  CanvasNode* canvasNode() { return m_cn[0]; }
  void setCanvasNode(CanvasNode* cn) { m_cn.push_back(cn); }

  bool isVisible() const { return m_visible; }
  void setVisible(bool v) { m_visible = v; }

  inline void label(const QString& label) {m_label = label;}
  inline const QString& label() const {return m_label;}

  inline unsigned int fontSize() const {return m_fontSize;}
  inline void fontSize(unsigned int fs) {m_fontSize = fs;}
  inline const QString& fontName() const {return m_fontName;}
  inline void fontName(const QString& fn) {m_fontName = fn;}
  inline const QString& fontColor() const {return m_fontColor;}
  inline void fontColor(const QString& fc) {m_fontColor = fc;}
  
  inline DotRenderOpVec& renderOperations() {return m_renderOperations;};
  inline const DotRenderOpVec& renderOperations() const {return m_renderOperations;};
  inline void renderOperations(DotRenderOpVec& drov) {m_renderOperations = drov;};
  
  inline unsigned int z() const {return m_z;}
  inline void z(unsigned int thez) {m_z = thez;}
  
  // keyboard navigation
  void setCalling(GraphEdge*);
  void setCaller(GraphEdge*);

  GraphEdgeList callers, callings;

  inline const QString& shapeFile() const {return m_shapeFile;}
  inline void shapeFile(const QString& sf) {m_shapeFile = sf;}
  
  inline const QString& url() const {return m_url;}
  inline void url(const QString& theUrl) {m_url = theUrl;}
  
  
private:
  std::vector< CanvasNode* > m_cn;
  bool m_visible;

  // for keyboard navigation
  int _lastCallerIndex, _lastCallingIndex;
  bool _lastFromCaller;
  
  
  QString m_label;
  
  QString m_id;
  double m_x, m_y, m_w, m_h;
  QString m_style;
  QString m_shape;
  QString m_lineColor;
  QString m_backColor;
  unsigned int m_fontSize;
  QString m_fontName;
  QString m_fontColor;
  unsigned int m_z;
  QString m_shapeFile;
  QString m_url;
  
  DotRenderOpVec m_renderOperations;
};

typedef QMap<QString, GraphNode*> GraphNodeMap;

#endif



