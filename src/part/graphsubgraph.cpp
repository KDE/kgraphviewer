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

//
// GraphSubgraph
//

GraphSubgraph::GraphSubgraph() :
    GraphElement(), m_content()
{
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
        dynamic_cast<GraphNode*>(ge)->updateWithNode(*dynamic_cast<GraphNode*>(updatingge));
        //     kDebug() << "node " << ngn->id();
        break;
      }
    }
    if (!found)
    {
  //       kDebug() << "new";
      GraphNode* newgn = new GraphNode(*dynamic_cast<GraphNode*>(updatingge));
  //       kDebug() << "new created";
      content().push_back(newgn);
  //       kDebug() << "new inserted";
    }
  }

  if (canvasSubgraph())
  {
    canvasSubgraph()->modelChanged();
    canvasSubgraph()->computeBoundingRect();
  }
//   kDebug() << "done";
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
