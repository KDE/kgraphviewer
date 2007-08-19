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


// #include <iostream>
// #include <math.h>
#include "fdstream.hpp"
#include <boost/spirit/utility/confix.hpp>

#include <kdebug.h>
#include <QFile>
#include <QPair>
#include <QByteArray>
#include <QProcess>



using namespace boost;
using namespace boost::spirit;

extern DotGraphParsingHelper* phelper;

const distinct_parser<> keyword_p("0-9a-zA-Z_");

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
  if (m_layoutCommand.isEmpty())
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
    
    m_layoutCommand = QString::fromStdString(cmd);// + " -Txdot" ;
  }
//   std::cerr << "File " << str << " will be layouted by '" << m_layoutCommand << "'" << std::endl;
  return m_layoutCommand;
}

bool DotGraph::parseDot(const QString& str)
{
  kDebug() << k_funcinfo << str;
  if (m_layoutCommand.isEmpty())
  {
    return false;
  }

  kDebug() << "Running " << m_layoutCommand  << str;
  QStringList options;
  if (m_readWrite && m_phase == Initial)
  {
    options << "-Tdot";
  }
  else
  {
    options << "-Txdot";
  }
  options << str;

  if (m_dot != 0)
  {
    m_dot->terminate();
  }
  m_dot = new QProcess();
  connect(m_dot,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slotDotRunningDone(int,QProcess::ExitStatus)));
  m_dot->start(m_layoutCommand, options);
  return true;
}

bool DotGraph::update()
{
  kDebug() << k_funcinfo;
  GraphExporter exporter;
  QString str = exporter.writeDot(this);

  return parseDot(str);
}

void DotGraph::slotDotRunningDone(int,QProcess::ExitStatus)
{
  qDebug() << k_funcinfo;
  if (m_dot == 0)
  {
    return;
  }
  QByteArray result = m_dot->readAll();
  m_dot->deleteLater();
  result.replace("\\\n","");

  qDebug() << k_funcinfo << "string content is:" << endl << result << endl << "=====================";
  std::string s =  result.data();
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

  qDebug() << k_funcinfo << "parsing new dot";
  bool parsingResult = parse(s);
  delete phelper;
  phelper = 0;
  qDebug() << k_funcinfo << "phelper deleted";

  if (parsingResult)
  {
    updateWith(newGraph);
  }
//   return parsingResult;
  if (m_readWrite && m_phase == Initial)
  {
    m_phase = Final;
    update();
  }
  else
  {
    emit(readyToDisplay());
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
  kDebug() << k_funcinfo << m_width << m_height << endl;
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
      int cellNum = cellNumber(int(gn->x()), int(gn->y()));
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
  kDebug() << k_funcinfo << "m_wdhcf=" << m_wdhcf << "; m_hdvcf=" << m_hdvcf << endl;
  kDebug() << k_funcinfo << "finished" << endl;
}

QSet< GraphNode* >& DotGraph::nodesOfCell(unsigned int id)
{
  return m_cells[id];
}

void DotGraph::storeOriginalAttributes()
{
  foreach (GraphNode* node, nodes().values())
  {
    node->storeOriginalAttributes();
  }
  foreach (GraphEdge* edge, edges().values())
  {
    edge->storeOriginalAttributes();
  }
  foreach (GraphSubgraph* subgraph, subgraphs().values())
  {
    subgraph->storeOriginalAttributes();
  }
  GraphElement::storeOriginalAttributes();
}

void DotGraph::saveTo(const QString& fileName)
{
  kDebug() << k_funcinfo << fileName;
  m_dotFileName = fileName;
  GraphExporter exporter;
  exporter.writeDot(this, fileName);
}

void DotGraph::updateWith(const DotGraph& newGraph)
{
  kDebug();
  GraphElement::updateWith(newGraph);
  m_width=newGraph.width();
  m_height=newGraph.height();
  m_scale=newGraph.scale();
  m_directed=newGraph.directed();
  m_strict=newGraph.strict();
  computeCells();
  foreach (GraphNode* ngn, newGraph.nodes())
  {
    qDebug() << k_funcinfo << "node " << ngn->id();
    if (nodes().contains(ngn->id()))
    {
      qDebug() << k_funcinfo << "known";
      nodes()[ngn->id()]->updateWith(*ngn);
    }
    else
    {
      qDebug() << k_funcinfo << "new";
      GraphNode* newgn = new GraphNode(*ngn);
      qDebug() << k_funcinfo << "new created";
      nodes().insert(ngn->id(), newgn);
      qDebug() << k_funcinfo << "new inserted";
    }
  }
  foreach (GraphEdge* nge, newGraph.edges())
  {
    qDebug() << k_funcinfo << "an edge";
    QPair<GraphNode*,GraphNode*> pair(nodes()[nge->fromNode()->id()],nodes()[nge->toNode()->id()]);
    if (edges().contains(pair))
    {
      edges().value(pair)->updateWith(*nge);
    }
    else
    {
      GraphEdge* newEdge = new GraphEdge();
      newEdge->updateWith(*nge);
      newEdge->setFromNode(nodes()[nge->fromNode()->id()]);
      newEdge->setToNode(nodes()[nge->toNode()->id()]);
      edges().insert(pair, newEdge);
    }
  }
  computeCells();
}

#include "dotgraph.moc"
