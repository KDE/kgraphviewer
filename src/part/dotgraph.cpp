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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "dotgraph.h"
#include "dotgrammar.h"
#include "graphexporter.h"
#include "DotGraphParsingHelper.h"


#include <iostream>
#include <math.h>
#include "fdstream.hpp"
#include <boost/spirit/utility/confix.hpp>

#include <kdebug.h>
#include <QFile>
#include <QPair>
#include <QByteArray>



using namespace boost;
using namespace boost::spirit;

extern DotGraphParsingHelper* phelper;

const distinct_parser<> keyword_p("0-9a-zA-Z_");

DotGraph::DotGraph(const QString& command, const QString& fileName) :
  GraphElement(),
  m_dotFileName(fileName),m_width(0.0), m_height(0.0),m_scale(1.0),
  m_directed(true),m_strict(false),
  m_layoutCommand(command)
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
  if (m_layoutCommand == "")
  {
    QFile iFILE(str);
    
    if (!iFILE.open(QIODevice::ReadOnly))
    {
      kError() << "Can't test dot file. Will try to use the dot command on the file: '" << str << "'" << endl;
      return "dot -Txdot";
    }
    
    QByteArray fileContent = iFILE.readAll();
    if (fileContent.isEmpty()) return "";
    std::string s =  fileContent.data();
    std::string cmd = "dot";
    parse(s.c_str(),
          (
            !(keyword_p("strict")) >> (keyword_p("graph")[assign_a(cmd,"neato")])
          ), (space_p|comment_p("/*", "*/")) );
    
    m_layoutCommand = QString::fromStdString(cmd) + " -Txdot" ;
  }
//   std::cerr << "File " << str << " will be layouted by '" << m_layoutCommand << "'" << std::endl;
  return m_layoutCommand;
}

bool DotGraph::parseDot(const QString& str)
{
  kDebug() << k_funcinfo << str << endl;
  QString popencmd;
  if (m_layoutCommand.isEmpty())
  {
    return false;
  }
//   std::cerr << "Building popencmd" << std::endl;
  popencmd = QString("%1 %2 2>/dev/null").arg(m_layoutCommand).arg(str);
  
//   kDebug() << "Running '" << popencmd << "'..." << endl;
  
  FILE* file = popen(popencmd.ascii(), "r");
  if (file == 0) 
  {
    kError() << "Can't run dot!" << endl;
    return false;
  }

  std::ostringstream oss;
  std::string line;
  boost::fdistream myfile(fileno(file));
 
  while (! myfile.eof() )
  {
    getline (myfile,line);
    if (*line.rbegin() != '\\')
    {
      oss << line;
      oss << std::endl;
    }
    else
    {
      line.resize(line.size()-1);
      oss << line;
    }
  }
 
  std::string s =  oss.str();
//   std::cerr << "std::string content is:" << std::endl << "'" << s << "'" << std::endl;
  if (phelper != 0)
  {
    phelper->graph = 0;
    delete phelper;
  }
  phelper = new DotGraphParsingHelper;
  phelper->graph = this;
  phelper->z = 1;
  phelper->maxZ = 1;
  phelper->uniq = 0;
  
  bool parsingResult = parse(s);
  kDebug() << k_funcinfo << "parsed" << endl;
  if (parsingResult)
  {
    computeCells();
  }
  delete phelper;
  phelper = 0;
  kDebug() << k_funcinfo << "return parsing result" << endl;
  return parsingResult;
}

bool DotGraph::update()
{
  kDebug() << k_funcinfo << endl;
  GraphExporter exporter;
  QString str = exporter.writeDot(this);

  kDebug() << k_funcinfo << "wrote to " << str << endl;
  QString popencmd;
  if (m_layoutCommand.isEmpty())
  {
    return false;
  }
  //   std::cerr << "Building popencmd" << std::endl;
  popencmd = QString("%1 %2 2>/dev/null").arg(m_layoutCommand).arg(str);
  
  //   kDebug() << "Running '" << popencmd << "'..." << endl;
  
  FILE* file = popen(popencmd.ascii(), "r");
  if (file == 0)
  {
    kError() << "Can't run dot!" << endl;
    return false;
  }
  
  std::ostringstream oss;
  std::string line;
  boost::fdistream myfile(fileno(file));
  
  while (! myfile.eof() )
  {
    getline (myfile,line);
    if (*line.rbegin() != '\\')
    {
      oss << line;
      oss << std::endl;
    }
    else
    {
      line.resize(line.size()-1);
      oss << line;
    }
  }
  
  std::string s =  oss.str();
  //   std::cerr << "std::string content is:" << std::endl << "'" << s << "'" << std::endl;
  if (phelper != 0)
  {
    phelper->graph = 0;
    delete phelper;
  }
  DotGraph newGraph(m_layoutCommand, m_dotFileName);
  phelper = new DotGraphParsingHelper;
  phelper->graph = &newGraph;
  phelper->z = 1;
  phelper->maxZ = 1;
  phelper->uniq = 0;
  
  kDebug() << k_funcinfo << "parsing new dot" << endl;
  bool parsingResult = parse(s);
  if (parsingResult)
  {
    computeCells();
  }
  delete phelper;
  phelper = 0;
  if (parsingResult)
  {
    foreach (GraphNode* ngn, newGraph.nodes())
    {
      if (nodes().contains(ngn->id()))
      {
        nodes()[ngn->id()]->updateWith(*ngn);
      }
      else
      {
        nodes().insert(ngn->id(), new GraphNode(*ngn));
      }
    }
    foreach (GraphEdge* nge, newGraph.edges())
    {
      QPair<GraphNode*,GraphNode*> pair(nodes()[nge->fromNode()->id()],nodes()[nge->toNode()->id()]);
      if (edges().contains(pair))
      {
        edges().value(pair)->updateWith(*nge);
      }
      else
      {
        edges().insert(pair, new GraphEdge(*nge));
      }
    }
  }
  return parsingResult;
}

unsigned int DotGraph::cellNumber(int x, int y)
{
/*  kDebug() << "x= " << x << ", y= " << y << ", m_width= " << m_width << ", m_height= " << m_height << ", m_horizCellFactor= " << m_horizCellFactor << ", m_vertCellFactor= " << m_vertCellFactor  << ", m_wdhcf= " << m_wdhcf << ", m_hdvcf= " << m_hdvcf << endl;*/
  
  unsigned int nx = (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf);
  unsigned int ny = (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf);
/*  kDebug() << "nx = " << (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf) << endl;
  kDebug() << "ny = " << (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf) << endl;
  kDebug() << "res = " << ny * m_horizCellFactor + nx << endl;*/
  
  unsigned int res = ny * m_horizCellFactor + nx;
  return res;
}

#define MAXCELLWEIGHT 800

void DotGraph::computeCells()
{
  kDebug() << k_funcinfo << endl;
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
      kDebug() << "Found cell number " << cellNum << endl;

      if (m_cells.size() <= cellNum)
      {
        m_cells.resize(cellNum+1);
      }
      m_cells[cellNum].insert(gn);
      
      kDebug() << "after insert" << endl;
      if ( m_cells[cellNum].size() > MAXCELLWEIGHT )
      {
        kDebug() << "cell number " << cellNum  << " contains " << m_cells[cellNum].size() << " nodes" << endl;
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
        kDebug() << "cell factor is now " << m_horizCellFactor << " / " << m_vertCellFactor << endl;
        stop = false;
        break;
      }
    }
  } while (!stop);
  kDebug() << k_funcinfo << "finished" << endl;
}

QSet< GraphNode* >& DotGraph::nodesOfCell(unsigned int id)
{
  return m_cells[id];
}
