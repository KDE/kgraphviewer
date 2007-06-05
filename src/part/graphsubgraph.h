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

/*
 * Subgraph model
 */

#ifndef GRAPH_SUBGRAPH_H
#define GRAPH_SUBGRAPH_H

#include <vector>
#include <qptrlist.h>
#include <qmap.h>

#include "dotgrammar.h"

class CanvasSubgraph;

/**
 * Colors and styles are dot names
 */
class GraphSubgraph
{
public:
  GraphSubgraph();
  
  virtual ~GraphSubgraph() {}  
  
  inline void id(const QString& id) {m_id = id;}
  inline void style(const QString& ls) {m_style = ls;}
  inline void lineColor(const QString& nt) {m_lineColor = nt;}
  inline void backColor(const QString& nc) {m_backColor = nc;}
  
  inline const QString& id() const {return m_id;}
  inline const QString& style() const {return m_style;}
  inline const QString& lineColor() const {return m_lineColor;}
  inline const QString& backColor() const {return m_backColor;}
  
  CanvasSubgraph* canvasSubgraph() { return m_cs; }
  void setCanvasSubgraph(CanvasSubgraph* cs) { m_cs = cs; }

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
  
 private:
  CanvasSubgraph* m_cs;
  bool m_visible;

  QString m_label;
  
  QString m_id;
  QString m_style;
  QString m_lineColor;
  QString m_backColor;
  unsigned int m_fontSize;
  QString m_fontName;
  QString m_fontColor;
  unsigned int m_z;
  
  DotRenderOpVec m_renderOperations;
};

typedef QMap<QString, GraphSubgraph*> GraphSubgraphMap;


#endif



