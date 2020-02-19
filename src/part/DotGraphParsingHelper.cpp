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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

#include "DotGraphParsingHelper.h"
#include "dotdefaults.h"
#include "dotgrammar.h"
#include "dotgraph.h"
//#include "graphsubgraph.h"
#include "graphedge.h"
#include "graphnode.h"
#include "kgraphviewerlib_debug.h"

#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_distinct.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/throw_exception.hpp>

#include <iostream>

#include <QDebug>

#include <QFile>
#include <QUuid>

using namespace std;

extern KGraphViewer::DotGraphParsingHelper *phelper;

namespace KGraphViewer
{
#define KGV_MAX_ITEMS_TO_LOAD std::numeric_limits<int>::max()

DotGraphParsingHelper::DotGraphParsingHelper()
    : attrid()
    , valid()
    , attributed()
    , subgraphid()
    , uniq(0)
    , attributes()
    , graphAttributes()
    , nodesAttributes()
    , edgesAttributes()
    , graphAttributesStack()
    , nodesAttributesStack()
    , edgesAttributesStack()
    , edgebounds()
    , z(0)
    , maxZ(0)
    , graph(nullptr)
    , gs(nullptr)
    , gn(nullptr)
    , ge(nullptr)
{
}

void DotGraphParsingHelper::setgraphelementattributes(GraphElement *ge, const AttributesMap &attributes)
{
    AttributesMap::const_iterator it, it_end;
    it = attributes.begin();
    it_end = attributes.end();
    for (; it != it_end; it++) {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "    " << QString::fromStdString((*it).first) << "\t=\t'" << QString::fromStdString((*it).second) <<"'";
        if ((*it).first == "label") {
            QString label = QString::fromUtf8((*it).second.c_str());
            label.replace("\\n", "\n");
            (*ge).attributes()["label"] = label;
        } else {
            (*ge).attributes()[QString::fromStdString((*it).first)] = QString::fromStdString((*it).second);
        }
    }

    DotRenderOpVec ops = ge->renderOperations();
    if (attributes.find("_draw_") != attributes.end()) {
        parse_renderop((attributes.find("_draw_"))->second, ops);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ge->renderOperations().size();
    }
    if (attributes.find("_ldraw_") != attributes.end()) {
        parse_renderop(attributes.find("_ldraw_")->second, ops);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ge->renderOperations().size();
    }
    if (attributes.find("_hldraw_") != attributes.end()) {
        parse_renderop(attributes.find("_hldraw_")->second, ops);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ge->renderOperations().size();
    }
    if (attributes.find("_tldraw_") != attributes.end()) {
        parse_renderop(attributes.find("_tldraw_")->second, ops);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "element renderOperations size is now " << ge->renderOperations().size();
    }
    ge->setRenderOperations(ops);
}

void DotGraphParsingHelper::setgraphattributes()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Attributes for graph are : ";
    setgraphelementattributes(graph, graphAttributes);
}

void DotGraphParsingHelper::setsubgraphattributes()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Attributes for subgraph are : ";
    gs->setZ(z);
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "z="<<gs->z();
    setgraphelementattributes(gs, graphAttributes);
}

void DotGraphParsingHelper::setnodeattributes()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "setnodeattributes with z = " << z;

    if (gn == nullptr) {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "gn is null";
        return;
    }
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Attributes for node " << gn->id() << " are : ";
    gn->setZ(z + 1);
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "z="<<gn->z();
    setgraphelementattributes(gn, nodesAttributes);
}

void DotGraphParsingHelper::setedgeattributes()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "setedgeattributeswith z = " << z;

    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Attributes for edge " << ge->fromNode()->id() << "->" << ge->toNode()->id() << " are : ";
    ge->setZ(z + 1);
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "z="<<ge->z();
    setgraphelementattributes(ge, edgesAttributes);

    DotRenderOpVec ops = ge->renderOperations();
    if (edgesAttributes.find("_tdraw_") != edgesAttributes.end()) {
        parse_renderop(edgesAttributes["_tdraw_"], ops);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "edge renderOperations size is now " << ge->renderOperations().size();
        DotRenderOpVec::const_iterator it, it_end;
        it = ops.constBegin();
        it_end = ops.constEnd();
        for (; it != it_end; it++)
            ge->arrowheads().push_back(*it);
    }
    if (edgesAttributes.find("_hdraw_") != edgesAttributes.end()) {
        parse_renderop(edgesAttributes["_hdraw_"], ops);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "edge renderOperations size is now " << ge->renderOperations().size();
        DotRenderOpVec::const_iterator it, it_end;
        it = ops.constBegin();
        it_end = ops.constEnd();
        for (; it != it_end; it++)
            ge->arrowheads().push_back(*it);
    }
    ge->setRenderOperations(ops);
}

void DotGraphParsingHelper::setattributedlist()
{
    // //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Setting attributes list for " << QString::fromStdString(attributed);
    if (attributed == "graph") {
        if (attributes.find("bb") != attributes.end()) {
            std::vector<int> v;
            parse_integers(attributes["bb"].c_str(), v);
            if (v.size() >= 4) {
                //         qCDebug(KGRAPHVIEWERLIB_LOG) << "setting width and height to " << v[2] << v[3];
                graph->width(v[2]);
                graph->height(v[3]);
            }
        }
        AttributesMap::const_iterator it, it_end;
        it = attributes.begin();
        it_end = attributes.end();
        for (; it != it_end; it++) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "    " << QString::fromStdString((*it).first) << " = " <<  QString::fromStdString((*it).second);
            graphAttributes[(*it).first] = (*it).second;
        }
    } else if (attributed == "node") {
        AttributesMap::const_iterator it, it_end;
        it = attributes.begin();
        it_end = attributes.end();
        for (; it != it_end; it++) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "    " << QString::fromStdString((*it).first) << " = " <<  QString::fromStdString((*it).second);
            nodesAttributes[(*it).first] = (*it).second;
        }
    } else if (attributed == "edge") {
        AttributesMap::const_iterator it, it_end;
        it = attributes.begin();
        it_end = attributes.end();
        for (; it != it_end; it++) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "    " << QString::fromStdString((*it).first) << " = " <<  QString::fromStdString((*it).second);
            edgesAttributes[(*it).first] = (*it).second;
        }
    }
    attributes.clear();
}

void DotGraphParsingHelper::createnode(const std::string &nodeid)
{
    QString id = QString::fromStdString(nodeid);
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << id;
    gn = dynamic_cast<GraphNode *>(graph->elementNamed(id));
    if (gn == nullptr && graph->nodes().size() < KGV_MAX_ITEMS_TO_LOAD) {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating a new node" << z << (void*)gs;
        gn = new GraphNode();
        gn->setId(id);
        //     gn->label(QString::fromStdString(nodeid));
        if (z > 0 && gs) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "Adding node" << id << "in subgraph" << gs->id();
            gs->content().push_back(gn);
        } else {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "Adding node" << id;
            graph->nodes()[id] = gn;
        }
    }
    edgebounds.clear();
}

void DotGraphParsingHelper::createsubgraph()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) ;
    if (phelper) {
        std::string str = phelper->subgraphid;
        if (str.empty()) {
            std::ostringstream oss;
            oss << "kgv_id_" << phelper->uniq++;
            str = oss.str();
        }
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << QString::fromStdString(str);
        if (graph->subgraphs().find(QString::fromStdString(str)) == graph->subgraphs().end()) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating a new subgraph";
            gs = new GraphSubgraph();
            gs->setId(QString::fromStdString(str));
            //       gs->label(QString::fromStdString(str));
            graph->subgraphs().insert(QString::fromStdString(str), gs);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "there is now"<<graph->subgraphs().size()<<"subgraphs in" << graph;
        } else {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "Found existing subgraph";
            gs = *(graph->subgraphs().find(QString::fromStdString(str)));
        }
        phelper->subgraphid = "";
    }
}

void DotGraphParsingHelper::createedges()
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG);
    std::string node1Name, node2Name;
    node1Name = edgebounds.front();
    edgebounds.pop_front();
    while (!edgebounds.empty()) {
        node2Name = edgebounds.front();
        edgebounds.pop_front();

        if (graph->nodes().size() >= KGV_MAX_ITEMS_TO_LOAD || graph->edges().size() >= KGV_MAX_ITEMS_TO_LOAD) {
            return;
        }
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << QString::fromStdString(node1Name) << ", " << QString::fromStdString(node2Name);
        ge = new GraphEdge();
        GraphElement *gn1 = graph->elementNamed(QString::fromStdString(node1Name));
        if (gn1 == nullptr) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new node 1";
            gn1 = new GraphNode();
            gn1->setId(QString::fromStdString(node1Name));
            graph->nodes()[QString::fromStdString(node1Name)] = dynamic_cast<GraphNode *>(gn1);
        }
        GraphElement *gn2 = graph->elementNamed(QString::fromStdString(node2Name));
        if (gn2 == nullptr) {
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new node 2";
            gn2 = new GraphNode();
            gn2->setId(QString::fromStdString(node2Name));
            graph->nodes()[QString::fromStdString(node2Name)] = dynamic_cast<GraphNode *>(gn2);
        }
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "Found gn1="<<gn1<<" and gn2=" << gn2;
        if (gn1 == nullptr || gn2 == nullptr) {
            qCWarning(KGRAPHVIEWERLIB_LOG) << "Unable to find or create edge bound(s) gn1=" << gn1 << "; gn2=" << gn2;
        }
        ge->setFromNode(gn1);
        ge->setToNode(gn2);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << ge->fromNode()->id() << " -> " << ge->toNode()->id();
        setedgeattributes();
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << ge->id();
        if (ge->id().isEmpty()) {
            ge->setId(QString::fromStdString(node1Name) + QString::fromStdString(node2Name) + QUuid::createUuid().toString().remove('{').remove('}').remove('-'));
        }
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << ge->id();
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "num before=" << graph->edges().size();
        graph->edges().insert(ge->id(), ge);
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "num after=" << graph->edges().size();

        node1Name = node2Name;
    }
    edgebounds.clear();
}

void DotGraphParsingHelper::finalactions()
{
    GraphEdgeMap::iterator it, it_end;
    it = graph->edges().begin();
    it_end = graph->edges().end();
    for (; it != it_end; it++) {
        (*it)->setZ(maxZ + 1);
    }
}

}
