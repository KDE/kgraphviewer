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
 * GraphViz dot Graph model
 */

#ifndef DOT_GRAPH_H
#define DOT_GRAPH_H

#include <vector>
#include <set>

#include <qstring.h>

#include "graphsubgraph.h"
#include "graphnode.h"
#include "graphedge.h"
#include "dotdefaults.h"


class DotGraph
{
public:
  DotGraph(const QString& command, const QString& fileName);

  virtual ~DotGraph();
  
  QString chooseLayoutProgramForFile(const QString& str);
  bool parseDot(const QString& str);
    
  inline GraphNodeMap& nodes() {return m_nodesMap;}
  inline GraphEdgeMap& edges() {return m_edgesMap;}
  inline GraphSubgraphMap& subgraphs() {return m_subgraphsMap;}
  double width() const {return m_width;}
  double height() const {return m_height;}
  double scale() const {return m_scale;}
  void width(double w) {m_width = w;}
  void height(double h) {m_height = h;}
  void scale(double s) {m_scale = s;}
  
  inline void strict(bool s) {m_strict = s;}
  inline void directed(bool d) {m_directed = d;}
  inline void id(const std::string& id) {m_id = id;}
  inline bool strict() const {return m_strict;}
  inline bool directed() const {return m_directed;}
  inline const std::string& id() const {return m_id;}

  inline void fontName(const std::string& theValue) {m_fontName = theValue;}
  inline const std::string& fontName() const {return m_fontName;}

  inline void fontSize(unsigned int theValue) {m_fontSize = theValue;}
  inline unsigned int fontSize() const {return m_fontSize;}
  
  std::set< GraphNode* >& nodesOfCell(unsigned int id);
  
  inline unsigned int horizCellFactor() const {return m_horizCellFactor;}
  inline unsigned int vertCellFactor() const {return m_vertCellFactor;}
  inline double wdhcf() const {return m_wdhcf;}
  inline double hdvcf() const {return m_hdvcf;}
  
  inline const QString& color() const {return m_color;}
  inline void color(const QString& color) {m_color = color;}
  
  inline const QString& backgroundColor() const {return m_backgroundColor;}
  inline void backgroundColor(const QString& backgroundColor) {m_backgroundColor = backgroundColor;}
  
  inline const QString& fontColor() const {return m_fontColor;}
  inline void fontColor(const QString& color) {m_fontColor = color;}
  
  inline const QString& label() const {return m_label;}
  inline void label(const QString& l) {m_label = l;}
  
  inline void layoutCommand(const QString& command) {m_layoutCommand = command;}
  inline const QString& layoutCommand() {return m_layoutCommand;}
  
  inline void dotFileName(const QString& fileName) {m_dotFileName = fileName;}
  inline const QString& dotFileName() const {return m_dotFileName;}
  
  inline DotRenderOpVec& renderOperations() {return m_renderOperations;};
  inline const DotRenderOpVec& renderOperations() const {return m_renderOperations;};
  inline void renderOperations(DotRenderOpVec& drov) {m_renderOperations = drov;};
  
private:
  unsigned int cellNumber(int x, int y);
  void computeCells();
    
  QString m_dotFileName;
  QString m_backgroundColor;
  GraphSubgraphMap m_subgraphsMap;
  GraphNodeMap m_nodesMap;
  GraphEdgeMap m_edgesMap;
  double m_width, m_height;
  double m_scale;
  bool m_directed;
  bool m_strict;
  std::string m_id;
  std::string m_fontName;
  unsigned int m_fontSize;
  QString m_color;
  QString m_fontColor;
  QString m_label;
  QString m_layoutCommand;
  
  unsigned int m_horizCellFactor, m_vertCellFactor;
  std::vector< std::set< GraphNode* > > m_cells;
  
  double m_wdhcf, m_hdvcf;
  
  DotRenderOpVec m_renderOperations;
};

#endif



