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
 * Graph Node model
 */

#ifndef GRAPH_NODE_H
#define GRAPH_NODE_H

#include <QVector>
#include <QList>
#include <QMap>
#include <QtCore/QTextStream>

#include "dotrenderop.h"
#include "dotgrammar.h"
#include "graphelement.h"

class CanvasNode;

/**
 * Colors and styles are dot names
 */
class GraphNode : public GraphElement
{
//   Q_OBJECT
public:
  GraphNode();
  
  virtual ~GraphNode() {}  
  
  inline void x(double x) {m_x = x;}
  inline void y(double y) {m_y = y;}
  inline void w(double w) {m_w = w;}
  inline void h(double h) {m_h = h;}

  inline double x() const {return m_x;}
  inline double y() const {return m_y;}
  inline double w() const {return m_w;}
  inline double h() const {return m_h;}

  CanvasNode* canvasNode();
  void setCanvasNode(CanvasNode* cn) { m_cn.push_back(cn); }

  bool isVisible() const { return m_visible; }
  void setVisible(bool v) { m_visible = v; }

private:
  QVector< CanvasNode* > m_cn;
  bool m_visible;

  double m_x, m_y, m_w, m_h;
};

typedef QMap<QString, GraphNode*> GraphNodeMap;

QTextStream& operator<<(QTextStream& s, const GraphNode& n);

#endif



