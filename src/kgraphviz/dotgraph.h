/* This file is part of KGraphViewer.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/*
 * GraphViz dot Graph model
 */

#ifndef DOT_GRAPH_H
#define DOT_GRAPH_H

#include <QList>
#include <QSet>
#include <QString>
#include <QProcess>
#include <QMutex>

#include "graphelement.h"
#include "graphsubgraph.h"
#include "graphnode.h"
#include "graphedge.h"
#include "kgraphviz_export.h"
#include "support/dotdefaults.h"

class Agraph_t;

namespace KGraphViz
{

class DotGraphPrivate;

/**
  * A class representing the model of a GraphViz dot graph
  */
class KGRAPHVIZ_EXPORT DotGraph : public GraphElement
{
  Q_OBJECT

public:
  enum ParsePhase {Initial, Final};

  DotGraph();
  DotGraph(const QString& layoutCommand, const QString& fileName);

  virtual ~DotGraph();
  
  bool parseDot(const QString& fileName);

  const GraphNodeMap& nodes() const;
  const GraphEdgeMap& edges() const;
  const GraphSubgraphMap& subgraphs() const;

  /** Constant accessor to the nodes of this graph */
  GraphNodeMap& nodes();
  /** Constant accessor to the edges of this graph */
  GraphEdgeMap& edges();
  GraphSubgraphMap& subgraphs();

  double width() const;
  double height() const;
  double scale() const;
  void setWidth(double w);
  void setHeight(double h);
  void setScale(double s);

  virtual QString backColor() const;
  
  void setStrict(bool isStrict);
  void setDirected(bool isDirected);
  bool strict() const;
  bool directed() const;

  QSet< GraphNode* >& nodesOfCell(unsigned int id);
  
  unsigned int horizCellFactor() const;
  unsigned int vertCellFactor() const;
  double wdhcf() const;
  double hdvcf() const;
  
  void setLayoutCommand(const QString& layoutCommand);
  QString layoutCommand() const;

  void setDotFileName(const QString& fileName);
  QString dotFileName() const;

  bool update();

  void setReadWrite() {m_readWrite = true;}
  void setReadOnly() {m_readWrite = false;}

  virtual void storeOriginalAttributes();

  void saveTo(const QString& fileName);

  virtual void updateWithGraph(Agraph_t* newGraph);
  virtual void updateWithGraph(const DotGraph& graph);

  void setAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue);

  GraphElement* elementNamed(const QString& id) const;

  void setUseLibrary(bool value);
  bool useLibrary() const;

  void setGraphAttributes(QMap<QString,QString> attribs);
  void addNewNode(QMap<QString,QString> attribs);
  void addNewSubgraph(QMap<QString,QString> attribs);
  void addNewNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph);
  void addExistingNodeToSubgraph(QMap<QString,QString> attribs,QString subgraph);
  void moveExistingNodeToMainGraph(QMap<QString,QString> attribs);

  void addNewEdge(QString src, QString tgt, QMap<QString,QString> attribs);
  void removeAttribute(const QString& nodeName, const QString& attribName);
  void renameNode(const QString& oldNodeName, const QString& newNodeName);
  void removeNodeNamed(const QString& nodeName);
  void removeNodeFromSubgraph(const QString& nodeName, const QString& subgraphName);
  void removeSubgraphNamed(const QString& subgraphName);
  void removeEdge(const QString& id);
  void removeElement(const QString& id);

Q_SIGNALS:
  void readyToDisplay();

private:
  Q_DECLARE_PRIVATE(DotGraph);
  DotGraphPrivate* d_ptr;

  unsigned int cellNumber(int x, int y);
  void computeCells();

  bool m_readWrite;

  ParsePhase m_phase;
};

}

#endif



