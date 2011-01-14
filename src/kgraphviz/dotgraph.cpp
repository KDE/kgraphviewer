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

#include "dotgraph.h"
#include "dotgraph_p.h"

#include "graphexporter.h"
#include "canvasedge.h"
#include "canvasnode.h"
#include "canvassubgraph.h"
#include "support/dotdefaults.h"
#include "support/dotgrammar.h"
#include "support/dotgraphparsinghelper.h"

#include <math.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_distinct.hpp>

#include <graphviz/gvc.h>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include <QFile>
#include <QPair>
#include <QByteArray>
#include <QProcess>
#include <QMutexLocker>
#include <QUuid>

#define AGINIT() aginitlib(sizeof(Agraph_t),sizeof(Agnode_t),sizeof(Agedge_t))

using namespace boost;
using namespace boost::spirit::classic;

extern KGraphViz::DotGraphParsingHelper* phelper;

using namespace KGraphViz;

const distinct_parser<> keyword_p("0-9a-zA-Z_");

DotGraphPrivate::DotGraphPrivate(DotGraph* parent) :
  q_ptr(parent),
  m_width(0.0), m_height(0.0),m_scale(1.0),
  m_directed(true), m_strict(false),
  m_horizCellFactor(0), m_vertCellFactor(0),
  m_wdhcf(0), m_hdvcf(0),
  m_useLibrary(false),
  m_readWrite(false)
{
  AGINIT();

  Q_Q(DotGraph);
  q->connect(&m_graphIO, SIGNAL(finished()), SLOT(graphIOFinished()));
  q->connect(&m_graphIO, SIGNAL(error(QString)), SLOT(graphIOError(QString)));
}

DotGraphPrivate::~DotGraphPrivate()
{
}

void DotGraphPrivate::graphIOFinished()
{
  kDebug();

  Q_Q(DotGraph);
  DotGraph* updatedGraph = m_graphIO.readData();
  q->updateWithGraph(*updatedGraph);
}

void DotGraphPrivate::graphIOError(const QString& error)
{
  // TODO: add error handling here
  kDebug() << "Error message from graph IO:" << error;
}

DotGraph::DotGraph() :
  d_ptr(new DotGraphPrivate(this))
{
  setId("unnamed");
}

DotGraph::DotGraph(const QString& layoutCommand, const QString& fileName) :
  d_ptr(new DotGraphPrivate(this))
{
  setId("unnamed");

  Q_D(DotGraph);
  d->m_fileName = fileName;
  d->m_layoutCommand = layoutCommand;
}

DotGraph::~DotGraph()
{
  kDebug();

  Q_D(DotGraph);
  GraphNodeMap::iterator itn, itn_end;
  itn = d->m_nodesMap.begin(); itn_end = d->m_nodesMap.end();
  for (; itn != itn_end; itn++)
  {
    delete *itn;
  }

  GraphEdgeMap::iterator ite, ite_end;
  ite = d->m_edgesMap.begin(); ite_end = d->m_edgesMap.end();
  for (; ite != ite_end; ite++)
  {
    delete (*ite);
  }

  delete d_ptr;
}

QString DotGraph::dotFileName() const
{
  Q_D(const DotGraph);
  return d->m_fileName;
}

void DotGraph::setDotFileName(const QString& fileName)
{
  Q_D(DotGraph);
  d->m_fileName = fileName;
}

QString DotGraph::layoutCommand() const
{
  Q_D(const DotGraph);
  return d->m_layoutCommand;
}

void DotGraph::setLayoutCommand(const QString& layoutCommand)
{
  Q_D(DotGraph);
  d->m_layoutCommand = layoutCommand;
}

const KGraphViz::GraphNodeMap& DotGraph::nodes() const
{
  Q_D(const DotGraph);
  return d->m_nodesMap;
}

const KGraphViz::GraphEdgeMap& DotGraph::edges() const
{
  Q_D(const DotGraph);
  return d->m_edgesMap;
}

const KGraphViz::GraphSubgraphMap& DotGraph::subgraphs() const
{
  Q_D(const DotGraph);
  return d->m_subgraphsMap;
}

KGraphViz::GraphNodeMap& DotGraph::nodes()
{
  Q_D(DotGraph);
  return d->m_nodesMap;
}

KGraphViz::GraphEdgeMap& DotGraph::edges()
{
  Q_D(DotGraph);
  return d->m_edgesMap;
}

KGraphViz::GraphSubgraphMap& DotGraph::subgraphs()
{
  Q_D(DotGraph);
  return d->m_subgraphsMap;
}

double DotGraph::width() const
{
  Q_D(const DotGraph);
  return d->m_width;
}
void DotGraph::setWidth(double width)
{
  Q_D(DotGraph);
  d->m_width = width;
}

double DotGraph::height() const
{
  Q_D(const DotGraph);
  return d->m_height;
}
void DotGraph::setHeight(double height)
{
  Q_D(DotGraph);
  d->m_height = height;
}

double DotGraph::scale() const
{
  Q_D(const DotGraph);
  return d->m_scale;
}
void DotGraph::setScale(double scale)
{
  Q_D(DotGraph);
  d->m_scale = scale;
}

bool DotGraph::strict() const
{
  Q_D(const DotGraph);
  return d->m_strict;
}
void DotGraph::setStrict(bool isStrict)
{
  Q_D(DotGraph);
  d->m_strict = isStrict;
}

bool DotGraph::directed() const
{
  Q_D(const DotGraph);
  return d->m_directed;
}
void DotGraph::setDirected(bool isDirected)
{
  Q_D(DotGraph);
  d->m_directed = isDirected;
}

double DotGraph::hdvcf() const
{
  Q_D(const DotGraph);
  return d->m_hdvcf;
}

double DotGraph::wdhcf() const
{
  Q_D(const DotGraph);
  return d->m_wdhcf;
}

unsigned int DotGraph::horizCellFactor() const
{
  Q_D(const DotGraph);
  return d->m_horizCellFactor;
}

unsigned int DotGraph::vertCellFactor() const
{
  Q_D(const DotGraph);
  return d->m_vertCellFactor;
}

bool DotGraph::useLibrary() const
{
  Q_D(const DotGraph);
  return d->m_useLibrary;
}
void DotGraph::setUseLibrary(bool value)
{
  Q_D(DotGraph);
  d->m_useLibrary = value;
}

bool DotGraph::parseDot(const QString& fileName)
{
  kDebug() << fileName;

  Q_D(DotGraph);
  d->m_graphIO.loadFromDotFile(fileName, layoutCommand());

  return true;
}

bool DotGraph::update()
{
  Q_D(DotGraph);
  if (!useLibrary())
  {
    kDebug() << "using command line";
    d->m_graphIO.updateDot(this);
    return true;
  }
  else
  {
    kDebug() << "using library, layout command:" << layoutCommand();
    graph_t* graph = GraphExporter::exportToGraphviz(this);

    GVC_t* gvc = gvContext();
    gvLayout(gvc, graph, layoutCommand().toUtf8().data());
    gvRender (gvc, graph, "xdot", NULL);

    updateWithGraph(graph);

    gvFreeLayout(gvc, graph);
    agclose(graph);
    bool result = (gvFreeContext(gvc) == 0);
    return result;
  }
}

unsigned int DotGraphPrivate::cellNumber(int x, int y) const
{
/*  kDebug() << "x= " << x << ", y= " << y << ", m_width= " << m_width << ", m_height= " << m_height << ", m_horizCellFactor= " << m_horizCellFactor << ", m_vertCellFactor= " << m_vertCellFactor  << ", m_wdhcf= " << m_wdhcf << ", m_hdvcf= " << m_hdvcf;*/

  const unsigned int nx = (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf);
  const unsigned int ny = (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf);
/*  kDebug() << "nx = " << (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf);
  kDebug() << "ny = " << (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf);
  kDebug() << "res = " << ny * m_horizCellFactor + nx;*/

  const unsigned int res = ny * m_horizCellFactor + nx;
  return res;
}

#define MAXCELLWEIGHT 800

void DotGraphPrivate::computeCells()
{
  kWarning() << "Not implemented";
  return;

/* FIXME: Is this used?
  kDebug() << m_width << m_height << endl;
  m_horizCellFactor = m_vertCellFactor = 1;
  m_wdhcf = (int)ceil(((double)m_width) / m_horizCellFactor)+1;
  m_hdvcf = (int)ceil(((double)m_height) / m_vertCellFactor)+1;
  bool stop = true;
  do
  {
    stop = true;
    m_cells.clear();
//     m_cells.resize(m_horizCellFactor * m_vertCellFactor);

    GraphNodeMap::iterator it, it_end;
    it = m_nodesMap.begin(); it_end = m_nodesMap.end();
    for (; it != it_end; it++)
    {
      GraphNode* gn = *it;
//       int cellNum = cellNumber(int(gn->x()), int(gn->y()));
      int cellNum = cellNumber(0,0);
      kDebug() << "Found cell number " << cellNum;

      if (m_cells.size() <= cellNum)
      {
        m_cells.resize(cellNum+1);
      }
      m_cells[cellNum].insert(gn);

      kDebug() << "after insert";
      if ( m_cells[cellNum].size() > MAXCELLWEIGHT )
      {
        kDebug() << "cell number " << cellNum  << " contains " << m_cells[cellNum].size() << " nodes";
        if ((m_width/m_horizCellFactor) > (m_height/m_vertCellFactor))
        {
          m_horizCellFactor++;
          m_wdhcf = m_width / m_horizCellFactor;
        }
        else
        {
          m_vertCellFactor++;
          m_hdvcf = m_height / m_vertCellFactor;
        }
        kDebug() << "cell factor is now " << m_horizCellFactor << " / " << m_vertCellFactor;
        stop = false;
        break;
      }
    }
  } while (!stop);
  kDebug() << "m_wdhcf=" << m_wdhcf << "; m_hdvcf=" << m_hdvcf << endl;
  kDebug() << "finished" << endl;
*/
}

QSet< GraphNode* >& DotGraph::nodesOfCell(unsigned int id)
{
  Q_D(DotGraph);
  return d->m_cells[id];
}

void DotGraph::storeOriginalAttributes()
{
  foreach (GraphNode* node, nodes())
  {
    node->storeOriginalAttributes();
  }
  foreach (GraphEdge* edge, edges())
  {
    edge->storeOriginalAttributes();
  }
  foreach (GraphSubgraph* subgraph, subgraphs())
  {
    subgraph->storeOriginalAttributes();
  }
  GraphElement::storeOriginalAttributes();
}

void DotGraph::saveTo(const QString& fileName)
{
  kDebug() << "Filename:" << fileName;

  Q_D(DotGraph);
  d->m_graphIO.saveToDotFile(this, fileName);
}

void DotGraph::updateWithGraph(graph_t* newGraph)
{
  Q_D(DotGraph);

  kDebug() << newGraph;
  QList<QString> drawingAttributes;
  drawingAttributes << "_draw_" << "_ldraw_";
  importFromGraphviz(newGraph, drawingAttributes);

  // copy subgraphs
  for (edge_t* e = agfstout(newGraph->meta_node->graph, newGraph->meta_node); e;
      e = agnxtout(newGraph->meta_node->graph, e))
  {
    graph_t* sg = agusergraph(e->head);
    kDebug() << "subgraph:" << sg->name;
    if (subgraphs().contains(sg->name))
    {
      kDebug() << "known";
      // ???
      //       nodes()[ngn->name]->setZ(ngn->z());
      subgraphs()[sg->name]->updateWithSubgraph(sg);
      if (subgraphs()[sg->name]->canvasElement()!=0)
      {
        //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      kDebug() << "new";
      GraphSubgraph* newsg = new GraphSubgraph(sg);
      //       kDebug() << "new created";
      subgraphs().insert(sg->name, newsg);
      //       kDebug() << "new inserted";
    }

  }

  // copy nodes
  node_t* ngn = agfstnode(newGraph);
  while (ngn != NULL)
  {
    kDebug() << "node " << ngn->name;
    if (nodes().contains(ngn->name))
    {
      kDebug() << "known";
// ???
//       nodes()[ngn->name]->setZ(ngn->z());
      nodes()[ngn->name]->updateWithNode(ngn);
      if (nodes()[ngn->name]->canvasElement()!=0)
      {
        //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      kDebug() << "new";
      GraphNode* newgn = new GraphNode(ngn);
      //       kDebug() << "new created";
      nodes().insert(ngn->name, newgn);
      //       kDebug() << "new inserted";
    }

    // copy node edges
    edge_t* nge = agfstout(newGraph, ngn);
    while (nge != NULL)
    {
      kDebug() << "edge" << nge->id;
      const QString edgeName = QString::number(nge->id);
      if (edges().contains(edgeName))
      {
        kDebug() << "edge known" << nge->id;
//         edges()[nge->name]->setZ(nge->z());
        edges()[edgeName]->updateWithEdge(nge);
        if (edges()[edgeName]->canvasElement()!=0)
        {
          //         edges()[nge->id()]->canvasElement()->setGh(m_height);
        }
      }
      else
      {
        kDebug() << "new edge" << edgeName;
        {
          GraphEdge* newEdge = new GraphEdge();
          newEdge->setId(edgeName);
          newEdge->updateWithEdge(nge);
          if (elementNamed(nge->tail->name) == 0)
          {
            GraphNode* newgn = new GraphNode();
            //       kDebug() << "new created";
            nodes().insert(nge->tail->name, newgn);
          }
          newEdge->setFromNode(elementNamed(nge->tail->name));
          if (elementNamed(nge->head->name) == 0)
          {
            GraphNode* newgn = new GraphNode();
            //       kDebug() << "new created";
            nodes().insert(nge->head->name, newgn);
          }
          newEdge->setToNode(elementNamed(nge->head->name));
          edges().insert(edgeName, newEdge);
        }
      }
      nge = agnxtedge(newGraph, nge, ngn);
    }
    ngn = agnxtnode(newGraph, ngn);
  }
  kDebug() << "Done";
  emit readyToDisplay();
  d->computeCells();
}

void DotGraph::updateWithGraph(const DotGraph& newGraph)
{
  Q_D(DotGraph);
  kDebug();
  GraphElement::updateWithElement(newGraph);
  d->m_width = newGraph.width();
  d->m_height = newGraph.height();
  d->m_scale = newGraph.scale();
  d->m_directed = newGraph.directed();
  d->m_strict = newGraph.strict();

  d->computeCells();
  foreach (GraphSubgraph* nsg, newGraph.subgraphs())
  {
    kDebug() << "subgraph" << nsg->id();
    if (subgraphs().contains(nsg->id()))
    {
      kDebug() << "subgraph known" << nsg->id();
      subgraphs().value(nsg->id())->updateWithSubgraph(*nsg);
      if (subgraphs().value(nsg->id())->canvasElement()!=0)
      {
//         subgraphs().value(nsg->id())->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      kDebug() << "new subgraph" << nsg->id();
      GraphSubgraph* newSubgraph = new GraphSubgraph();
      newSubgraph->updateWithSubgraph(*nsg);
      newSubgraph->setZ(0);
      subgraphs().insert(nsg->id(), newSubgraph);
    }
  }
  foreach (GraphNode* ngn, newGraph.nodes())
  {
    kDebug() << "node" << ngn->id();
    if (nodes().contains(ngn->id()))
    {
      kDebug() << "known";
      nodes()[ngn->id()]->setZ(ngn->z());
      nodes()[ngn->id()]->updateWithNode(*ngn);
      if (nodes()[ngn->id()]->canvasElement()!=0)
      {
//         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      kDebug() << "new";
      GraphNode* newgn = new GraphNode(*ngn);
//       kDebug() << "new created";
      nodes().insert(ngn->id(), newgn);
//       kDebug() << "new inserted";
    }
  }
  foreach (GraphEdge* nge, newGraph.edges())
  {
    kDebug() << "edge " << nge->id();
    if (edges().contains(nge->id()))
    {
      kDebug() << "edge known" << nge->id();
      edges()[nge->id()]->setZ(nge->z());
      edges()[nge->id()]->updateWithEdge(*nge);
      if (edges()[nge->id()]->canvasElement()!=0)
      {
//         edges()[nge->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      kDebug() << "new edge" << nge->id();
      {
        GraphEdge* newEdge = new GraphEdge();
        newEdge->setId(nge->id());
        newEdge->updateWithEdge(*nge);
        newEdge->setFromNode(elementNamed(nge->fromNode()->id()));
        newEdge->setToNode(elementNamed(nge->toNode()->id()));
        edges().insert(nge->id(), newEdge);
      }
    }
  }
  kDebug() << "Done";
  emit readyToDisplay();
  d->computeCells();
}

void DotGraph::removeNodeNamed(const QString& nodeName)
{
  Q_D(DotGraph);
  kDebug() << nodeName;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(nodeName));
  if (node == 0)
  {
    kError() << "No such node " << nodeName;
    return;
  }

  GraphEdgeMap::iterator it, it_end;
  it = d->m_edgesMap.begin(); it_end = d->m_edgesMap.end();
  while (it != it_end)
  {
    if ( it.value()->fromNode() == node
        || it.value()->toNode() == node )
    {
      GraphEdge* edge = it.value();
      if (edge->canvasElement() != 0)
      {
        edge->canvasElement()->hide();
        delete edge->canvasElement();
        delete edge;
      }
      it = edges().erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (node->canvasElement() != 0)
  {
    node->canvasElement()->hide();
    delete node->canvasElement();
    node->setCanvasElement(0);
  }
  nodes().remove(nodeName);
  delete node;

}

void DotGraph::removeNodeFromSubgraph(
    const QString& nodeName,
    const QString& subgraphName)
{
  kDebug() << nodeName << subgraphName;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(nodeName));
  if (node == 0)
  {
    kError() << "No such node " << nodeName;
    return;
  }

  GraphSubgraph* subgraph = subgraphs()[subgraphName];
  if (subgraph == 0)
  {
    kError() << "No such subgraph " << subgraphName;
    return;
  }

  subgraph->removeElement(node);
  if (subgraph->content().isEmpty())
  {
    removeSubgraphNamed(subgraphName);
  }
}

void DotGraph::removeSubgraphNamed(const QString& subgraphName)
{
  Q_D(DotGraph);
  kDebug() << subgraphName << " from " << subgraphs().keys();
  GraphSubgraph* subgraph = subgraphs()[subgraphName];

  if (subgraph == 0)
  {
    kError() << "Subgraph" << subgraphName << "not found";
    return;
  }
  GraphEdgeMap::iterator it, it_end;
  it = d->m_edgesMap.begin(); it_end = d->m_edgesMap.end();
  while (it != it_end)
  {
    if ( it.value()->fromNode() == subgraph
        || it.value()->toNode() == subgraph )
    {
      GraphEdge* edge = it.value();
      if (edge->canvasElement() != 0)
      {
        edge->canvasElement()->hide();
        delete edge->canvasElement();
        delete edge;
      }
      it = edges().erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (subgraph->canvasElement() != 0)
  {
    subgraph->canvasElement()->hide();
    delete subgraph->canvasElement();
    subgraph->setCanvasElement(0);
  }
  foreach(GraphElement* element, subgraph->content())
  {
    if (dynamic_cast<GraphNode*>(element) != 0)
    {
      kDebug() << "Adding" << element->id() << "to main graph";
      nodes()[element->id()] = dynamic_cast<GraphNode*>(element);
    }
    else if (dynamic_cast<GraphSubgraph*>(element) != 0)
    {
      subgraphs()[element->id()] = dynamic_cast<GraphSubgraph*>(element);
    }
    else
    {
      kError() << "Don't know how to handle" << element->id();
    }
  }
  subgraph->content().clear();
  subgraphs().remove(subgraphName);
  delete subgraph;
}

void DotGraph::removeEdge(const QString& id)
{
  kDebug() << id;
  GraphEdgeMap::iterator it = edges().begin();
  for (; it != edges().end(); it++)
  {
    GraphEdge* edge = it.value();
    if (edge->id() ==id)
    {
      if (edge->canvasElement() != 0)
      {
        edge->canvasElement()->hide();
        delete edge->canvasElement();
        delete edge;
      }
      edges().remove(id);
      break;
    }
  }
}

void DotGraph::removeElement(const QString& id)
{
  kDebug() << id;
  GraphNodeMap::const_iterator itN = nodes().constBegin();
  for (; itN != nodes().constEnd(); itN++)
  {
    const GraphNode* node = itN.value();
    if (node->id() == id)
    {
      removeNodeNamed(id);
      return;
    }
  }
  GraphEdgeMap::const_iterator itE = edges().constBegin();
  for (; itE != edges().constEnd(); itE++)
  {
    const GraphEdge* edge = itE.value();
    if (edge->id() == id)
    {
      removeEdge(id);
      return;
    }
  }
  GraphSubgraphMap::const_iterator itS = subgraphs().constBegin();
  for (; itS != subgraphs().constEnd(); itS++)
  {
    const GraphSubgraph* subgraph = itS.value();
    if (subgraph->id() == id)
    {
      removeSubgraphNamed(id);
      return;
    }
  }
}

void DotGraph::setAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue)
{
  if (nodes().contains(elementId))
  {
    nodes()[elementId]->attributes()[attributeName] = attributeValue;
  }
  else if (edges().contains(elementId))
  {
    edges()[elementId]->attributes()[attributeName] = attributeValue;
  }
  else if (subgraphs().contains(elementId))
  {
    subgraphs()[elementId]->attributes()[attributeName] = attributeValue;
  }
}

GraphElement* DotGraph::elementNamed(const QString& id) const
{
  Q_D(const DotGraph);
  GraphElement* ret = 0;
  if ((ret = d->m_nodesMap.value(id, 0))) {
    return ret;
  }
  if ((ret = d->m_edgesMap.value(id, 0))) {
    return ret;
  }
  foreach(GraphSubgraph* subGraph, subgraphs()) {
    if ((ret = subGraph->elementNamed(id))) {
      return ret;
    }
  }
  return 0;
}

void DotGraph::setGraphAttributes(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  attributes() = attribs;
}


void DotGraph::addNewNode(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  GraphNode* newNode = new GraphNode();
  newNode->attributes() = attribs;
  nodes().insert(newNode->id(), newNode);
  kDebug() << "node added as" << newNode->id();
}

void DotGraph::addNewSubgraph(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  GraphSubgraph* newSG = new GraphSubgraph();
  newSG->attributes() = attribs;
  subgraphs()[newSG->id()] = newSG;
  kDebug() << "subgraph added as" << newSG->id();
}

void DotGraph::addNewNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph)
{
  kDebug() << attribs << "to" << subgraph;
  if (!subgraphs().contains(subgraph)) {
    kWarning() << "Invalid subgraph:" << subgraph;
    return;
  }

  GraphNode* newNode = new GraphNode();
  newNode->attributes() = attribs;
  subgraphs()[subgraph]->content().push_back(newNode);
  kDebug() << "node added as" << newNode->id() << "in" << subgraph;
}

void DotGraph::addExistingNodeToSubgraph(QMap<QString,QString> attribs,QString subgraph)
{
  kDebug() << attribs << "to" << subgraph;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(attribs["id"]));
  if (node == 0)
  {
    kError() << "No such node" << attribs["id"];
    return;
  }
  if (nodes().contains(attribs["id"]))
  {
    nodes().remove(attribs["id"]);
    node->attributes() = attribs;
    subgraphs()[subgraph]->content().push_back(node);
    kDebug() << "node " << node->id() << " added in " << subgraph;
  }
  else
  {
    foreach(GraphSubgraph* gs, subgraphs())
    {
      GraphElement* elt = 0;
      foreach(GraphElement* element, gs->content())
      {
        if (element == node)
        {
          elt = element;
          break;
        }
      }
      if (elt != 0)
      {
        kDebug() << "removing node " << elt->id() << " from " << gs->id();
        gs->removeElement(elt);
        subgraphs()[subgraph]->content().push_back(elt);
        kDebug() << "node " << elt->id() << " added in " << subgraph;
        break;
      }
    }
  }
}

void DotGraph::moveExistingNodeToMainGraph(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(attribs["id"]));
  if (node == 0)
  {
    kError() << "No such node" << attribs["id"];
    return;
  }
  else if (nodes().contains(attribs["id"]))
  {
    kError() << "Node" << attribs["id"] << "already in main graph";
    return;
  }
  else
  {
    foreach(GraphSubgraph* gs, subgraphs())
    {
      bool found = false;
      foreach(GraphElement* element, gs->content())
      {
        if (element == node)
        {
          found = true;
          break;
        }
      }
      if (found)
      {
        kDebug() << "removing node " << node->id() << " from " << gs->id();
        gs->removeElement(node);
        nodes()[node->id()] = node;
        kDebug() << "node " << node->id() << " moved to main graph";
        break;
      }
    }
  }
}

void DotGraph::addNewEdge(QString src, QString tgt, QMap<QString,QString> attribs)
{
  kDebug() << src << tgt << attribs;
  GraphEdge* newEdge = new GraphEdge();
  newEdge->attributes() = attribs;
  GraphElement* srcElement = elementNamed(src);
  if (srcElement == 0)
  {
    srcElement = elementNamed(QString("cluster_")+src);
  }
  GraphElement* tgtElement = elementNamed(tgt);
  if (tgtElement == 0)
  {
    tgtElement = elementNamed(QString("cluster_")+tgt);
  }

  if (srcElement == 0 || tgtElement == 0)
  {
    kError() << src << "or" << tgt << "missing";
    return;
  }
  if (attribs.contains("id"))
  {
    newEdge->setId(attribs["id"]);
  }
  else
  {
    newEdge->setId(QString::number(edges().size()+1));
  }
  newEdge->setFromNode(srcElement);
  newEdge->setToNode(tgtElement);
  edges().insert(newEdge->id(), newEdge);
}

void DotGraph::removeElementAttribute(const QString& nodeName, const QString& attribName)
{
  kDebug();
  GraphElement* element = elementNamed(nodeName);
  if (element != 0)
  {
    element->removeAttribute(attribName);
  }
}

void DotGraph::renameNode(const QString& oldNodeName, const QString& newNodeName)
{
  if (oldNodeName != newNodeName)
  {
    kDebug() << "Renaming " << oldNodeName << " into " << newNodeName;
    GraphNode* node = nodes()[oldNodeName];
    nodes().remove(oldNodeName);
    node->setId(newNodeName);
    nodes()[newNodeName] = node;
  }
}

QString DotGraph::backColor() const
{
  if (m_attributes.find("bgcolor") != m_attributes.end())
  {
    return m_attributes["bgcolor"];
  }
  else
  {
    return QString();
  }
}

QTextStream& KGraphViz::operator<<(QTextStream& s, const KGraphViz::DotGraph& graph)
{
  if (graph.strict())
    s << "strict ";

  s << (graph.directed() ? "digraph " : "graph ") << graph.id() << " {" << endl;
  foreach(const GraphSubgraph* subgraph, graph.subgraphs()) {
    s << *subgraph;
  }
  foreach(const GraphNode* node, graph.nodes()) {
    s << *node;
  }
  foreach(const GraphEdge* edge, graph.edges()) {
    s << *edge;
  }
  s << "}" << endl;
  return s;
}

#include "dotgraph.moc"
