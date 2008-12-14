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


#include <iostream>
// #include <math.h>
#include "fdstream.hpp"
#include <boost/spirit/utility/confix.hpp>

#include <kdebug.h>
#include <KMessageBox>

#include <QFile>
#include <QPair>
#include <QByteArray>
#include <QProcess>
#include <QMutexLocker>



using namespace boost;
using namespace boost::spirit;

extern DotGraphParsingHelper* phelper;

const distinct_parser<> keyword_p("0-9a-zA-Z_");

DotGraph::DotGraph() :
  GraphElement(),
  m_dotFileName(""),m_width(0.0), m_height(0.0),m_scale(1.0),
  m_directed(true),m_strict(false),
  m_layoutCommand(""),
  m_readWrite(false),
  m_dot(0),
  m_phase(Initial)
{
}

DotGraph::DotGraph(const QString& command, const QString& fileName) :
  GraphElement(),
  m_dotFileName(fileName),m_width(0.0), m_height(0.0),m_scale(1.0),
  m_directed(true),m_strict(false),
  m_layoutCommand(command),
  m_readWrite(false),
  m_dot(0),
  m_phase(Initial)
{ 
}

DotGraph::~DotGraph()  
{
  GraphNodeMap::iterator itn, itn_end;
  itn = m_nodesMap.begin(); itn_end = m_nodesMap.end();
  for (; itn != itn_end; itn++)
  {
    delete *itn;
  }

  GraphEdgeMap::iterator ite, ite_end;
  ite = m_edgesMap.begin(); ite_end = m_edgesMap.end();
  for (; ite != ite_end; ite++)
  {
    delete (*ite);
  }
}

QString DotGraph::chooseLayoutProgramForFile(const QString& str)
{
  QFile iFILE(str);

  if (!iFILE.open(QIODevice::ReadOnly))
  {
    kError() << "Can't test dot file. Will try to use the dot command on the file: '" << str << "'" << endl;
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
  kDebug() << str;
  if (m_layoutCommand.isEmpty())
  {
    m_layoutCommand = chooseLayoutProgramForFile(str);
    if (m_layoutCommand.isEmpty())
    {
      m_layoutCommand = chooseLayoutProgramForFile(str);
      return false;
    }
  }

  kDebug() << "Running " << m_layoutCommand  << str;
  QStringList options;
  /// @TODO handle the non-dot commands that could don't know the -T option
  if (m_readWrite && m_phase == Initial)
  {
    options << "-Tdot";
  }
  else
  {
    options << "-Txdot";
  }
  options << str;

  kDebug() << "m_dot is " << m_dot  << ". Acquiring mutex";
  QMutexLocker locker(&m_dotProcessMutex);
  kDebug() << "mutex acquired ";
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
  kDebug() << "process started";
 return true;
}

bool DotGraph::update()
{
  kDebug() ;
  GraphExporter exporter;
  QString str = exporter.writeDot(this);

  return parseDot(str);
}


QByteArray DotGraph::getDotResult(int , QProcess::ExitStatus )
{
  kDebug();

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
  kDebug();
  
  QByteArray result = getDotResult(exitCode, exitStatus);
  result.replace("\\\n","");

  kDebug() << "string content is:" << endl << result << endl << "=====================" << result.size();
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

  kDebug() << "parsing new dot";
  bool parsingResult = parse(s);
  delete phelper;
  phelper = 0;
  kDebug() << "phelper deleted";

  if (parsingResult)
  {
    updateWithGraph(newGraph);
  }
  else
  {
    kError() << "parsing failed";
  }
//   return parsingResult;
  if (m_readWrite && m_phase == Initial)
  {
    m_phase = Final;
    update();
  }
  else
  {
    kDebug() << "emiting readyToDisplay";
    emit(readyToDisplay());
  }
}

void DotGraph::slotDotRunningError(QProcess::ProcessError error)
{
  kError() << "DotGraph::slotDotRunningError" << error;
  switch (error)
  {
    case QProcess::FailedToStart:
      KMessageBox::error(0, i18n("Unable to start %1.", m_layoutCommand),i18n("Layout process failed"),KMessageBox::Notify);
    break;
    case QProcess::Crashed:
      KMessageBox::error(0, i18n("%1 crashed.", m_layoutCommand),i18n("Layout process failed"),KMessageBox::Notify);
    break;
    case QProcess::Timedout:
      KMessageBox::error(0, i18n("%1 timed out.", m_layoutCommand),i18n("Layout process failed"),KMessageBox::Notify);
    break;
    case QProcess::WriteError:
      KMessageBox::error(0, i18n("Was not able to write data to the %1 process.", m_layoutCommand),i18n("Layout process failed"),KMessageBox::Notify);
    break;
    case QProcess::ReadError:
      KMessageBox::error(0, i18n("Was not able to read data from the %1 process.", m_layoutCommand),i18n("Layout process failed"),KMessageBox::Notify);
    break;
    default:
      KMessageBox::error(0, i18n("Unknown error running %1.", m_layoutCommand),i18n("Layout process failed"),KMessageBox::Notify);
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
  kDebug() << fileName;
  m_dotFileName = fileName;
  GraphExporter exporter;
  exporter.writeDot(this, fileName);
}

void DotGraph::updateWithGraph(const DotGraph& newGraph)
{
  kDebug();
  GraphElement::updateWithElement(newGraph);
  m_width=newGraph.width();
  m_height=newGraph.height();
  m_scale=newGraph.scale();
  m_directed=newGraph.directed();
  m_strict=newGraph.strict();
  computeCells();
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
    kDebug() << "node " << ngn->id();
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
      if (edges()[nge->id()]->canvasEdge()!=0)
      {
//         edges()[nge->id()]->canvasEdge()->setGh(m_height);
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
  computeCells();
}

void DotGraph::removeNodeNamed(const QString& nodeName)
{
  kDebug() << nodeName;
  GraphNode* node = dynamic_cast<GraphNode*>(elementNamed(nodeName));
  if (node == 0)
  {
    kError() << "No such node " << nodeName;
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
  kDebug() << subgraphName << " from " << subgraphs().keys();
  GraphSubgraph* subgraph = subgraphs()[subgraphName];

  if (subgraph == 0)
  {
    kError() << "Subgraph" << subgraphName << "not found";
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
  foreach (const QString& eid, edges().keys())
  {
    GraphEdge* edge = edges()[eid];
    if (edge->id() ==id)
    {
      if (edge->canvasEdge() != 0)
      {
        edge->canvasEdge()->hide();
        delete edge->canvasEdge();
        delete edge;
      }
      edges().remove(eid);
      break;
    }
  }
}

void DotGraph::removeElement(const QString& id)
{
  kDebug() << id;
  foreach (const QString& eid, nodes().keys())
  {
    GraphNode* node = nodes()[eid];
    if (node->id() == id)
    {
      removeNodeNamed(id);
      return;
    }
  }
  foreach (const QString& eid, edges().keys())
  {
    GraphEdge* edge = edges()[eid];
    if (edge->id() == id)
    {
      removeEdge(id);
      return;
    }
  }
  foreach (const QString& sid, subgraphs().keys())
  {
    GraphSubgraph* subgraph = subgraphs()[sid];
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
  foreach (const QString& nid, nodes().keys())
  {
    if (nid == id)
    {
      return nodes()[nid];
    }
  }
  foreach (const QString& eid, edges().keys())
  {
    if (eid == id)
    {
      return edges()[eid];
    }
  }
  foreach (const QString& sid, subgraphs().keys())
  {
    GraphElement* element = subgraphs()[sid]->elementNamed(id);
    if (element != 0)
    {
      return element;
    }
  }
  return 0;
}

#include "dotgraph.moc"
