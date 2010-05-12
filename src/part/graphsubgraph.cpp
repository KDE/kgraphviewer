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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/*
 * Graph Subgraph
 */

#include "graphsubgraph.h"
#include "graphnode.h"
#include "canvassubgraph.h"
#include "dotdefaults.h"

#include <kdebug.h>

namespace KGraphViewer
{
  
//
// GraphSubgraph
//

GraphSubgraph::GraphSubgraph() :
  GraphElement(), m_content()
{
}

GraphSubgraph::GraphSubgraph(graph_t* sg) :
  GraphElement(), m_content()
{
  updateWithSubgraph(sg);
}

void GraphSubgraph::updateWithSubgraph(const GraphSubgraph& subgraph)
{
  kDebug() << id() << subgraph.id();
  GraphElement::updateWithElement(subgraph);

  bool found = false;
  foreach (GraphElement* updatingge, subgraph.content())
  {
    foreach (GraphElement* ge, content())
    {
      if (ge->id() == updatingge->id())
      {
        found = true;
        if (dynamic_cast<GraphNode*>(ge) != 0)
        {
          dynamic_cast<GraphNode*>(ge)->updateWithNode(*dynamic_cast<GraphNode*>(updatingge));
        //     kDebug() << "node " << ngn->id();
        }
        else if (dynamic_cast<GraphSubgraph*>(ge) != 0)
        {
          dynamic_cast<GraphSubgraph*>(ge)->updateWithSubgraph(*dynamic_cast<GraphSubgraph*>(updatingge));
        }
        else
        {
          kError() << "Updated element is neither a node nor a subgraph";
        }
        break;
      }
    }
    if (!found)
    {
  //       kDebug() << "new";
      if (dynamic_cast<GraphNode*>(updatingge) != 0)
      {
        GraphNode* newgn = new GraphNode(*dynamic_cast<GraphNode*>(updatingge));
  //       kDebug() << "new created";
        content().push_back(newgn);
  //       kDebug() << "new inserted";
      }
      else if (dynamic_cast<GraphSubgraph*>(updatingge) != 0)
      {
        GraphSubgraph* newsg = new GraphSubgraph(*dynamic_cast<GraphSubgraph*>(updatingge));
        content().push_back(newsg);
      }
    }
  }

  if (canvasSubgraph())
  {
    canvasSubgraph()->modelChanged();
    canvasSubgraph()->computeBoundingRect();
  }
//   kDebug() << "done";
}

void GraphSubgraph::updateWithSubgraph(graph_t* subgraph)
{
  kDebug() << subgraph->name;
  m_attributes["id"] = subgraph->name;
  if (GD_label(subgraph))
    m_attributes["label"] = GD_label(subgraph)->text;
  
  
  renderOperations().clear();
  if (agget(subgraph, (char*)"_draw_") != NULL)
  {
    parse_renderop(agget(subgraph, (char*)"_draw_"), renderOperations());
    kDebug() << "_draw_: element renderOperations size is now " << renderOperations().size();
  }
  if (agget(subgraph, (char*)"_ldraw_") != NULL)
  {
    parse_renderop(agget(subgraph, (char*)"_ldraw_"), renderOperations());
    kDebug() << "_ldraw_: element renderOperations size is now " << renderOperations().size();
  }
  
  Agsym_t *attr = agfstattr(subgraph);
  while(attr)
  {
    kDebug() << subgraph->name << ":" << attr->name << agxget(subgraph,attr->index);
    m_attributes[attr->name] = agxget(subgraph,attr->index);
    attr = agnxtattr(subgraph,attr);
  }


  for (edge_t* e = agfstout(subgraph->meta_node->graph, subgraph->meta_node); e;
      e = agnxtout(subgraph->meta_node->graph, e))
  {
    graph_t* sg = agusergraph(e->head);
    kDebug() << "subsubgraph:" << sg->name;
    if ( subgraphs().contains(sg->name))
    {
      kDebug() << "known subsubgraph";
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
      kDebug() << "new subsubgraph";
      GraphSubgraph* newsg = new GraphSubgraph(sg);
      //       kDebug() << "new created";
      subgraphs().insert(sg->name, newsg);
      //       kDebug() << "new inserted";
    }
    
  }
}


QString GraphSubgraph::backColor() const
{
  if (m_attributes.find("bgcolor") != m_attributes.end())
  {
    return m_attributes["bgcolor"];
  }
  else if ( (m_attributes.find("style") != m_attributes.end())
    && (m_attributes["style"] == "filled")
    && (m_attributes.find("color") != m_attributes.end()) )
  {
    return m_attributes["color"];
  }
  else if ((m_attributes.find("style") != m_attributes.end())
    && (m_attributes["style"] == "filled")
    && (m_attributes.find("fillcolor") != m_attributes.end()))
  {
    return m_attributes["fillcolor"];
  }
  else
  {
    return DOT_DEFAULT_BACKCOLOR;
  }
}

void GraphSubgraph::removeElement(GraphElement* element)
{
  m_content.removeAll(element);
}

GraphElement* GraphSubgraph::elementNamed(const QString& id)
{
  if (this->id() == id) return this;
  foreach (GraphElement* element, content())
  {
    if (element->id() == id)
    {
      return element;
    }
    else if (dynamic_cast<GraphSubgraph*>(element))
    {
      GraphElement* subgraphElement = dynamic_cast<GraphSubgraph*>(element)->elementNamed(id);
      if (subgraphElement != 0)
      {
        return subgraphElement;
      }
    }
  }
  return 0;
}

bool GraphSubgraph::setElementSelected(
    GraphElement* element,
    bool selectValue,
    bool unselectOthers)
{
  if (element)
    kDebug() << element->id() << selectValue << unselectOthers;
  bool res = false;
  if (element == this)
  {
    if (isSelected() != selectValue)
    {
      setSelected(selectValue);
      canvasElement()->update();
    }
    res = true;
  }
  else if (isSelected() && unselectOthers)
  {
    setSelected(false);
    canvasElement()->update();
  }
  foreach (GraphElement* el, content())
  {
    if (dynamic_cast<GraphSubgraph*>(el) != 0)
    {
      bool subres = dynamic_cast<GraphSubgraph*>(el)->setElementSelected(element, selectValue, unselectOthers);
      if (!res) res = subres;
    }
    else if (element == el)
    {
      res = true;
      if (el->isSelected() != selectValue)
      {
        el->setSelected(selectValue);
        el->canvasElement()->update();
      }
    }
    else 
    {
      if (unselectOthers && el->isSelected())
      {
        el->setSelected(false);
        el->canvasElement()->update();
      }
    }
  }
  return res;
}

void GraphSubgraph::retrieveSelectedElementsIds(QList<QString> selection)
{
  if (isSelected())
  {
    selection.push_back(id());
  }
  foreach (GraphElement* el, content())
  {
    if (dynamic_cast<GraphSubgraph*>(el) != 0)
    {
      dynamic_cast<GraphSubgraph*>(el)->retrieveSelectedElementsIds(selection);
    }
    else  if (el->isSelected())
    {
      selection.push_back(el->id());
    }
  }
}

QTextStream& operator<<(QTextStream& s, const GraphSubgraph& sg)
{
  s << "subgraph " << sg.id() << "  {"
    << dynamic_cast<const GraphElement&>(sg);
  foreach (const GraphElement* el, sg.content())
  {
    s << *(dynamic_cast<const GraphNode*>(el));
  }
  s <<"}"<<endl;
  return s;
}

}
