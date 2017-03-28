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

#include <QDebug>
#include <QLoggingCategory>

static QLoggingCategory debugCategory("org.kde.kgraphviewer");

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
  qCDebug(debugCategory) << id() << subgraph.id();
  GraphElement::updateWithElement(subgraph);

  bool found = false;
  foreach (GraphElement* updatingge, subgraph.content())
  {
    foreach (GraphElement* ge, content())
    {
      if (ge->id() == updatingge->id())
      {
        found = true;
        if (dynamic_cast<GraphNode*>(ge))
        {
          dynamic_cast<GraphNode*>(ge)->updateWithNode(*dynamic_cast<GraphNode*>(updatingge));
        //     kDebug() << "node " << ngn->id();
        }
        else if (dynamic_cast<GraphSubgraph*>(ge))
        {
          dynamic_cast<GraphSubgraph*>(ge)->updateWithSubgraph(*dynamic_cast<GraphSubgraph*>(updatingge));
        }
        else
        {
          qWarning() << "Updated element is neither a node nor a subgraph";
        }
        break;
      }
    }
    if (!found)
    {
  //       kDebug() << "new";
      if (dynamic_cast<GraphNode*>(updatingge))
      {
        GraphNode* newgn = new GraphNode(*dynamic_cast<GraphNode*>(updatingge));
  //       kDebug() << "new created";
        content().push_back(newgn);
  //       kDebug() << "new inserted";
      }
      else if (dynamic_cast<GraphSubgraph*>(updatingge))
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
  qCDebug(debugCategory) << agnameof(subgraph);
  m_attributes["id"] = agnameof(subgraph);
  if (GD_label(subgraph))
    m_attributes["label"] = GD_label(subgraph)->text;
  
  DotRenderOpVec ops;
  // decrease mem peak
  setRenderOperations(ops);

  if (agget(subgraph, (char*)"_draw_"))
  {
    parse_renderop(agget(subgraph, (char*)"_draw_"), ops);
    qCDebug(debugCategory) << "_draw_: element renderOperations size is now " << ops.size();
  }
  if (agget(subgraph, (char*)"_ldraw_"))
  {
    parse_renderop(agget(subgraph, (char*)"_ldraw_"), ops);
    qCDebug(debugCategory) << "_ldraw_: element renderOperations size is now " << ops.size();
  }

  setRenderOperations(ops);

  Agsym_t *attr = agnxtattr(subgraph, AGRAPH, nullptr);
  while(attr)
  {
    qCDebug(debugCategory) << agnameof(subgraph) << ":" << attr->name << agxget(subgraph,attr);
    m_attributes[attr->name] = agxget(subgraph,attr);
    attr = agnxtattr(subgraph, AGRAPH, attr);
  }


  for (graph_t* sg = agfstsubg(subgraph); sg; sg = agnxtsubg(sg))
  {
    qCDebug(debugCategory) << "subsubgraph:" << agnameof(sg);
    if ( subgraphs().contains(agnameof(sg)))
    {
      qCDebug(debugCategory) << "known subsubgraph";
      // ???
      //       nodes()[ngn->name]->setZ(ngn->z());
      subgraphs()[agnameof(sg)]->updateWithSubgraph(sg);
      if (subgraphs()[agnameof(sg)]->canvasElement())
      {
        //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
      }
    }
    else
    {
      qCDebug(debugCategory) << "new subsubgraph";
      GraphSubgraph* newsg = new GraphSubgraph(sg);
      //       kDebug() << "new created";
      subgraphs().insert(agnameof(sg), newsg);
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
      if (subgraphElement)
      {
        return subgraphElement;
      }
    }
  }
  return nullptr;
}

bool GraphSubgraph::setElementSelected(
    GraphElement* element,
    bool selectValue,
    bool unselectOthers)
{
  if (element)
    qCDebug(debugCategory) << element->id() << selectValue << unselectOthers;
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
    if (dynamic_cast<GraphSubgraph*>(el))
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
    if (dynamic_cast<GraphSubgraph*>(el))
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
  s << "subgraph " << sg.id() << "  {" << endl
    << "graph [ "
    << dynamic_cast<const GraphElement&>(sg)
    << " ] " << endl;
  foreach (const GraphElement* el, sg.content())
  {
    s << *(dynamic_cast<const GraphNode*>(el));
  }
  s <<"}"<<endl;
  return s;
}

}
