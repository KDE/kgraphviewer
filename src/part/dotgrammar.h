/* This file is part of KGraphViewer.
   Copyright (C) 2006 Gaël de Chalendar <kleag@free.fr>

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

/*
 * GraphViz dot Graph parsing grammar implemented with boost Spirit
 */

#ifndef DOT_GRAMMAR_H
#define DOT_GRAMMAR_H

#include <boost/throw_exception.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/distinct.hpp>
#include <boost/spirit/utility/loops.hpp>

#include <map>
#include <list>
#include <string>
#include <sstream>
#include <qstring.h>
#include <qpair.h>
#include <qvaluevector.h>

class DotGraph;
class GraphSubgraph;
class GraphNode;
class GraphEdge;


/** 
  * members are interpreted in function of render operations definitions given at:
 * @URL http://www.graphviz.org/cvs/doc/info/output.html#d:dot 
 */
struct DotRenderOp
{
  std::string renderop;
  std::vector< int > integers;
  std::string str;
};

typedef std::vector< DotRenderOp > DotRenderOpVec;

class DotGraph;
class GraphNode;
class GraphEdge;

void dump(char const* first, char const* last);
void strict(char const* first, char const* last);
void undigraph(char const* first, char const* last);
void digraph(char const* first, char const* last);
void graphid(char const* first, char const* last);
void attrid(char const* first, char const* last);
void subgraphid(char const* first, char const* last);
void valid(char const* first, char const* last);
void addattr(char const* first, char const* last);
void pushAttrListC(char const c);
void popAttrListC(char const c);
void pushAttrList(char const* first, char const* last);
void popAttrList(char const* first, char const* last);
void createsubgraph(char const);
void createnode(char const* first, char const* last);
void setgraphattributes(char const* first, char const* last);
void setsubgraphattributes(char const* first, char const* last);
void setnodeattributes(char const* first, char const* last);
void setattributedlist(char const* first, char const* last);
void checkedgeop(char const* first, char const* last);
void edgebound(char const* first, char const* last);
void createedges(char const* first, char const* last);
void incrz(char const);
void decrz(char const);
void finalactions(char const* first, char const* last);

bool parse_point(char const* str, QPoint& p);
bool parse_real(char const* str, double& d);
bool parse_integers(char const* str, std::vector<int>& v);
bool parse_spline(char const* str, QValueVector< QPair< float, float > >& points);
void init_op();
void valid_op(char const* first, char const* last);
bool parse_renderop(const std::string& str, DotRenderOpVec& arenderopvec);
bool parse_numeric_color(char const* str, QColor& c);

struct DotGraphParsingHelper
{
  static bool parse(const std::string& str);
  
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
    
  std::string attrid;
  std::string valid;
  std::string attributed;
  std::string subgraphid;
  
  uint only;
  
  typedef std::map< std::string, std::string > AttributesMap;
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



