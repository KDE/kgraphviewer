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

#include <QMap>

#include "graphelement.h"
#include "graphsubgraph.h"
#include "graphnode.h"
#include "graphedge.h"
#include "kgraphviz_export.h"

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
  DotGraph();
  DotGraph(const QString& layoutCommand, const QString& fileName);

  virtual ~DotGraph();
  
  bool parseDot(const QString& fileName);

  const GraphNodeMap& nodes() const;
  const GraphEdgeMap& edges() const;
  const GraphSubgraphMap& subgraphs() const;

  GraphNodeMap& nodes();
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
  void scheduleUpdate();

  virtual void storeOriginalAttributes();

  void saveTo(const QString& fileName);

  virtual void updateWithGraph(Agraph_t* newGraph);
  virtual void updateWithGraph(const DotGraph& graph);

  void setAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue);

  GraphElement* elementNamed(const QString& id) const;
  GraphNode* nodeNamed(const QString& id) const;

  void setUseLibrary(bool value);
  bool useLibrary() const;

  void setGraphAttributes(const QMap<QString,QString>& attribs); // TODO: Redundant, GraphElement already has setter

  void addNewNode(const QString& id);
  void addNewNode(const QMap<QString,QString>& attribs);
  void addNewSubgraph(const QString& id);
  void addNewSubgraph(const QMap<QString,QString>& attribs);
  void addNewNodeToSubgraph(const QString& id, const QString& subgraph);
  void addNewNodeToSubgraph(const QMap<QString,QString>& attribs, const QString& subgraph);
  void addExistingNodeToSubgraph(const QMap<QString,QString>& attribs, const QString& subgraph);
  void moveExistingNodeToMainGraph(const QMap<QString,QString>& attribs);

  void addNewEdge(const QString& sourceState, const QString& targetState, const QMap<QString,QString>& attribs = QMap<QString,QString>());
  void removeElementAttribute(const QString& nodeName, const QString& attribName);
  void renameNode(const QString& oldNodeName, const QString& newNodeName);
  void removeNodeNamed(const QString& nodeName);
  void removeNodeFromSubgraph(const QString& nodeName, const QString& subgraphName);
  void removeSubgraphNamed(const QString& subgraphName);
  void removeEdge(const QString& id);
  void removeElement(const QString& id);

Q_SIGNALS:
  void readyToDisplay();

protected:
  DotGraphPrivate* const d_ptr;

private:
  Q_DECLARE_PRIVATE(DotGraph);

  Q_PRIVATE_SLOT(d_func(), void graphIOFinished());
  Q_PRIVATE_SLOT(d_func(), void graphIOError(QString));

  Q_PRIVATE_SLOT(d_func(), void doUpdate());
};

KGRAPHVIZ_EXPORT QTextStream& operator<<(QTextStream& s, const DotGraph& n);
}

#endif
