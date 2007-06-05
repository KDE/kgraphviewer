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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <iostream>
#include <fstream>
#include <math.h>

#include <kdebug.h>
#include <qvaluevector.h>
#include <qfile.h>
#include <qpair.h>


#include "fdstream.hpp"
#include "dotgraph.h"
#include "dotgrammar.h"
#include <boost/spirit/utility/confix.hpp>

using namespace boost;
using namespace boost::spirit;

extern DotGraphParsingHelper* phelper;

const distinct_parser<> keyword_p("0-9a-zA-Z_");

DotGraph::DotGraph(const QString& command, const QString& fileName) : 
  m_dotFileName(fileName),m_width(0.0), m_height(0.0),m_scale(1.0),
  m_directed(true),m_strict(false),
  m_fontName(DOT_DEFAULT_FONTNAME),
  m_fontSize(DOT_DEFAULT_FONTSIZE),
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
    delete (*ite).second;
  }
}

QString DotGraph::chooseLayoutProgramForFile(const QString& str)
{
  if (m_layoutCommand.isEmpty())
  {
    QFile iFILE(str);
    
    if (!iFILE.open(IO_ReadOnly))
    {
      kdError() << "Can't test dot file. Will try to use the dot command" << str << endl;
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
    
    m_layoutCommand = (cmd + " -Txdot").c_str() ;
  }
//   std::cerr << "File " << str << " will be layouted by '" << m_layoutCommand << "'" << std::endl;
  return m_layoutCommand;
}

bool DotGraph::parseDot(const QString& str)
{
//   std::cerr << "parseDot " << str << std::endl;
  QString popencmd;
  if (m_layoutCommand.isEmpty())
  {
    return false;
  }
//   std::cerr << "Building popencmd" << std::endl;
  popencmd = QString("%1 %2 2>/dev/null").arg(m_layoutCommand).arg(str);
  
//   kdDebug() << "Running '" << popencmd << "'..." << endl;
  
  FILE* file = popen(popencmd.ascii(), "r");
  if (file == 0) 
  {
    kdError() << "Can't run dot!" << endl;
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
  phelper->only = 0;
  
  bool parsingResult = DotGraphParsingHelper::parse(s);
  if (parsingResult)
  {
    computeCells();
  }
  delete phelper;
  phelper = 0;
  return parsingResult;
}

unsigned int DotGraph::cellNumber(int x, int y)
{
/*  kdDebug() << "x= " << x << ", y= " << y << ", m_width= " << m_width << ", m_height= " << m_height << ", m_horizCellFactor= " << m_horizCellFactor << ", m_vertCellFactor= " << m_vertCellFactor  << ", m_wdhcf= " << m_wdhcf << ", m_hdvcf= " << m_hdvcf << endl;*/
  
  unsigned int nx = (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf);
  unsigned int ny = (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf);
/*  kdDebug() << "nx = " << (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf) << endl;
  kdDebug() << "ny = " << (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf) << endl;
  kdDebug() << "res = " << ny * m_horizCellFactor + nx << endl;*/
  
  unsigned int res = ny * m_horizCellFactor + nx;
  return res;
}

#define MAXCELLWEIGHT 800

void DotGraph::computeCells()
{
  m_horizCellFactor = m_vertCellFactor = 1;
  m_wdhcf = (int)ceil(((double)m_width) / m_horizCellFactor)+1;
  m_hdvcf = (int)ceil(((double)m_height) / m_vertCellFactor)+1;
  bool stop = true;
  do
  {
    stop = true;
    m_cells.clear();
    m_cells.resize(m_horizCellFactor * m_vertCellFactor);
    
    GraphNodeMap::iterator it, it_end;
    it = m_nodesMap.begin(); it_end = m_nodesMap.end();
    for (; it != it_end; it++)
    {
      GraphNode* gn = *it;
      int cellNum = cellNumber(int(gn->x()), int(gn->y()));
//       kdDebug() << "Found cell number " << cellNum << endl;
      m_cells[cellNum].insert(gn);
      
      if ( m_cells[cellNum].size() > MAXCELLWEIGHT )
      {
//         kdDebug() << "cell number " << cellNum  << " contains " << m_cells[cellNum].size() << " nodes" << endl;
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
//         kdDebug() << "cell factor is now " << m_horizCellFactor << " / " << m_vertCellFactor << endl;
        stop = false;
        break;
      }
    }
  } while (!stop);
}

std::set< GraphNode* >& DotGraph::nodesOfCell(unsigned int id)
{
  return m_cells[id];
}
