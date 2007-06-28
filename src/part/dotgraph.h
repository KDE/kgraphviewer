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
 * GraphViz dot Graph model
 */

#ifndef DOT_GRAPH_H
#define DOT_GRAPH_H

#include <vector>
#include <set>

#include <qstring.h>

#include "graphelement.h"
#include "graphsubgraph.h"
#include "graphnode.h"
#include "graphedge.h"
#include "dotdefaults.h"


class DotGraph : public GraphElement
{
public:
  DotGraph(const QString& command, const QString& fileName);

  virtual ~DotGraph();
  
  QString chooseLayoutProgramForFile(const QString& str);
  bool parseDot(const QString& str);
    
  inline const GraphNodeMap& nodes() const {return m_nodesMap;}
  inline const GraphEdgeMap& edges() const {return m_edgesMap;}
  inline const GraphSubgraphMap& subgraphs() const {return m_subgraphsMap;}
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
  inline bool strict() const {return m_strict;}
  inline bool directed() const {return m_directed;}

  std::set< GraphNode* >& nodesOfCell(unsigned int id);
  
  inline unsigned int horizCellFactor() const {return m_horizCellFactor;}
  inline unsigned int vertCellFactor() const {return m_vertCellFactor;}
  inline double wdhcf() const {return m_wdhcf;}
  inline double hdvcf() const {return m_hdvcf;}
  
  inline void layoutCommand(const QString& command) {m_layoutCommand = command;}
  inline const QString& layoutCommand() {return m_layoutCommand;}
  
  inline void dotFileName(const QString& fileName) {m_dotFileName = fileName;}
  inline const QString& dotFileName() const {return m_dotFileName;}
  
private:
  unsigned int cellNumber(int x, int y);
  void computeCells();
    
  QString m_dotFileName;
  GraphSubgraphMap m_subgraphsMap;
  GraphNodeMap m_nodesMap;
  GraphEdgeMap m_edgesMap;
  double m_width, m_height;
  double m_scale;
  bool m_directed;
  bool m_strict;
  QString m_layoutCommand;
  
  unsigned int m_horizCellFactor, m_vertCellFactor;
  std::vector< std::set< GraphNode* > > m_cells;
  
  double m_wdhcf, m_hdvcf;
  
};

#endif



