/*
    Copyright (C) 2010 Kevin Funk <krf@electrostorm.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef DOTGRAPH_P_H
#define DOTGRAPH_P_H

#include "graphio.h"

namespace KGraphViz
{

class DotGraphPrivate
{
public:
  explicit DotGraphPrivate(DotGraph* parent);
  virtual ~DotGraphPrivate();

  unsigned int cellNumber(int x, int y) const;
  void computeCells();

  Q_DECLARE_PUBLIC(DotGraph);
  DotGraph* q_ptr;

  GraphIO m_graphIO;

  QString m_fileName;
  QString m_layoutCommand;

  GraphSubgraphMap m_subgraphsMap;
  GraphNodeMap m_nodesMap;
  GraphEdgeMap m_edgesMap;

  double m_width, m_height;
  double m_scale;
  bool m_directed;
  bool m_strict;

  unsigned int m_horizCellFactor, m_vertCellFactor;
  double m_wdhcf, m_hdvcf;

  QVector< QSet< GraphNode* > > m_cells;

  bool m_useLibrary;

  bool m_readWrite;

private:
  void graphIOFinished();
  void graphIOError(const QString& error);
};

}

#endif
