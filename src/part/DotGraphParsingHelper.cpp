/* This file is part of KGraphViewer.
   Copyright (C) 2006-2007 Gael de Chalendar <kleag@free.fr>

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
#include "dotdefaults.h"
//#include "graphsubgraph.h"
#include "graphnode.h"
#include "graphedge.h"
#include "DotGraphParsingHelper.h"

#include <boost/throw_exception.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/distinct.hpp>
#include <boost/spirit/utility/loops.hpp>
#include <boost/spirit/utility/confix.hpp>


#include <iostream>

#include <kdebug.h>
    
#include <QFile>


#include <boost/throw_exception.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/distinct.hpp>
#include <boost/spirit/utility/loops.hpp>


using namespace std;

#define KGV_MAX_ITEMS_TO_LOAD std::numeric_limits<size_t>::max()

extern DotGraphParsingHelper* phelper;

void DotGraphParsingHelper::setgraphelementattributes(GraphElement* ge, const AttributesMap& attributes)
{
  AttributesMap::const_iterator it, it_end;
  it = attributes.begin(); it_end = attributes.end();
  for (; it != it_end; it++)
  {
    kDebug() << "    " << QString::fromStdString((*it).first) << "\t=\t'" << QString::fromStdString((*it).second) <<"'";
    if ((*it).first=="label")
    {
      QString label = QString::fromUtf8((*it).first.c_str());
      label.replace("\\n","\n");
      (*ge).attributes()["label"] = label;
    }
    else
    {
      (*ge).attributes()[QString::fromStdString((*it).first)] =
      QString::fromStdString((*it).second);
    }
  }
  
  if (attributes.find("color") == attributes.end())
  {
    ge->color(DOT_DEFAULT_LINECOLOR);
  }
  if (attributes.find("fontcolor") == attributes.end())
  {
    ge->fontColor(DOT_DEFAULT_FONTCOLOR);
  }
  if (attributes.find("fontname") == attributes.end())
  {
    ge->fontName("Times-Roman");
  }
  if (attributes.find("fontsize") == attributes.end())
  {
    ge->fontSize(graph->fontSize());
  }
  
  if (attributes.find("_draw_") != attributes.end())
  {
    parse_renderop((attributes.find("_draw_"))->second, ge->renderOperations());
  }
  if (attributes.find("_ldraw_") != attributes.end())
  {
    parse_renderop(attributes.find("_ldraw_")->second, ge->renderOperations());
  }
  if (attributes.find("_hldraw_") != attributes.end())
  {
    parse_renderop(attributes.find("_hldraw_")->second, ge->renderOperations());
  }
  if (attributes.find("_tldraw_") != attributes.end())
  {
    parse_renderop(attributes.find("_tldraw_")->second, ge->renderOperations());
  }
}

void DotGraphParsingHelper::setgraphattributes()
{
  kDebug() << "Attributes for graph are : ";
  setgraphelementattributes(graph, graphAttributes);
  
  if (graphAttributes.find("bgcolor") == graphAttributes.end())
  {
    graph->backColor(DOT_DEFAULT_BACKCOLOR);
  }
}

void DotGraphParsingHelper::setsubgraphattributes()
{
  kDebug() << "Attributes for subgraph are : ";
  setgraphelementattributes(gs, graphAttributes);
}

void DotGraphParsingHelper::setnodeattributes()
{
  kDebug() << "setnodeattributes with z = " << z;
  
  if (gn == 0)
  {
    return;
  }
  kDebug() << "Attributes for node " << gn->id() << " are : ";
  setgraphelementattributes(gn, nodesAttributes);
}

void DotGraphParsingHelper::setedgeattributes()
{
  kDebug() << "setedgeattributeswith z = " << z;
  
  kDebug() << "Attributes for edge " << ge->fromNode()->id() << "->" << ge->toNode()->id() << " are : ";
  setgraphelementattributes(ge, edgesAttributes);
  
  if (edgesAttributes.find("style") == edgesAttributes.end())
  {
    ge->style(DOT_DEFAULT_EDGE_STYLE);
  }
  if (edgesAttributes.find("_tdraw_") != edgesAttributes.end())
  {
    parse_renderop(edgesAttributes["_tdraw_"], ge->renderOperations());
    DotRenderOpVec::const_iterator it, it_end;
    it = ge->renderOperations().begin(); it_end = ge->renderOperations().end();
    for (; it != it_end; it++)
      ge->arrowheads().push_back(*it);
  }
  if (edgesAttributes.find("_hdraw_") != edgesAttributes.end())
  {
    parse_renderop(edgesAttributes["_hdraw_"], ge->renderOperations());
    DotRenderOpVec::const_iterator it, it_end;
    it = ge->renderOperations().begin(); it_end = ge->renderOperations().end();
    for (; it != it_end; it++)
      ge->arrowheads().push_back(*it);
  }
}

void DotGraphParsingHelper::setattributedlist()
{
  kDebug() << "Setting attributes list for " << QString::fromStdString(attributed);
  if (attributed == "graph") 
  {
    if (attributes.find("bb") != attributes.end())
    {
      std::vector< int > v;
      parse_integers(attributes["bb"].c_str(), v);
      if ( (v.size()>=4) && (graph->width() == 0) &&  (graph->width() == 0) )
      {
        graph->width(v[2]);
        graph->height(v[3]);
      }
    }
    AttributesMap::const_iterator it, it_end;
    it = attributes.begin(); it_end = attributes.end();
    for (; it != it_end; it++)
    {
      kDebug() << "    " << QString::fromStdString((*it).first) << " = " <<  QString::fromStdString((*it).second);
      graphAttributes[(*it).first] = (*it).second;
    }
  }
  else if (attributed == "node")
  {
    AttributesMap::const_iterator it, it_end;
    it = attributes.begin(); it_end = attributes.end();
    for (; it != it_end; it++)
    {
      kDebug() << "    " << QString::fromStdString((*it).first) << " = " <<  QString::fromStdString((*it).second);
      nodesAttributes[(*it).first] = (*it).second;
    }
  }
  else if (attributed == "edge")
  {
    AttributesMap::const_iterator it, it_end;
    it = attributes.begin(); it_end = attributes.end();
    for (; it != it_end; it++)
    {
      kDebug() << "    " << QString::fromStdString((*it).first) << " = " <<  QString::fromStdString((*it).second);
      edgesAttributes[(*it).first] = (*it).second;
    }
  }
  attributes.clear();
}

void DotGraphParsingHelper::createnode(const std::string& nodeid)
{
  kDebug() << k_funcinfo << QString::fromStdString(nodeid);
  if (graph->nodes().find(QString::fromStdString(nodeid)) == graph->nodes().end() && graph->nodes().size() < KGV_MAX_ITEMS_TO_LOAD)
  {
    kDebug() << "Creating a new node";
    gn = new GraphNode();
    gn->id(QString::fromStdString(nodeid)); 
    gn->label(QString::fromStdString(nodeid)); 
    graph->nodes()[QString::fromStdString(nodeid)] = gn;
  }
  else if (graph->nodes().find(QString::fromStdString(nodeid)) != graph->nodes().end())
  {
    kDebug() << "Found existing node";
    gn = *(graph->nodes().find(QString::fromStdString(nodeid)));
  }
  edgebounds.clear();
}

void DotGraphParsingHelper::createsubgraph()
{
  kDebug() << k_funcinfo;
  if (phelper)
  {
    std::string str = phelper->subgraphid;
    if (str.empty())
    {
      std::ostringstream oss;
      oss << "kgv_id_" << phelper->uniq++;
      str = oss.str();
    }
    kDebug() << k_funcinfo << QString::fromStdString(str);
    if (graph->subgraphs().find(QString::fromStdString(str)) == graph->subgraphs().end())
    {
      kDebug() << "Creating a new subgraph";
      gs = new GraphSubgraph();
      gs->id(QString::fromStdString(str)); 
      gs->label(QString::fromStdString(str)); 
      graph->subgraphs()[QString::fromStdString(str)] = gs;
    }
    else
    {
      kDebug() << "Found existing subgraph";
      gs = *(graph->subgraphs().find(QString::fromStdString(str)));
    }
    phelper->subgraphid = "";
  }
}

void DotGraphParsingHelper::createedges()
{
  std::string node1Name, node2Name;
  node1Name = edgebounds.front();
  edgebounds.pop_front();
  while (!edgebounds.empty())
  {
    node2Name = edgebounds.front();
    edgebounds.pop_front();

    if (graph->nodes().size() >= KGV_MAX_ITEMS_TO_LOAD || graph->edges().size() >= KGV_MAX_ITEMS_TO_LOAD)
    {
      return;
    }
    kDebug() << k_funcinfo << QString::fromStdString(node1Name) << ", " << QString::fromStdString(node2Name);
    ge = new GraphEdge();
    if (!graph->nodes().contains(QString::fromStdString(node1Name)))
    {
      GraphNode* gn1 = new GraphNode();
      gn1->id(QString::fromStdString(node1Name));
      graph->nodes()[QString::fromStdString(node1Name)] = gn1;
    }
    if (!graph->nodes().contains(QString::fromStdString(node2Name)))
    {
      GraphNode* gn2 = new GraphNode();
      gn2->id(QString::fromStdString(node2Name));
      graph->nodes()[QString::fromStdString(node2Name)] = gn2;
    }
    ge->setCallerNode(graph->nodes()[QString::fromStdString(node1Name)]);
    ge->setCallingNode(graph->nodes()[QString::fromStdString(node2Name)]);
    graph->edges().insert(
      qMakePair(
        graph->nodes()[QString::fromStdString(node1Name)],graph->nodes()[QString::fromStdString(node2Name)]
        ), ge);

    setedgeattributes();

    node1Name = node2Name;
  }
  edgebounds.clear();
}

void DotGraphParsingHelper::finalactions()
{
  GraphEdgeMap::iterator it, it_end;
  it = graph->edges().begin(); it_end = graph->edges().end();
  for (; it != it_end; it++)
  {
    (*it)->z(maxZ+1);
  }
}
