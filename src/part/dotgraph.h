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

#include <graphviz/gvc.h>

#include "kgraphviewer_export.h"
#include "graphelement.h"
#include "graphsubgraph.h"
#include "graphnode.h"
#include "graphedge.h"
#include "dotdefaults.h"

namespace KGraphViewer
{
  
/**
  * A class representing the model of a GraphViz dot graph
  */
class DotGraph : public GraphElement
{
  Q_OBJECT
public:
  enum ParsePhase {Initial, Final};
  
  DotGraph();
  DotGraph(const QString& command, const QString& fileName);

  virtual ~DotGraph();
  
  QString chooseLayoutProgramForFile(const QString& str);
  bool parseDot(const QString& str);
  
  /** Constant accessor to the nodes of this graph */
  inline const GraphNodeMap& nodes() const {return m_nodesMap;}
  /** Constant accessor to the edges of this graph */
  inline const GraphEdgeMap& edges() const {return m_edgesMap;}
  inline const GraphSubgraphMap& subgraphs() const {return m_subgraphsMap;}
  /** Accessor to the nodes of this graph */
  inline GraphNodeMap& nodes() {return m_nodesMap;}
  /** Accessor to the edges of this graph */
  inline GraphEdgeMap& edges() {return m_edgesMap;}
  inline GraphSubgraphMap& subgraphs() {return m_subgraphsMap;}
  double width() const {return m_width;}
  double height() const {return m_height;}
  double scale() const {return m_scale;}
  void width(double w) {m_width = w;}
  void height(double h) {m_height = h;}
  void scale(double s) {m_scale = s;}
  virtual QString backColor() const;
  
  inline void strict(bool s) {m_strict = s;}
  inline void directed(bool d) {m_directed = d;}
  inline bool strict() const {return m_strict;}
  inline bool directed() const {return m_directed;}

  QSet< GraphNode* >& nodesOfCell(unsigned int id);
  
  inline unsigned int horizCellFactor() const {return m_horizCellFactor;}
  inline unsigned int vertCellFactor() const {return m_vertCellFactor;}
  inline double wdhcf() const {return m_wdhcf;}
  inline double hdvcf() const {return m_hdvcf;}
  
  inline void layoutCommand(const QString& command) {m_layoutCommand = command;}
  inline const QString& layoutCommand() {return m_layoutCommand;}
  
  inline void dotFileName(const QString& fileName) {m_dotFileName = fileName;}
  inline const QString& dotFileName() const {return m_dotFileName;}

  bool update();

  inline void setReadWrite() {m_readWrite = true;}
  inline void setReadOnly() {m_readWrite = false;}

  virtual void storeOriginalAttributes();

  void KGRAPHVIEWER_EXPORT saveTo(const QString& fileName);

  virtual void updateWithGraph(graph_t* newGraph);
  virtual void updateWithGraph(const DotGraph& graph);

  void KGRAPHVIEWER_EXPORT setAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue);

  GraphElement* elementNamed(const QString& id);

  inline void setUseLibrary(bool value) {m_useLibrary = value;}
  inline bool useLibrary() {return m_useLibrary;}

  void KGRAPHVIEWER_EXPORT setGraphAttributes(QMap<QString,QString> attribs);
  void KGRAPHVIEWER_EXPORT addNewNode(QMap<QString,QString> attribs);
  void KGRAPHVIEWER_EXPORT addNewSubgraph(QMap<QString,QString> attribs);
  void KGRAPHVIEWER_EXPORT addNewNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph);
  void KGRAPHVIEWER_EXPORT addExistingNodeToSubgraph(QMap<QString,QString> attribs,QString subgraph);
  void KGRAPHVIEWER_EXPORT moveExistingNodeToMainGraph(QMap<QString,QString> attribs);
  void KGRAPHVIEWER_EXPORT addNewEdge(QString src, QString tgt, QMap<QString,QString> attribs);
  void KGRAPHVIEWER_EXPORT removeAttribute(const QString& nodeName, const QString& attribName);
  void KGRAPHVIEWER_EXPORT renameNode(const QString& oldNodeName, const QString& newNodeName);
  void KGRAPHVIEWER_EXPORT removeNodeNamed(const QString& nodeName);
  void KGRAPHVIEWER_EXPORT removeNodeFromSubgraph(const QString& nodeName, const QString& subgraphName);
  void KGRAPHVIEWER_EXPORT removeSubgraphNamed(const QString& subgraphName);
  void KGRAPHVIEWER_EXPORT removeEdge(const QString& id);
  void KGRAPHVIEWER_EXPORT removeElement(const QString& id);

Q_SIGNALS:
  void readyToDisplay();

private Q_SLOTS:
  void slotDotRunningDone(int,QProcess::ExitStatus);
  void slotDotRunningError(QProcess::ProcessError);
  
private:
  unsigned int cellNumber(int x, int y);
  void computeCells();
  QByteArray getDotResult(int exitCode, QProcess::ExitStatus exitStatus);
    
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
  QVector< QSet< GraphNode* > > m_cells;
  
  double m_wdhcf, m_hdvcf;

  bool m_readWrite;
  QProcess* m_dot;

  ParsePhase m_phase;

  QMutex m_dotProcessMutex;

  bool m_useLibrary;
};

}

#endif



