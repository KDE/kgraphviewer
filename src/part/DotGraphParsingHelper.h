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

#ifndef DOT_GRAPHPARSINGHELPER_H
#define DOT_GRAPHPARSINGHELPER_H

#include <map>
#include <list>
#include <string>

class DotGraph;
class GraphSubgraph;
class GraphNode;
class GraphEdge;
class GraphElement;

struct DotGraphParsingHelper
{
  typedef std::map< std::string, std::string > AttributesMap;

  void createnode(const std::string& nodeid);
  void createsubgraph();
  void setgraphattributes();
  void setsubgraphattributes();
      void setnodeattributes();
  void setedgeattributes();
  void setattributedlist();
  void createedges();
  void edgebound(const std::string& bound) {edgebounds.push_back(bound);}
  void finalactions();
  void setgraphelementattributes(GraphElement* ge, const AttributesMap& attributes);

  std::string attrid;
  std::string valid;
  std::string attributed;
  std::string subgraphid;
  
  uint uniq;
  
  AttributesMap attributes;
  AttributesMap graphAttributes;
  AttributesMap nodesAttributes;
  AttributesMap edgesAttributes;
  std::list< AttributesMap > graphAttributesStack;
  std::list< AttributesMap > nodesAttributesStack;
  std::list< AttributesMap > edgesAttributesStack;
  
  std::list< std::string > edgebounds;
  
  unsigned int z;
  unsigned int maxZ;
  
  DotGraph* graph;
  
  GraphSubgraph* gs;
  GraphNode* gn;
  GraphEdge* ge;
};

#endif



