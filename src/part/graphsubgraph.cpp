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

// lib
#include "dotdefaults.h"
#include "dotgrammar.h"
#include "graphnode.h"
#include "canvassubgraph.h"
#include "kgraphviewerlib_debug.h"

namespace KGraphViewer
{
//
// GraphSubgraph
//

GraphSubgraph::GraphSubgraph()
    : GraphElement()
    , m_content()
{
}

GraphSubgraph::GraphSubgraph(graph_t *sg)
    : GraphElement()
    , m_content()
{
    updateWithSubgraph(sg);
}

void GraphSubgraph::updateWithSubgraph(const GraphSubgraph &subgraph)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << id() << subgraph.id();
    GraphElement::updateWithElement(subgraph);

    bool found = false;
    for (GraphElement *updatingge : subgraph.content()) {
        for (GraphElement *ge : content()) {
            if (ge->id() == updatingge->id()) {
                found = true;
                if (dynamic_cast<GraphNode *>(ge)) {
                    dynamic_cast<GraphNode *>(ge)->updateWithNode(*dynamic_cast<GraphNode *>(updatingge));
                    //     qCDebug(KGRAPHVIEWERLIB_LOG) << "node " << ngn->id();
                } else if (dynamic_cast<GraphSubgraph *>(ge)) {
                    dynamic_cast<GraphSubgraph *>(ge)->updateWithSubgraph(*dynamic_cast<GraphSubgraph *>(updatingge));
                } else {
                    qCWarning(KGRAPHVIEWERLIB_LOG) << "Updated element is neither a node nor a subgraph";
                }
                break;
            }
        }
        if (!found) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new";
            if (dynamic_cast<GraphNode *>(updatingge)) {
                GraphNode *newgn = new GraphNode(*dynamic_cast<GraphNode *>(updatingge));
                //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new created";
                content().push_back(newgn);
                //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new inserted";
            } else if (dynamic_cast<GraphSubgraph *>(updatingge)) {
                GraphSubgraph *newsg = new GraphSubgraph(*dynamic_cast<GraphSubgraph *>(updatingge));
                content().push_back(newsg);
            }
        }
    }

    if (canvasSubgraph()) {
        canvasSubgraph()->modelChanged();
        canvasSubgraph()->computeBoundingRect();
    }
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "done";
}

void GraphSubgraph::updateWithSubgraph(graph_t *subgraph)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << agnameof(subgraph);
    m_attributes[QStringLiteral("id")] = QString::fromUtf8(agnameof(subgraph));
    if (GD_label(subgraph))
        m_attributes[QStringLiteral("label")] = QString::fromUtf8(GD_label(subgraph)->text);

    DotRenderOpVec ops;
    // decrease mem peak
    setRenderOperations(ops);

    if (agget(subgraph, (char *)"_draw_")) {
        parse_renderop(agget(subgraph, (char *)"_draw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "_draw_: element renderOperations size is now " << ops.size();
    }
    if (agget(subgraph, (char *)"_ldraw_")) {
        parse_renderop(agget(subgraph, (char *)"_ldraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "_ldraw_: element renderOperations size is now " << ops.size();
    }

    setRenderOperations(ops);

    Agsym_t *attr = agnxtattr(subgraph, AGRAPH, nullptr);
    while (attr) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << agnameof(subgraph) << ":" << attr->name << agxget(subgraph, attr);
        m_attributes[QString::fromUtf8(attr->name)] = QString::fromUtf8(agxget(subgraph, attr));
        attr = agnxtattr(subgraph, AGRAPH, attr);
    }

    for (graph_t *sg = agfstsubg(subgraph); sg; sg = agnxtsubg(sg)) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "subsubgraph:" << agnameof(sg);
        const QString name = QString::fromUtf8(agnameof(sg));
        auto it = subgraphs().find(name);
        if (it != subgraphs().end()) {
            GraphSubgraph *subSubgraph = it.value();
            qCDebug(KGRAPHVIEWERLIB_LOG) << "known subsubgraph";
            // ???
            //       nodes()[ngn->name]->setZ(ngn->z());
            subSubgraph->updateWithSubgraph(sg);
            if (subSubgraph->canvasElement()) {
                //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
            }
        } else {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "new subsubgraph";
            GraphSubgraph *newsg = new GraphSubgraph(sg);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new created";
            subgraphs().insert(name, newsg);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new inserted";
        }
    }
}

QString GraphSubgraph::backColor() const
{
    auto it = m_attributes.find(QStringLiteral("bgcolor"));
    if (it != m_attributes.end()) {
        return *it;
    }
    it = m_attributes.find(QStringLiteral("style"));
    if ((it != m_attributes.end()) && (*it == QLatin1String("filled"))) {
        it = m_attributes.find(QStringLiteral("color"));
        if (it != m_attributes.end()) {
            return *it;
        }
        it = m_attributes.find(QStringLiteral("fillcolor"));
        if (it != m_attributes.end()) {
            return *it;
        }
    }
    return QStringLiteral(DOT_DEFAULT_BACKCOLOR);
}

void GraphSubgraph::removeElement(GraphElement *element)
{
    m_content.removeAll(element);
}

GraphElement *GraphSubgraph::elementNamed(const QString &id)
{
    if (this->id() == id)
        return this;
    for (GraphElement *element : content()) {
        if (element->id() == id) {
            return element;
        } else if (dynamic_cast<GraphSubgraph *>(element)) {
            GraphElement *subgraphElement = dynamic_cast<GraphSubgraph *>(element)->elementNamed(id);
            if (subgraphElement) {
                return subgraphElement;
            }
        }
    }
    return nullptr;
}

bool GraphSubgraph::setElementSelected(GraphElement *element, bool selectValue, bool unselectOthers)
{
    if (element)
        qCDebug(KGRAPHVIEWERLIB_LOG) << element->id() << selectValue << unselectOthers;
    bool res = false;
    if (element == this) {
        if (isSelected() != selectValue) {
            setSelected(selectValue);
            canvasElement()->update();
        }
        res = true;
    } else if (isSelected() && unselectOthers) {
        setSelected(false);
        canvasElement()->update();
    }
    for (GraphElement *el : content()) {
        if (dynamic_cast<GraphSubgraph *>(el)) {
            bool subres = dynamic_cast<GraphSubgraph *>(el)->setElementSelected(element, selectValue, unselectOthers);
            if (!res)
                res = subres;
        } else if (element == el) {
            res = true;
            if (el->isSelected() != selectValue) {
                el->setSelected(selectValue);
                el->canvasElement()->update();
            }
        } else {
            if (unselectOthers && el->isSelected()) {
                el->setSelected(false);
                el->canvasElement()->update();
            }
        }
    }
    return res;
}

void GraphSubgraph::retrieveSelectedElementsIds(QList<QString> selection)
{
    if (isSelected()) {
        selection.push_back(id());
    }
    for (GraphElement *el : content()) {
        if (dynamic_cast<GraphSubgraph *>(el)) {
            dynamic_cast<GraphSubgraph *>(el)->retrieveSelectedElementsIds(selection);
        } else if (el->isSelected()) {
            selection.push_back(el->id());
        }
    }
}

QTextStream &operator<<(QTextStream &s, const GraphSubgraph &sg)
{
    s << "subgraph " << sg.id() << "  {" << Qt::endl << "graph [ " << dynamic_cast<const GraphElement &>(sg) << " ] " << Qt::endl;
    for (const GraphElement *el : sg.content()) {
        s << *(dynamic_cast<const GraphNode *>(el));
    }
    s << "}" << Qt::endl;
    return s;
}

}

#include "moc_graphsubgraph.cpp"
