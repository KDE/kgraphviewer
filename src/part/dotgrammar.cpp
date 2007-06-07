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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <iostream>

#include <kdebug.h>
    
#include <QFile>

#include <boost/spirit/utility/confix.hpp>
#include <boost/throw_exception.hpp> 
namespace boost
{
  void throw_exception(std::exception const & e) {}
}


#include "dotgrammar.h"
#include "dotgraph.h"
#include "dotdefaults.h"
//#include "graphsubgraph.h"
#include "graphnode.h"
#include "graphedge.h"

using namespace std;
using namespace boost;
using namespace boost::spirit;

#define KGV_MAX_ITEMS_TO_LOAD std::numeric_limits<size_t>::max()

DotGraphParsingHelper* phelper = 0;

// keyword_p for C++
// (for basic usage instead of std_p)
const boost::spirit::distinct_parser<> keyword_p("0-9a-zA-Z_");


struct DotGrammar : public boost::spirit::grammar<DotGrammar>
{
  template <typename ScannerT>
      struct definition
  {
    definition<ScannerT>(DotGrammar const& self)
    { 
      graph  = (!(keyword_p("strict")[&strict]) >> (keyword_p("graph")[&undigraph] | keyword_p("digraph")[&digraph]) 
                >> !ID[&graphid] >> ch_p('{') >> !stmt_list >> ch_p('}'))[&finalactions];
      ID = (
          ( ( alpha_p | '_' ) >> *(alnum_p | '_'))
          | real_p
          | ( '"' >>  *( (ch_p('\\') >> '"') | (anychar_p - '"') ) >>  '"' )
          | ( ch_p('<') >>  *( anychar_p  - '<' - '>' | tag ) >>  '>' )
           );
      tag = ('<' >> *( anychar_p  - '>') >> '>');
      stmt_list  =  stmt >> !(ch_p(';')) >> !( stmt_list ) ;
      stmt  =  (
          attr_stmt
          |  edge_stmt
          |  node_stmt
          |  ( ID >> '=' >> ID )
          |  subgraph
               );
      
      attr_stmt  = (
                     (keyword_p("graph")[assign_a(phelper->attributed)] >> attr_list[&setattributedlist])[&setgraphattributes]
                     | (keyword_p("node")[assign_a(phelper->attributed)] >> attr_list[&setattributedlist])
                     | (keyword_p("edge")[assign_a(phelper->attributed)] >> attr_list[&setattributedlist])
                   ) ;
      
      attr_list  = ch_p('[') >> !( a_list ) >> ch_p(']');
      a_list  =  ((ID[&attrid] >> !( '=' >> ID[&valid] ))[&addattr] >> !(',' >> a_list ));
      edge_stmt  =  ( (node_id[&edgebound] | subgraph) >>  edgeRHS >> !( attr_list[assign_a(phelper->attributed,"edge")][&pushAttrList][&setattributedlist] ) )[&createedges][&popAttrList];
      edgeRHS  =  edgeop[&checkedgeop] >> (node_id[&edgebound] | subgraph) >> !( edgeRHS );
      edgeop = str_p("->") | str_p("--");
      node_stmt  = ( node_id[&createnode] >> !( attr_list ) )[assign_a(phelper->attributed,"node")][&pushAttrList][&setattributedlist][&setnodeattributes][&popAttrList];
      node_id  =  (ID >> !( port ));
      port  =  ( ch_p(':') >> ID >> !( ':' >> compass_pt ) )
          |  ( ':' >> compass_pt );
      subgraph  =  ( !( keyword_p("subgraph") >> !( ID ) ) >> ch_p('{')[&createsubgraph][&incrz][&pushAttrListC] >> stmt_list >> ch_p('}') [&decrz][&popAttrListC])
          |  ( keyword_p("subgraph") >> ID);
      compass_pt  =  (keyword_p("n") | keyword_p("ne") | keyword_p("e") 
          | keyword_p("se") | keyword_p("s") | keyword_p("sw") 
          | keyword_p("w") | keyword_p("nw") );
  
    }
    
    boost::spirit::rule<ScannerT> graph, ID, tag, stmt_list, stmt, attr_stmt, 
    attr_list, a_list, edge_stmt, edgeop,
    edgeRHS, node_stmt, node_id, 
    port, subgraph, compass_pt;
        
    boost::spirit::rule<ScannerT> const& start() const 
    { 
      return graph; 
    }
  };
  
};


void incrz(char const first)
{
  if (phelper)
  {
    phelper->z++;
    if (phelper->z > phelper->maxZ)
    {
      phelper->maxZ = phelper->z;
    }
  }
}

void decrz(char const first)
{
  if (phelper)
  {
    phelper->z--;
  }
}

void dump(char const* first, char const* last)
{
  std::string str(first, last);
  kError() << ">>>> " << QString::fromStdString(str) << " <<<<" << endl;
}

void strict(char const* first, char const* last)
{
  if (phelper) phelper->graph->strict(true);
}

void undigraph(char const* first, char const* last)
{
  std::cerr << "Setting graph as undirected" << std::endl;
  if (phelper) phelper->graph->directed(false);
}

void digraph(char const* first, char const* last)
{
   std::cerr << "Setting graph as directed" << std::endl;
  if (phelper) phelper->graph->directed(true);
}

void graphid(char const* first, char const* last)
{
  if (phelper) phelper->graph->id(QString::fromStdString(std::string(first,last)));
}

void attrid(char const* first, char const* last)
{
  if (phelper) 
  {
    std::string id(first,last);
    if (id.size()>0 && id[0] == '"') id = id.substr(1);
    if (id.size()>0 && id[id.size()-1] == '"') id = id.substr(0,id.size()-1);
    phelper->attrid = id;
    phelper->valid = "";
    std::cerr << "Got attr ID  = '"<<phelper->attrid<<"'"<< std::endl;
  }
}

void subgraphid(char const* first, char const* last)
{
  if (phelper) 
  {
    std::string id(first,last);
    if (id.size()>0 && id[0] == '"') id = id.substr(1);
    if (id.size()>0 && id[id.size()-1] == '"') id = id.substr(0,id.size()-1);
    phelper->subgraphid = id;
    std::cerr << "Got attr ID  = '"<<phelper->attrid<<"'"<< std::endl;
  }
}

void valid(char const* first, char const* last)
{
  if (phelper) 
  {
    std::string id(first,last);
    if (id.size()>0 && id[0] == '"') id = id.substr(1);
    if (id.size()>0 && id[id.size()-1] == '"') id = id.substr(0,id.size()-1);
    phelper->valid = id;
    std::cerr << "Got attr val = '"<<phelper->valid<<"'"<< std::endl;
  }
}

void addattr(char const* first, char const* last)
{
  if (phelper) 
  {
    phelper->attributes.insert(std::make_pair(phelper->attrid,phelper->valid));
  }
}

void pushAttrListC(char const c)
{
  pushAttrList(0,0);
}

void pushAttrList(char const* first, char const* last)
{
  std::cerr << "Pushing attributes" << std::endl;
  if (phelper) 
  {
    phelper->graphAttributesStack.push_back(phelper->graphAttributes);
    phelper->nodesAttributesStack.push_back(phelper->nodesAttributes);
    phelper->edgesAttributesStack.push_back(phelper->edgesAttributes);
  }
}

void popAttrListC(char const c)
{
  popAttrList(0,0);
}

void popAttrList(char const* first, char const* last)
{
  std::cerr << "Poping attributes" << std::endl;
  if (phelper) 
  {
    phelper->graphAttributes = phelper->graphAttributesStack.back();
    phelper->graphAttributesStack.pop_back();
    phelper->nodesAttributes = phelper->nodesAttributesStack.back();
    phelper->nodesAttributesStack.pop_back();
    phelper->edgesAttributes = phelper->edgesAttributesStack.back();
    phelper->edgesAttributesStack.pop_back();
  }
}

void createnode(char const* first, char const* last)
{
  if (phelper) 
  {
    std::string id(first,last);
    if (id.size()>0 && id[0] == '"') id = id.substr(1);
    if (id.size()>0 && id[id.size()-1] == '"') id = id.substr(0,id.size()-1);
    phelper->createnode(id);
  }
}

void createsubgraph(char const c)
{
  if (phelper) 
  {
    phelper->createsubgraph();
  }
}

void setgraphattributes(char const* first, char const* last)
{
  std::cerr << "setgraphattributes with z = " << phelper->z << std::endl;
  if (phelper) 
  {
    if (phelper->z == 1) // main graph
    {
      phelper->setgraphattributes();
    }
    else
    {
      phelper->setsubgraphattributes();
    }
  }
}

void setnodeattributes(char const* first, char const* last)
{
  std::cerr << "setnodeattributes with z = " << phelper->z << std::endl;
  if (phelper) 
  {
    phelper->setnodeattributes();
  }
}

void setattributedlist(char const* first, char const* last)
{
  if (phelper) 
  {
    phelper->setattributedlist();
  }
}

void checkedgeop(char const* first, char const* last)
{
  std::string str(first,last);
  if (phelper) 
  {
    if ( ( (phelper->graph->directed()) && (str == "->") ) || 
      ( (!phelper->graph->directed()) && (str == "--") ) )
      return;
    
    kError() << "Error !! uncoherent relation : directed = '" << phelper->graph->directed() << "' and op = '" << QString::fromStdString(str) << "'" << endl;
  }
}

void edgebound(char const* first, char const* last)
{
  std::cerr << "edgebound: " << std::string(first,last) << std::endl;
  if (phelper) 
  {
    std::string id(first,last);
    if (id.size()>0 && id[0] == '"') id = id.substr(1);
    if (id.size()>0 && id[id.size()-1] == '"') id = id.substr(0,id.size()-1);
    phelper->edgebound(id);
  }
}

void createedges(char const* first, char const* last)
{
  if (phelper) 
  {
    phelper->createedges();
  }
}

void finalactions(char const* first, char const* last)
{
  if (phelper) 
  {
    phelper->finalactions();
  }
}

bool parse_point(char const* str, QPoint& p)
{
  int x,y;
  bool res;
  res = parse(str,
              (
                int_p[assign_a(x)] >> ',' >> int_p[assign_a(y)]
              )
              ,
              space_p).full;
  if (!res) return false;
  p = QPoint(x,y);
  return true;
}

bool parse_numeric_color(char const* str, QColor& c)
{
  if (str==0) return false;
  int r,g,b,a;
  bool res;
  uint_parser<unsigned, 16, 2, 2> hex2digits_p;
  res = parse(str,
              (
                ch_p('#') >> 
                hex2digits_p[assign_a(r)] >>  
                hex2digits_p[assign_a(g)] >> 
                hex2digits_p[assign_a(b)] >> 
                !hex2digits_p[assign_a(a)]
              )
              ,
              space_p).full;
  if (res)
  {
    c.setRgb(r,g,b);
    return true;
  }
  
  double h,s,v;
  res = parse(str,
              (
                real_p[assign_a(h)] >> !ch_p(',') >> real_p[assign_a(s)] >> !ch_p(',') >> real_p[assign_a(v)]
              )
              ,
              space_p).full;
  if (res)
  {
    c.setHsv(int(255*h),int(255*s),int(255*v));
    return true;
  }
  return false;
}

bool parse_real(char const* str, double& d)
{
  return parse(str,
               (
                 real_p[assign_a(d)]
               )
               ,
               space_p).full;
}

bool parse_integers(char const* str, std::vector<int>& v)
{
  return parse(str,
               (
                 int_p[push_back_a(v)] >> *(',' >> int_p[push_back_a(v)])
               )
               ,
               space_p).full;
}

bool parse_spline(char const* str, QVector< QPair< float, float > >& points)
{
  std::cerr << "Parsing spline..." << str << std::endl;
  char e = 'n', s = 'n';
  int sx,sy,ex,ey;
  QPair< float, float > p;
  bool res;
  res = parse(str,
              (
                !(ch_p('e')[assign_a(e)] >> ',' >> int_p[assign_a(ex)] >> ',' >> int_p[assign_a(ey)]) 
                >> !(ch_p('s')[assign_a(s)] >> ',' >> int_p[assign_a(sx)] >> ',' >> int_p[assign_a(sy)])
                >> ((int_p[assign_a(p.first)] >> ',' >> int_p[assign_a(p.second)]))[push_back_a(points,p)] 
                >> +(
                      (int_p[assign_a(p.first)] >> ',' >> int_p[assign_a(p.second)])[push_back_a(points,p)] >> 
                      (int_p[assign_a(p.first)] >> ',' >> int_p[assign_a(p.second)])[push_back_a(points,p)] >> 
                      (int_p[assign_a(p.first)] >> ',' >> int_p[assign_a(p.second)])[push_back_a(points,p)]
                    )
              )
              ,
              space_p).full;
  if (!res) return false;
  if (s == 's')
  {
   std::cerr << "inserting the s point" << std::endl;
    points.insert(points.begin(), qMakePair(float(sx),float(sy)));
  }
  if (e == 'e')
  {
//     points.insert(points.begin(), qMakePair(float(ex),float(ey)));
  }
  return true;
}

DotRenderOp renderop ;

DotRenderOpVec* renderopvec = 0;

void init_op()
{
  renderop = DotRenderOp();
}

void valid_op(char const* first, char const* last)
{
  std::string s(first, last);
  std::cerr << "Validating render operation "<<s<<": '"<<renderop.renderop<<"/"<<renderop.str<<"'" << std::endl;
  renderopvec->push_back(renderop);
  renderop.renderop = "";
  renderop.integers = std::vector<int>();
  renderop.str= "";
}

bool parse_renderop(const std::string& str, DotRenderOpVec& arenderopvec)
{
  if (str == "")
  {
    return false;
  }
  // T 1537 228 0 40 9 -#1 (== 0) T 1537 217 0 90 19 -MAIN:./main/main.pl 
  init_op();
  renderopvec = &arenderopvec;
  bool res;
  int c;
  res = parse(str.c_str(),
              (
                +(
                   (
                     (ch_p('E')|ch_p('e'))[assign_a(renderop.renderop)] >> space_p >> 
                     repeat_p(4)[int_p[push_back_a(renderop.integers)] >> space_p]
                   )[&valid_op] 
                   | (
                       (ch_p('P')|ch_p('p')|ch_p('L')|ch_p('B')|ch_p('b'))[assign_a(renderop.renderop)] >> space_p >> 
                       int_p[assign_a(c)][push_back_a(renderop.integers)] >> space_p >> 
                       repeat_p(boost::ref(c))[
                                                int_p[push_back_a(renderop.integers)] >> space_p >> 
                                                int_p[push_back_a(renderop.integers)] >> space_p
                                              ] 
                     )[&valid_op]
  // "T 1537 228 0 40 9 -#1 (== 0) T 1537 217 0 90 19 -MAIN:./main/main.pl "
                   | (
                       ch_p('T')[assign_a(renderop.renderop)] >> space_p >> 
                       int_p[push_back_a(renderop.integers)] >> space_p >> 
                       int_p[push_back_a(renderop.integers)] >> space_p >> 
                       int_p[push_back_a(renderop.integers)] >> space_p >> 
                       int_p[push_back_a(renderop.integers)] >> space_p >> 
                       int_p[assign_a(c)] >> space_p >> '-' >> 
                       (repeat_p(boost::ref(c))[anychar_p])[assign_a(renderop.str)] >> space_p
                     )[&valid_op]
                   | (
                       (ch_p('C')|ch_p('c')|ch_p('S'))[assign_a(renderop.renderop)] >> space_p >> 
                       int_p[assign_a(c)] >> space_p >> '-' >> 
                       (repeat_p(boost::ref(c))[anychar_p])[assign_a(renderop.str)] >> space_p
                     )[&valid_op] 
                   | ( 
                       ch_p('F')[assign_a(renderop.renderop)] >> space_p >> 
                       real_p[push_back_a(renderop.integers)] >> space_p >> 
                       int_p[assign_a(c)] >> space_p >> '-' >> 
                       (repeat_p(boost::ref(c))[anychar_p])[assign_a(renderop.str)] >> space_p
                     )[&valid_op]
                 )
              )
             ).full;
  if (res ==false)
  {
    std::cerr << "Parsing render operation '"<<str<<"'" << std::endl;
    kError() << "parse_renderop failed on '"<<QString::fromStdString(str)<<"'. Last renderop string is '"<<QString::fromUtf8(renderop.str.c_str())<<"'" << endl;
  }
//   delete renderop; renderop = 0;
  
  return res;
}

// void boost::throw_exception(std::exception const & e)
// {
  //  throw e;
// }

void DotGraphParsingHelper::setgraphattributes()
{
/*  pushAttrList('a');
  attributed = "graph";
  setattributedlist();*/
  
  std::cerr << "Attributes for graph are : "  << std::endl;
  AttributesMap::const_iterator it, it_end;
  it = graphAttributes.begin(); it_end = graphAttributes.end();
  for (; it != it_end; it++)
  {
    std::cerr << "    " << (*it).first << "\t=\t'" << (*it).second <<"'"<< std::endl;
  }
  
  if (graphAttributes.find("color") != graphAttributes.end())
  {
    graph->color(QString::fromStdString(graphAttributes["color"]));
  }
  else
  {
    graph->color(DOT_DEFAULT_LINECOLOR);
  }
  if (graphAttributes.find("bgcolor") != graphAttributes.end())
  {
    graph->backgroundColor(QString::fromStdString(graphAttributes["bgcolor"]));
  }
  else 
  {
    graph->backgroundColor(DOT_DEFAULT_BACKCOLOR);
  }
  if (graphAttributes.find("fontcolor") != graphAttributes.end())
  {
    graph->fontColor(QString::fromStdString(graphAttributes["fontcolor"]));
  }
  else
  {
    graph->fontColor(DOT_DEFAULT_FONTCOLOR);
  }
  if (graphAttributes.find("label") != graphAttributes.end())
  {
    QString label = QString::fromUtf8(graphAttributes["label"].c_str());
    label.replace("\\n","\n");
    graph->label(label);
  }
  else
  {
    graph->label("");
  }
  if (graphAttributes.find("fontname") != graphAttributes.end())
  {
    graph->fontName(QString::fromStdString(graphAttributes["fontname"]));
  }
  else
  {
    graph->fontName("Times-Roman");
  }
  graph->fontSize(14);
  if (graphAttributes.find("fontsize") != graphAttributes.end())
  {
    unsigned int fontSize;
    bool res = boost::spirit::parse(graphAttributes["fontsize"].c_str(),
                                    (int_p[assign_a(fontSize)])
                                    ,
                                    space_p).full;
    if (res)
    {
      graph->fontSize(fontSize);
    }
  }
  if (graphAttributes.find("_draw_") != graphAttributes.end())
  {
    parse_renderop(graphAttributes["_draw_"], graph->renderOperations());
  }
  if (graphAttributes.find("_ldraw_") != graphAttributes.end())
  {
  std::cerr << "Calling parse of _ldraw_: '"<<graphAttributes["_ldraw_"]<<"'" << std::endl;
    parse_renderop(graphAttributes["_ldraw_"], graph->renderOperations());
  }
//   popAttrList('a');
}

void DotGraphParsingHelper::setsubgraphattributes()
{
/*  pushAttrList('a');
  attributed = "graph";
  setattributedlist();*/
  
  std::cerr << "Attributes for graph are : "  << std::endl;
  AttributesMap::const_iterator it, it_end;
  it = graphAttributes.begin(); it_end = graphAttributes.end();
  for (; it != it_end; it++)
  {
     std::cerr << "    " << (*it).first << "\t=\t'" << (*it).second <<"'"<< std::endl;
  }
  
  if (graphAttributes.find("style") != graphAttributes.end())
  {
    gs->style(QString::fromStdString(graphAttributes["style"]));
  }
  if (graphAttributes.find("color") != graphAttributes.end())
  {
    gs->lineColor(QString::fromStdString(graphAttributes["color"]));
//     gs->backColor(graphAttributes["color"]);
  }
  else
  {
    gs->lineColor(DOT_DEFAULT_LINECOLOR);

  }
  if (graphAttributes.find("bgcolor") != graphAttributes.end())
  {
    gs->backColor(QString::fromStdString(graphAttributes["bgcolor"]));
  }
  else if ((graphAttributes.find("style") != graphAttributes.end())
      && (graphAttributes["style"] == "filled") 
      && (graphAttributes.find("color") != graphAttributes.end()))
  {
    gs->backColor(QString::fromStdString(graphAttributes["color"]));
  }
  else
  {
    gs->backColor(DOT_DEFAULT_BACKCOLOR);
  }
  if ((graphAttributes.find("style") != graphAttributes.end())
      && (graphAttributes["style"] == "filled") 
      && (graphAttributes.find("fillcolor") != graphAttributes.end()))
  {
    gs->backColor(QString::fromStdString(graphAttributes["fillcolor"]));
  }  
  if (graphAttributes.find("fontcolor") != graphAttributes.end())
  {
    gs->fontColor(QString::fromStdString(graphAttributes["fontcolor"]));
  }
  else
  {
    gs->fontColor(DOT_DEFAULT_FONTCOLOR);
  }
  if (graphAttributes.find("label") != graphAttributes.end())
  {
    QString label = QString::fromUtf8(graphAttributes["label"].c_str());
    label.replace("\\n","\n");
    gs->label(label);
  }
  else
  {
    gs->label("");
  }
  if (graphAttributes.find("fontname") != graphAttributes.end())
  {
    gs->fontName(QString::fromStdString(graphAttributes["fontname"]));
  }
  else
  {
    gs->fontName("Times-Roman");
  }
  gs->fontSize(14);
  if (graphAttributes.find("fontsize") != graphAttributes.end())
  {
    unsigned int fontSize;
    bool res = boost::spirit::parse(graphAttributes["fontsize"].c_str(),
                                    (int_p[assign_a(fontSize)])
                                        ,
                                    space_p).full;
    if (res)
    {
      gs->fontSize(fontSize);
    }
  }
  if (graphAttributes.find("_draw_") != graphAttributes.end())
  {
    parse_renderop(graphAttributes["_draw_"], gs->renderOperations());
  }
  if (graphAttributes.find("_ldraw_") != graphAttributes.end())
  {
  std::cerr << "Calling parse of _ldraw_: '"<<graphAttributes["_ldraw_"]<<"'" << std::endl;
    parse_renderop(graphAttributes["_ldraw_"], gs->renderOperations());
  }
//   popAttrList('a');
}

void DotGraphParsingHelper::setnodeattributes()
{
  std::cerr << "setnodeattributes with z = " << z << std::endl;
/*  pushAttrList('a');
  attributed = "node";
  setattributedlist();*/
  
  if (gn == 0)
  {
    return;
  }
  std::cerr << "Attributes for node " << gn->id().toStdString() << " are : "  << std::endl;
  AttributesMap::const_iterator it, it_end;
  it = nodesAttributes.begin(); it_end = nodesAttributes.end();
  for (; it != it_end; it++)
  {
    std::cerr << "    " << (*it).first << "\t=\t'" << (*it).second <<"'"<< std::endl;
  }
  
  gn->z(z);
  if (nodesAttributes.find("style") != nodesAttributes.end())
  {
    gn->style(QString::fromStdString(nodesAttributes["style"]));
  }
  if (nodesAttributes.find("shape") != nodesAttributes.end())
  {
    gn->shape(QString::fromStdString(nodesAttributes["shape"]));
  }
  if (nodesAttributes.find("color") != nodesAttributes.end())
  {
    gn->lineColor(QString::fromStdString(nodesAttributes["color"]));
  }
  if (nodesAttributes.find("fontcolor") != nodesAttributes.end())
  {
    gn->fontColor(QString::fromStdString(nodesAttributes["fontcolor"]));
  }
  if (nodesAttributes.find("fillcolor") != nodesAttributes.end())
  {
    gn->backColor(QString::fromStdString(nodesAttributes["fillcolor"]));
  }
  else if ( (nodesAttributes.find("color") != nodesAttributes.end())
          && (gn->style() == "filled") )
  {
    gn->backColor(QString::fromStdString(nodesAttributes["color"]));
  }
  else
  {
    gn->backColor(DOT_DEFAULT_NODE_BACKCOLOR);
  }
  if (nodesAttributes.find("label") != nodesAttributes.end())
  {
    QString label = QString::fromUtf8(nodesAttributes["label"].c_str());
    label.replace("\\n","\n");
    gn->label(label);
  }
  if (nodesAttributes.find("fontname") != nodesAttributes.end())
  {
    gn->fontName(QString::fromStdString(nodesAttributes["fontname"]));
  }
  else
  {
    gn->fontName(graph->fontName());
  }
  if (nodesAttributes.find("fontsize") != nodesAttributes.end())
  {
    unsigned int fontSize;
    bool res = boost::spirit::parse(nodesAttributes["fontsize"].c_str(),
                                    (
                                      int_p[assign_a(fontSize)]
                                    )
                                    ,
                                    space_p).full;
    if (res)
    {
      gn->fontSize(fontSize);
    }
    else
    {
      gn->fontSize(graph->fontSize());
    }
  }
  else
  {
    gn->fontSize(graph->fontSize());
  }
  if (nodesAttributes.find("pos") != nodesAttributes.end())
  {
    std::cerr << "node position: '" << nodesAttributes["pos"] << "'" << std::endl;
    std::string pos= nodesAttributes["pos"];
    QPoint p;
    parse_point(pos.c_str(), p);
    gn->x(p.x());
    gn->y(p.y());
  }
  if (nodesAttributes.find("width") != nodesAttributes.end())
  {
     std::cerr << "node width: '" << nodesAttributes["width"] << "'" << std::endl;
    std::string width= nodesAttributes["width"];
    double w;
    parse_real(width.c_str(), w);
    gn->w(DOT_DEFAULT_DPI*w);
  }
  if (nodesAttributes.find("height") != nodesAttributes.end())
  {
     std::cerr << "node height: '" << nodesAttributes["height"] << "'" << std::endl;
    std::string height = nodesAttributes["height"];
    double h;
    parse_real(height.c_str(), h);
    gn->h(DOT_DEFAULT_DPI*h);
  }
  if (nodesAttributes.find("shapefile") != nodesAttributes.end())
  {
     std::cerr << "node height: '" << nodesAttributes["height"] << "'" << std::endl;
    gn->shapeFile(QString::fromStdString(nodesAttributes["shapefile"]));
  }
  if (nodesAttributes.find("URL") != nodesAttributes.end())
  {
    gn->url(QString::fromStdString(nodesAttributes["URL"]));
  }
  if (nodesAttributes.find("_draw_") != nodesAttributes.end())
  {
    parse_renderop(nodesAttributes["_draw_"], gn->renderOperations());
  }
  if (nodesAttributes.find("_ldraw_") != nodesAttributes.end())
  {
  std::cerr << "Calling parse of _ldraw_: '"<<nodesAttributes["_ldraw_"]<<"'" << std::endl;
    parse_renderop(nodesAttributes["_ldraw_"], gn->renderOperations());
  }
//   popAttrList('a');
}

void DotGraphParsingHelper::setedgeattributes()
{
  std::cerr << "setedgeattributeswith z = " << z << std::endl;
/*  pushAttrList('a');
  attributed = "edge";
  setattributedlist();*/
  
  std::cerr << "Attributes for edge " << ge->fromNode()->id().toStdString() << "->" << ge->toNode()->id().toStdString() << " are : "  << std::endl;
  AttributesMap::const_iterator it, it_end;
  it = edgesAttributes.begin(); it_end = edgesAttributes.end();
  for (; it != it_end; it++)
  {
    std::cerr << "    " << (*it).first << "\t=\t'" << (*it).second << "'" << std::endl;
  }
  
  ge->z(z);
  if (edgesAttributes.find("label") != edgesAttributes.end())
  {
    QString label = QString::fromUtf8(edgesAttributes["label"].c_str());
    label.replace("\\n","\n");
    ge->label(label);
  }
  if (edgesAttributes.find("fontname") != edgesAttributes.end())
  {
    ge->fontName(QString::fromStdString(edgesAttributes["fontname"]));
  }
  else
  {
    ge->fontName(graph->fontName());
  }
  if (edgesAttributes.find("fontsize") != edgesAttributes.end())
  {
    unsigned int fontSize;
    bool res = boost::spirit::parse(edgesAttributes["fontsize"].c_str(),
                                    (
                                        int_p[assign_a(fontSize)]
                                    )
                                        ,
                                    space_p).full;
    if (res)
    {
      ge->fontSize(fontSize);
    }
    else
    {
      ge->fontSize(graph->fontSize());
    }
  }
  else
  {
    ge->fontSize(graph->fontSize());
  }
  if (edgesAttributes.find("fontcolor") != edgesAttributes.end())
  {
    ge->fontColor(QString::fromStdString(edgesAttributes["fontcolor"]));
  }
  else
  {
    ge->fontColor(DOT_DEFAULT_FONTCOLOR);
  }
  
  if (edgesAttributes.find("color") != edgesAttributes.end())
  {
    ge->colors(QString::fromStdString(edgesAttributes["color"]));
  }
  if (edgesAttributes.find("style") != edgesAttributes.end())
  {
    std::cerr << "setting edge style to " << edgesAttributes["style"] << std::endl;
    ge->style(QString::fromStdString(edgesAttributes["style"]));
  }
  else
  {
    ge->style(DOT_DEFAULT_EDGE_STYLE);
  }
  if (edgesAttributes.find("dir") != edgesAttributes.end())
  {
    std::string dir = edgesAttributes["dir"];
    if ( (dir == "forward") || (dir == "back") || (dir == "both") || (dir == "none") )
    {
      ge->dir(QString::fromStdString(dir));
    }
    else
    {
      kError() << "Invalid edge dir: " << QString::fromStdString(dir) << endl;
    }
  }
  
  if (edgesAttributes.find("pos") != edgesAttributes.end())
  {
    if (parse_spline(edgesAttributes["pos"].c_str(), ge->edgePoints()) == false)
    {
      std::cerr << "Parsing spline failed ! (" << edgesAttributes["pos"] << ")" << std::endl;
    }
  }
  if (edgesAttributes.find("lp") != edgesAttributes.end())
  {
    QPoint lp;
    parse_point(edgesAttributes["lp"].c_str(), lp);
    ge->labelX(lp.x());
    ge->labelY(lp.y());
  }
  if (edgesAttributes.find("_draw_") != edgesAttributes.end())
  {
    parse_renderop(edgesAttributes["_draw_"], ge->renderOperations());
  }
  if (edgesAttributes.find("_ldraw_") != edgesAttributes.end())
  {
    parse_renderop(edgesAttributes["_ldraw_"], ge->renderOperations());
  }
  if (edgesAttributes.find("_hldraw_") != edgesAttributes.end())
  {
    parse_renderop(edgesAttributes["_hldraw_"], ge->renderOperations());
  }
  if (edgesAttributes.find("_tldraw_") != edgesAttributes.end())
  {
    parse_renderop(edgesAttributes["_tldraw_"], ge->renderOperations());
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
//   popAttrList('a');
}

void DotGraphParsingHelper::setattributedlist()
{
   std::cerr << "Setting attributes list for " << attributed << std::endl;
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
      std::cerr << "    " << (*it).first << " = " <<  (*it).second << std::endl;
      graphAttributes[(*it).first] = (*it).second;
    }
  }
  else if (attributed == "node")
  {
    AttributesMap::const_iterator it, it_end;
    it = attributes.begin(); it_end = attributes.end();
    for (; it != it_end; it++)
    {
      std::cerr << "    " << (*it).first << " = " <<  (*it).second << std::endl;
      nodesAttributes[(*it).first] = (*it).second;
    }
  }
  else if (attributed == "edge")
  {
    AttributesMap::const_iterator it, it_end;
    it = attributes.begin(); it_end = attributes.end();
    for (; it != it_end; it++)
    {
      std::cerr << "    " << (*it).first << " = " <<  (*it).second << std::endl;
      edgesAttributes[(*it).first] = (*it).second;
    }
  }
  attributes.clear();
}

void DotGraphParsingHelper::createnode(const std::string& nodeid)
{
   std::cerr << "DotGraphParsingHelper::createnode " << nodeid << std::endl;
  if (graph->nodes().find(QString::fromStdString(nodeid)) == graph->nodes().end() && graph->nodes().size() < KGV_MAX_ITEMS_TO_LOAD)
  {
    gn = new GraphNode();
    gn->id(QString::fromStdString(nodeid)); 
    gn->label(QString::fromStdString(nodeid)); 
    graph->nodes()[QString::fromStdString(nodeid)] = gn;
  }
  else
  {
    gn = 0;
  }
  edgebounds.clear();
}

void DotGraphParsingHelper::createsubgraph()
{
  if (phelper)
  {
    std::string str = phelper->subgraphid;
    if (str.empty())
    {
      std::ostringstream oss;
      oss << "kgv_id_" << phelper->uniq++;
      str = oss.str();
    }
    std::cerr << "DotGraphParsingHelper::createsubgraph " << str << std::endl;
    if (graph->subgraphs().find(QString::fromStdString(str)) == graph->subgraphs().end())
    {
      gs = new GraphSubgraph();
      gs->id(QString::fromStdString(str)); 
      gs->label(QString::fromStdString(str)); 
      graph->subgraphs()[QString::fromStdString(str)] = gs;
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
       std::cerr << "DotGraphParsingHelper::createedges(" << node1Name << ", " << node2Name << ")" << std::endl;
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
    graph->edges().insert(std::make_pair(qMakePair(graph->nodes()[QString::fromStdString(node1Name)],graph->nodes()[QString::fromStdString(node2Name)]), ge));

    setedgeattributes();

    node1Name = node2Name;
  }
  edgebounds.clear();
}

bool DotGraphParsingHelper::parse(const std::string& str)
{
  DotGrammar g;
  return boost::spirit::parse(str.c_str(), g, (space_p|comment_p("/*", "*/"))).full;
}

void DotGraphParsingHelper::finalactions()
{
  GraphEdgeMap::iterator it, it_end;
  it = graph->edges().begin(); it_end = graph->edges().end();
  for (; it != it_end; it++)
  {
    (*it).second->z(maxZ+1);
  }
}
