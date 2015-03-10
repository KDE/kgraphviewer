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
#include "dotgrammar.h"
#include "graphexporter.h"
#include "DotGraphParsingHelper.h"
#include "canvasedge.h"
#include "canvassubgraph.h"
#include "layoutagraphthread.h"


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "fdstream.hpp"
#include <boost/spirit/include/classic_confix.hpp>
#include <graphviz/gvc.h>

#include <QMessageBox>

#include <QFile>
#include <QPair>
#include <QByteArray>
#include <QProcess>
#include <QMutexLocker>
#include <QUuid>
#include <QLoggingCategory>

static QLoggingCategory debugCategory("org.kde.kgraphviewer");

using namespace boost;
using namespace boost::spirit::classic;

extern KGraphViewer::DotGraphParsingHelper* phelper;


namespace KGraphViewer
{
  
  
const distinct_parser<> keyword_p("0-9a-zA-Z_");

DotGraph::DotGraph() :
  GraphElement(),
  m_dotFileName(""),m_width(0.0), m_height(0.0),m_scale(1.0),
  m_directed(true),m_strict(false),
  m_layoutCommand(""),
  m_horizCellFactor(0), m_vertCellFactor(0),
  m_wdhcf(0), m_hdvcf(0),
  m_readWrite(false),
  m_dot(0),
  m_phase(Initial),
  m_useLibrary(false)
{
  setId("unnamed");
}

DotGraph::DotGraph(const QString& command, const QString& fileName) :
  GraphElement(),
  m_dotFileName(fileName),m_width(0.0), m_height(0.0),m_scale(1.0),
  m_directed(true),m_strict(false),
  m_layoutCommand(command),
  m_horizCellFactor(0), m_vertCellFactor(0),
  m_wdhcf(0), m_hdvcf(0),
  m_readWrite(false),
  m_dot(0),
  m_phase(Initial),
  m_useLibrary(false)
{
  setId("unnamed");
}

DotGraph::~DotGraph()  
{
  qDeleteAll(m_subgraphsMap);
  m_subgraphsMap.clear();
  qDeleteAll(m_nodesMap);
  m_nodesMap.clear();
  qDeleteAll(m_edgesMap);
  m_edgesMap.clear();
}

QString DotGraph::chooseLayoutProgramForFile(const QString& str)
{
  QFile iFILE(str);

  if (!iFILE.open(QIODevice::ReadOnly))
  {
    qWarning() << "Can't test dot file. Will try to use the dot command on the file: '" << str << "'" << endl;
    return "dot";// -Txdot";
  }

  QByteArray fileContent = iFILE.readAll();
  if (fileContent.isEmpty()) return "";
  std::string s =  fileContent.data();
  std::string cmd = "dot";
  parse(s.c_str(),
        (
          !(keyword_p("strict")) >> (keyword_p("graph")[assign_a(cmd,"neato")])
        ), (space_p|comment_p("/*", "*/")) );

  return  QString::fromStdString(cmd);// + " -Txdot" ;
}

bool DotGraph::parseDot(const QString& str)
{
  qCDebug(debugCategory) << str;
  m_useLibrary = false;
  if (m_layoutCommand.isEmpty())
  {
    m_layoutCommand = chooseLayoutProgramForFile(str);
    if (m_layoutCommand.isEmpty())
    {
      m_layoutCommand = chooseLayoutProgramForFile(str);
      return false;
    }
  }

  qCDebug(debugCategory) << "Running " << m_layoutCommand  << str;
  QStringList options;
  /// @TODO handle the non-dot commands that could don't know the -T option
//  if (m_readWrite && m_phase == Initial)
//  {
//    options << "-Tdot";
//  }
//  else
//  {
    options << "-Txdot";
//   }
  options << str;

  qCDebug(debugCategory) << "m_dot is " << m_dot  << ". Acquiring mutex";
  QMutexLocker locker(&m_dotProcessMutex);
  qCDebug(debugCategory) << "mutex acquired ";
  if (m_dot != 0)
  {
    disconnect(m_dot,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slotDotRunningDone(int,QProcess::ExitStatus)));
    disconnect(m_dot,SIGNAL(error(QProcess::ProcessError)),this,SLOT(slotDotRunningError(QProcess::ProcessError)));
    m_dot->kill();
    delete m_dot;
  }
  m_dot = new QProcess();
  connect(m_dot,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slotDotRunningDone(int,QProcess::ExitStatus)));
  connect(m_dot,SIGNAL(error(QProcess::ProcessError)),this,SLOT(slotDotRunningError(QProcess::ProcessError)));
  m_dot->start(m_layoutCommand, options);
  qCDebug(debugCategory) << "process started";
 return true;
}

bool DotGraph::update()
{
  GraphExporter exporter;
  if (!m_useLibrary)
  {
    qCDebug(debugCategory) << "command";
    QString str = exporter.writeDot(this);
    return parseDot(str);
  }
  else
  {
    qCDebug(debugCategory) << "library";
    graph_t* graph = exporter.exportToGraphviz(this);

    GVC_t* gvc = gvContext();
    threadsafe_wrap_gvLayout(gvc, graph, m_layoutCommand.toUtf8().data());
    threadsafe_wrap_gvRender(gvc, graph, "xdot", NULL);

    updateWithGraph(graph);
    
    gvFreeLayout(gvc, graph);
    agclose(graph);
    bool result = true; //(gvFreeContext(gvc) == 0);
    return result;
  }
}

QByteArray DotGraph::getDotResult(int , QProcess::ExitStatus )
{
  qCDebug(debugCategory);

  QMutexLocker locker(&m_dotProcessMutex);
  if (m_dot == 0)
  {
    return QByteArray();
  }
  QByteArray result = m_dot->readAll();
  delete m_dot;
  m_dot = 0;
  return result;
}

void DotGraph::slotDotRunningDone(int exitCode, QProcess::ExitStatus exitStatus)
{
  qCDebug(debugCategory);
  
  QByteArray result = getDotResult(exitCode, exitStatus);
  result.replace("\\\n","");

  qCDebug(debugCategory) << "string content is:" << endl << result << endl << "=====================" << result.size();
  std::string s =  result.data();
  //   std::cerr << "stdstring content is:" << std::endl << s << std::endl << "===================== " << s.size() << std::endl;
  if (phelper != 0)
  {
    phelper->graph = 0;
    delete phelper;
  }
//   if (parsingResult)
//   {
//     if (m_readWrite)
//     {
//       storeOriginalAttributes();
//       update();
//     }
//     computeCells();
//   }

  DotGraph newGraph(m_layoutCommand, m_dotFileName);
  phelper = new DotGraphParsingHelper;
  phelper->graph = &newGraph;
  phelper->z = 1;
  phelper->maxZ = 1;
  phelper->uniq = 0;

  qCDebug(debugCategory) << "parsing new dot";
  bool parsingResult = parse(s);
  delete phelper;
  phelper = 0;
  qCDebug(debugCategory) << "phelper deleted";

  if (parsingResult)
  {
    qCDebug(debugCategory) << "calling updateWithGraph";
    updateWithGraph(newGraph);
  }
  else
  {
    qWarning() << "parsing failed";
  }
//   return parsingResult;
//   if (m_readWrite && m_phase == Initial)
//   {
//     m_phase = Final;
//     update();
//   }
//   else
//   {
    qCDebug(debugCategory) << "emiting readyToDisplay";
    emit(readyToDisplay());
//   }
}

void DotGraph::slotDotRunningError(QProcess::ProcessError error)
{
  qWarning() << "DotGraph::slotDotRunningError" << error;
  switch (error)
  {
    case QProcess::FailedToStart:
      QMessageBox::critical(0, i18n("Layout process failed"), i18n("Unable to start %1.", m_layoutCommand));
    break;
    case QProcess::Crashed:
      QMessageBox::critical(0, i18n("Layout process failed"), i18n("%1 crashed.", m_layoutCommand));
    break;
    case QProcess::Timedout:
      QMessageBox::critical(0, i18n("Layout process failed"), i18n("%1 timed out.", m_layoutCommand));
    break;
    case QProcess::WriteError:
      QMessageBox::critical(0, i18n("Layout process failed"), i18n("Was not able to write data to the %1 process.", m_layoutCommand));
    break;
    case QProcess::ReadError:
      QMessageBox::critical(0, i18n("Layout process failed"), i18n("Was not able to read data from the %1 process.", m_layoutCommand));
    break;
    default:
      QMessageBox::critical(0, i18n("Layout process failed"), i18n("Unknown error running %1.", m_layoutCommand));
  }
}

unsigned int DotGraph::cellNumber(int x, int y)
{
/*  kDebug() << "x= " << x << ", y= " << y << ", m_width= " << m_width << ", m_height= " << m_height << ", m_horizCellFactor= " << m_horizCellFactor << ", m_vertCellFactor= " << m_vertCellFactor  << ", m_wdhcf= " << m_wdhcf << ", m_hdvcf= " << m_hdvcf;*/
  
  unsigned int nx = (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf);
  unsigned int ny = (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf);
/*  kDebug() << "nx = " << (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf);
  kDebug() << "ny = " << (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf);
  kDebug() << "res = " << ny * m_horizCellFactor + nx;*/
  
  unsigned int res = ny * m_horizCellFactor + nx;
  return res;
}

#define MAXCELLWEIGHT 800

void DotGraph::computeCells()
{
  return;
  qCDebug(debugCategory) << m_width << m_height << endl;
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
      qCDebug(debugCategory) << "Found cell number " << cellNum;

      if (m_cells.size() <= cellNum)
      {
        m_cells.resize(cellNum+1);
      }
      m_cells[cellNum].insert(gn);
      
      qCDebug(debugCategory) << "after insert";
      if ( m_cells[cellNum].size() > MAXCELLWEIGHT )
      {
        qCDebug(debugCategory) << "cell number " << cellNum  << " contains " << m_cells[cellNum].size() << " nodes";
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
        qCDebug(debugCategory) << "cell factor is now " << m_horizCellFactor << " / " << m_vertCellFactor;
        stop = false;
        break;
      }
    }
  } while (!stop);
  qCDebug(debugCategory) << "m_wdhcf=" << m_wdhcf << "; m_hdvcf=" << m_hdvcf << endl;
  qCDebug(debugCategory) << "finished" << endl;
}

QSet< GraphNode* >& DotGraph::nodesOfCell(unsigned int id)
{
  return m_cells[id];
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
  qCDebug(debugCategory) << fileName;
  m_dotFileName = fileName;
  GraphExporter exporter;
  exporter.writeDot(this, fileName);
}

void DotGraph::updateWithGraph(graph_t* newGraph)
{
  qCDebug(debugCategory);

  // copy global graph render operations and attributes
  DotRenderOpVec ops;
  // decrease mem peak
  setRenderOperations(ops);

  if (agget(newGraph, (char*)"_draw_") != NULL)
  {
    parse_renderop(agget(newGraph, (char*)"_draw_"), ops);
    qCDebug(debugCategory) << "_draw_: element renderOperations size is now " << ops.size();
  }
  if (agget(newGraph, (char*)"_ldraw_") != NULL)
  {
    parse_renderop(agget(newGraph, (char*)"_ldraw_"), ops);
    qCDebug(debugCategory) << "_ldraw_: element renderOperations size is now " << ops.size();
  }

  setRenderOperations(ops);

  Agsym_t *attr = agnxtattr(newGraph, AGRAPH, NULL);
  while(attr)
  {
    qCDebug(debugCategory) << agnameof(newGraph) << ":" << attr->name << agxget(newGraph,attr);
    m_attributes[attr->name] = agxget(newGraph,attr);
    attr = agnxtattr(newGraph, AGRAPH, attr);
  }
  
  // copy subgraphs
  for (graph_t* sg = agfstsubg(newGraph); sg; sg = agnxtsubg(sg))
  {
    qCDebug(debugCategory) << "subgraph:" << agnameof(sg);
    if (subgraphs().contains(agnameof(sg)))
    {
      qCDebug(debugCategory) << "known";
      // ???
      //       nodes()[ngn->name]->setZ(ngn->z());
      subgraphs()[agnameof(sg)]->updateWithSubgraph(sg);
      if (subgraphs()[agnameof(sg)]->canvasElement()!=0)
      {
        //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      qCDebug(debugCategory) << "new";
      GraphSubgraph* newsg = new GraphSubgraph(sg);
      //       kDebug() << "new created";
      subgraphs().insert(agnameof(sg), newsg);
      //       kDebug() << "new inserted";
    }

  }

  // copy nodes
  node_t* ngn = agfstnode(newGraph);
  qCDebug(debugCategory) << "first node:" << (void*)ngn;
  
  while (ngn != NULL)
//   foreach (GraphNode* ngn, newGraph.nodes())
  {
    qCDebug(debugCategory) << "node " << agnameof(ngn);
    if (nodes().contains(agnameof(ngn)))
    {
      qCDebug(debugCategory) << "known";
// ???
//       nodes()[ngn->name]->setZ(ngn->z());
      nodes()[agnameof(ngn)]->updateWithNode(ngn);
      if (nodes()[agnameof(ngn)]->canvasElement()!=0)
      {
        //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      qCDebug(debugCategory) << "new";
      GraphNode* newgn = new GraphNode(ngn);
      //       kDebug() << "new created";
      nodes().insert(agnameof(ngn), newgn);
      //       kDebug() << "new inserted";
    }

    // copy node edges
    edge_t* nge = agfstout(newGraph, ngn);
    while (nge != NULL)
    {
//      kDebug() << "edge " << nge->id;
      QString edgeName = QString(agnameof(aghead(nge))) + agnameof(agtail(nge));
      if (edges().contains(edgeName))
      {
//        () << "edge known" << nge->id;
//         edges()[nge->name]->setZ(nge->z());
        edges()[edgeName]->updateWithEdge(nge);
        if (edges()[edgeName]->canvasEdge()!=0)
        {
          //         edges()[nge->id()]->canvasEdge()->setGh(m_height);
        }
      }
      else
      {
        qCDebug(debugCategory) << "new edge" << edgeName;
        {
          GraphEdge* newEdge = new GraphEdge();
          newEdge->setId(edgeName);
          newEdge->updateWithEdge(nge);
          if (elementNamed(agnameof(agtail(nge))) == 0)
          {
            GraphNode* newgn = new GraphNode();
            //       kDebug() << "new created";
            nodes().insert(agnameof(agtail(nge)), newgn);
          }
          newEdge->setFromNode(elementNamed(agnameof(agtail(nge))));
          if (elementNamed(agnameof(aghead(nge))) == 0)
          {
            GraphNode* newgn = new GraphNode();
            //       kDebug() << "new created";
            nodes().insert(agnameof(aghead(nge)), newgn);
          }
          newEdge->setToNode(elementNamed(agnameof(aghead(nge))));
          edges().insert(edgeName, newEdge);
        }
      }
      nge = agnxtedge(newGraph, nge, ngn);
    }
    ngn = agnxtnode(newGraph, ngn);
  }
  qCDebug(debugCategory) << "Done";
  emit readyToDisplay();
  computeCells();
}

void DotGraph::updateWithGraph(const DotGraph& newGraph)
{
  GraphElement::updateWithElement(newGraph);
  m_width=newGraph.width();
  m_height=newGraph.height();
  m_scale=newGraph.scale();
  m_directed=newGraph.directed();
  m_strict=newGraph.strict();
  computeCells();
  foreach (GraphSubgraph* nsg, newGraph.subgraphs())
  {
    qCDebug(debugCategory) << "subgraph" << nsg->id();
    if (subgraphs().contains(nsg->id()))
    {
      qCDebug(debugCategory) << "subgraph known" << nsg->id();
      subgraphs().value(nsg->id())->updateWithSubgraph(*nsg);
      if (subgraphs().value(nsg->id())->canvasElement()!=0)
      {
//         subgraphs().value(nsg->id())->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      qCDebug(debugCategory) << "new subgraph" << nsg->id();
      GraphSubgraph* newSubgraph = new GraphSubgraph();
      newSubgraph->updateWithSubgraph(*nsg);
      newSubgraph->setZ(0);
      subgraphs().insert(nsg->id(), newSubgraph);
    }
  }
  foreach (GraphNode* ngn, newGraph.nodes())
  {
    qCDebug(debugCategory) << "node " << ngn->id();
    if (nodes().contains(ngn->id()))
    {
      qCDebug(debugCategory) << "known";
      nodes()[ngn->id()]->setZ(ngn->z());
      nodes()[ngn->id()]->updateWithNode(*ngn);
      if (nodes()[ngn->id()]->canvasElement()!=0)
      {
//         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      qCDebug(debugCategory) << "new";
      GraphNode* newgn = new GraphNode(*ngn);
//       kDebug() << "new created";
      nodes().insert(ngn->id(), newgn);
//       kDebug() << "new inserted";
    }
  }
  foreach (GraphEdge* nge, newGraph.edges())
  {
    qCDebug(debugCategory) << "edge " << nge->id();
    if (edges().contains(nge->id()))
    {
      qCDebug(debugCategory) << "edge known" << nge->id();
      edges()[nge->id()]->setZ(nge->z());
      edges()[nge->id()]->updateWithEdge(*nge);
      if (edges()[nge->id()]->canvasEdge()!=0)
      {
//         edges()[nge->id()]->canvasEdge()->setGh(m_height);
      }
    }
    else
    {
      qCDebug(debugCategory) << "new edge" << nge->id();
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
  qCDebug(debugCategory) << "Done";
  computeCells();
}

void DotGraph::removeNodeNamed(const QString& nodeName)
{
  qCDebug(debugCategory) << nodeName;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(nodeName));
  if (node == 0)
  {
    qWarning() << "No such node " << nodeName;
    return;
  }
  
  GraphEdgeMap::iterator it, it_end;
  it = m_edgesMap.begin(); it_end = m_edgesMap.end();
  while (it != it_end)
  {
    if ( it.value()->fromNode() == node
        || it.value()->toNode() == node )
    {
      GraphEdge* edge = it.value();
      if (edge->canvasEdge() != 0)
      {
        edge->canvasEdge()->hide();
        delete edge->canvasEdge();
        delete edge;
      }
      it = edges().erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (node->canvasNode() != 0)
  {
    node->canvasNode()->hide();
    delete node->canvasNode();
    node->setCanvasNode(0);
  }
  nodes().remove(nodeName);
  delete node;

}

void DotGraph::removeNodeFromSubgraph(
    const QString& nodeName,
    const QString& subgraphName)
{
  qCDebug(debugCategory) << nodeName << subgraphName;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(nodeName));
  if (node == 0)
  {
    qWarning() << "No such node " << nodeName;
    return;
  }

  GraphSubgraph* subgraph = subgraphs()[subgraphName];
  if (subgraph == 0)
  {
    qWarning() << "No such subgraph " << subgraphName;
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
  qCDebug(debugCategory) << subgraphName << " from " << subgraphs().keys();
  GraphSubgraph* subgraph = subgraphs()[subgraphName];

  if (subgraph == 0)
  {
    qWarning() << "Subgraph" << subgraphName << "not found";
    return;
  }
  GraphEdgeMap::iterator it, it_end;
  it = m_edgesMap.begin(); it_end = m_edgesMap.end();
  while (it != it_end)
  {
    if ( it.value()->fromNode() == subgraph
        || it.value()->toNode() == subgraph )
    {
      GraphEdge* edge = it.value();
      if (edge->canvasEdge() != 0)
      {
        edge->canvasEdge()->hide();
        delete edge->canvasEdge();
        delete edge;
      }
      it = edges().erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (subgraph->canvasSubgraph() != 0)
  {
    subgraph->canvasSubgraph()->hide();
    delete subgraph->canvasSubgraph();
    subgraph->setCanvasSubgraph(0);
  }
  foreach(GraphElement* element, subgraph->content())
  {
    if (dynamic_cast<GraphNode*>(element) != 0)
    {
      qCDebug(debugCategory) << "Adding" << element->id() << "to main graph";
      nodes()[element->id()] = dynamic_cast<GraphNode*>(element);
    }
    else if (dynamic_cast<GraphSubgraph*>(element) != 0)
    {
      subgraphs()[element->id()] = dynamic_cast<GraphSubgraph*>(element);
    }
    else
    {
      qWarning() << "Don't know how to handle" << element->id();
    }
  }
  subgraph->content().clear();
  subgraphs().remove(subgraphName);
  delete subgraph;
}

void DotGraph::removeEdge(const QString& id)
{
  qCDebug(debugCategory) << id;
  GraphEdgeMap::iterator it = edges().begin();
  for (; it != edges().end(); it++)
  {
    GraphEdge* edge = it.value();
    if (edge->id() ==id)
    {
      if (edge->canvasEdge() != 0)
      {
        edge->canvasEdge()->hide();
        delete edge->canvasEdge();
        delete edge;
      }
      edges().remove(id);
      break;
    }
  }
}

void DotGraph::removeElement(const QString& id)
{
  qCDebug(debugCategory) << id;
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

GraphElement* DotGraph::elementNamed(const QString& id)
{
  GraphElement* ret = 0;
  if ((ret = m_nodesMap.value(id, 0))) {
    return ret;
  }
  if ((ret = m_edgesMap.value(id, 0))) {
    return ret;
  }
  foreach(GraphSubgraph* subGraph, m_subgraphsMap) {
    if ((ret = subGraph->elementNamed(id))) {
      return ret;
    }
  }
  return 0;
}

void DotGraph::setGraphAttributes(QMap<QString,QString> attribs)
{
  qCDebug(debugCategory) << attribs;
  attributes() = attribs;
}


void DotGraph::addNewNode(QMap<QString,QString> attribs)
{
  qCDebug(debugCategory) << attribs;
  GraphNode* newNode = new GraphNode();
  newNode->attributes() = attribs;
  nodes().insert(newNode->id(), newNode);
  qCDebug(debugCategory) << "node added as" << newNode->id();
}

void DotGraph::addNewSubgraph(QMap<QString,QString> attribs)
{
  qCDebug(debugCategory) << attribs;
  GraphSubgraph* newSG = new GraphSubgraph();
  newSG->attributes() = attribs;
  subgraphs()[newSG->id()] = newSG;
  qCDebug(debugCategory) << "subgraph added as" << newSG->id();
}

void DotGraph::addNewNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph)
{
  qCDebug(debugCategory) << attribs << "to" << subgraph;
  GraphNode* newNode = new GraphNode();
  newNode->attributes() = attribs;
  subgraphs()[subgraph]->content().push_back(newNode);

  qCDebug(debugCategory) << "node added as" << newNode->id() << "in" << subgraph;
}

void DotGraph::addExistingNodeToSubgraph(QMap<QString,QString> attribs,QString subgraph)
{
  qCDebug(debugCategory) << attribs << "to" << subgraph;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(attribs["id"]));
  if (node == 0)
  {
    qWarning() << "No such node" << attribs["id"];
    return;
  }
  if (nodes().contains(attribs["id"]))
  {
    nodes().remove(attribs["id"]);
    node->attributes() = attribs;
    subgraphs()[subgraph]->content().push_back(node);
    qCDebug(debugCategory) << "node " << node->id() << " added in " << subgraph;
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
        qCDebug(debugCategory) << "removing node " << elt->id() << " from " << gs->id();
        gs->removeElement(elt);
        subgraphs()[subgraph]->content().push_back(elt);
        qCDebug(debugCategory) << "node " << elt->id() << " added in " << subgraph;
        break;
      }
    }
  }
}

void DotGraph::moveExistingNodeToMainGraph(QMap<QString,QString> attribs)
{
  qCDebug(debugCategory) << attribs;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(attribs["id"]));
  if (node == 0)
  {
    qWarning() << "No such node" << attribs["id"];
    return;
  }
  else if (nodes().contains(attribs["id"]))
  {
    qWarning() << "Node" << attribs["id"] << "already in main graph";
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
        qCDebug(debugCategory) << "removing node " << node->id() << " from " << gs->id();
        gs->removeElement(node);
        nodes()[node->id()] = node;
        qCDebug(debugCategory) << "node " << node->id() << " moved to main graph";
        break;
      }
    }
  }
}

void DotGraph::addNewEdge(QString src, QString tgt, QMap<QString,QString> attribs)
{
  qCDebug(debugCategory) << src << tgt << attribs;
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
    qWarning() << src << "or" << tgt << "missing";
    return;
  }
  if (attribs.contains("id"))
  {
    newEdge->setId(attribs["id"]);
  }
  else
  {
    newEdge->setId(src+tgt+QUuid::createUuid().toString().remove('{').remove('}').remove('-'));
  }
  newEdge->setFromNode(srcElement);
  newEdge->setToNode(tgtElement);
  edges().insert(newEdge->id(), newEdge);
}

void DotGraph::removeAttribute(const QString& nodeName, const QString& attribName)
{
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
    qCDebug(debugCategory) << "Renaming " << oldNodeName << " into " << newNodeName;
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


}

#include "dotgraph.moc"
