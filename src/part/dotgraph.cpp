/* This file is part of KGraphViewer.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

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

#include "dotgraph.h"

// lib
#include "canvasedge.h"
#include "canvassubgraph.h"
#include "dotgrammar.h"
#include "graphexporter.h"
#include "layoutagraphthread.h"
#include "DotGraphParsingHelper.h"
#include "kgraphviewerlib_debug.h"
// KF
#include <KLocalizedString>
// Qt
#include <QFile>
#include <QMessageBox>
#include <QUuid>
// Boost
#include <boost/spirit/include/classic_confix.hpp>
// Std
#include <cmath>

using namespace boost;
using namespace boost::spirit::classic;

extern KGraphViewer::DotGraphParsingHelper *phelper;

namespace KGraphViewer
{
const distinct_parser<> keyword_p("0-9a-zA-Z_");

DotGraph::DotGraph()
    : GraphElement()
    , m_dotFileName()
    , m_width(0.0)
    , m_height(0.0)
    , m_scale(1.0)
    , m_directed(true)
    , m_strict(false)
    , m_layoutCommand()
    , m_horizCellFactor(0)
    , m_vertCellFactor(0)
    , m_wdhcf(0)
    , m_hdvcf(0)
    , m_readWrite(false)
    , m_dot(nullptr)
    , m_phase(Initial)
    , m_useLibrary(false)
{
    setId(QStringLiteral("unnamed"));
}

DotGraph::DotGraph(const QString &command, const QString &fileName)
    : GraphElement()
    , m_dotFileName(fileName)
    , m_width(0.0)
    , m_height(0.0)
    , m_scale(1.0)
    , m_directed(true)
    , m_strict(false)
    , m_layoutCommand(command)
    , m_horizCellFactor(0)
    , m_vertCellFactor(0)
    , m_wdhcf(0)
    , m_hdvcf(0)
    , m_readWrite(false)
    , m_dot(nullptr)
    , m_phase(Initial)
    , m_useLibrary(false)
{
    setId(QStringLiteral("unnamed"));
}

DotGraph::~DotGraph()
{
    qDeleteAll(m_subgraphsMap);
    m_subgraphsMap.clear();
    qDeleteAll(m_nodesMap);
    m_nodesMap.clear();
    qDeleteAll(m_edgesMap);
    m_edgesMap.clear();
}

QString DotGraph::chooseLayoutProgramForFile(const QString &str)
{
    QFile iFILE(str);

    if (!iFILE.open(QIODevice::ReadOnly)) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "Can't test dot file. Will try to use the dot command on the file: '" << str << "'";
        return QStringLiteral("dot"); // -Txdot";
    }

    QByteArray fileContent = iFILE.readAll();
    if (fileContent.isEmpty())
        return QString();
    std::string s = fileContent.data();
    std::string cmd = "dot";
    parse(s.c_str(), (!(keyword_p("strict")) >> (keyword_p("graph")[assign_a(cmd, "neato")])), (space_p | comment_p("/*", "*/")));

    return QString::fromStdString(cmd); // + " -Txdot" ;
}

bool DotGraph::parseDot(const QString &str)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << str;
    m_useLibrary = false;
    if (m_layoutCommand.isEmpty()) {
        m_layoutCommand = chooseLayoutProgramForFile(str);
        if (m_layoutCommand.isEmpty()) {
            m_layoutCommand = chooseLayoutProgramForFile(str);
            return false;
        }
    }

    qCDebug(KGRAPHVIEWERLIB_LOG) << "Running " << m_layoutCommand << str;
    QStringList options;
    /// @TODO handle the non-dot commands that could don't know the -T option
    //  if (m_readWrite && m_phase == Initial)
    //  {
    //    options << "-Tdot";
    //  }
    //  else
    //  {
    options << QStringLiteral("-Txdot");
    //   }
    options << str;

    qCDebug(KGRAPHVIEWERLIB_LOG) << "m_dot is " << m_dot << ". Acquiring mutex";
    QMutexLocker locker(&m_dotProcessMutex);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "mutex acquired ";
    if (m_dot) {
        disconnect(m_dot, nullptr, this, nullptr);
        m_dot->kill();
        delete m_dot;
    }
    m_dot = new QProcess();
    connect(m_dot, &QProcess::finished, this, &DotGraph::slotDotRunningDone);
    connect(m_dot, &QProcess::errorOccurred, this, &DotGraph::slotDotRunningError);
    m_dot->start(m_layoutCommand, options);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "process started";
    return true;
}

bool DotGraph::update()
{
    GraphExporter exporter;
    if (!m_useLibrary) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "command";
        QString str = exporter.writeDot(this);
        return parseDot(str);
    } else {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "library";
        graph_t *graph = exporter.exportToGraphviz(this);

        GVC_t *gvc = gvContext();
        threadsafe_wrap_gvLayout(gvc, graph, m_layoutCommand.toUtf8().data());
        threadsafe_wrap_gvRender(gvc, graph, "xdot", nullptr);

        updateWithGraph(graph);

        gvFreeLayout(gvc, graph);
        agclose(graph);
        bool result = true; //(gvFreeContext(gvc) == 0);
        return result;
    }
}

QByteArray DotGraph::getDotResult(int, QProcess::ExitStatus)
{
    qCDebug(KGRAPHVIEWERLIB_LOG);

    QMutexLocker locker(&m_dotProcessMutex);
    if (m_dot == nullptr) {
        return QByteArray();
    }
    QByteArray result = m_dot->readAll();
    delete m_dot;
    m_dot = nullptr;
    return result;
}

void DotGraph::slotDotRunningDone(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KGRAPHVIEWERLIB_LOG);

    QByteArray result = getDotResult(exitCode, exitStatus);
    result.replace("\\\n", "");

    qCDebug(KGRAPHVIEWERLIB_LOG) << "string content is:" << Qt::endl << result << Qt::endl << "=====================" << result.size();
    std::string s = result.data();
    //   std::cerr << "stdstring content is:" << std::endl << s << std::endl << "===================== " << s.size() << std::endl;
    if (phelper) {
        phelper->graph = nullptr;
        delete phelper;
    }
    //   if (parsingResult)
    //   {
    //     if (m_readWrite)
    //     {
    //       storeOriginalAttributes();
    //       update();
    //     }
    //     computeCells();
    //   }

    DotGraph newGraph(m_layoutCommand, m_dotFileName);
    phelper = new DotGraphParsingHelper;
    phelper->graph = &newGraph;
    phelper->z = 1;
    phelper->maxZ = 1;
    phelper->uniq = 0;

    qCDebug(KGRAPHVIEWERLIB_LOG) << "parsing new dot";
    bool parsingResult = parse(s);
    delete phelper;
    phelper = nullptr;
    qCDebug(KGRAPHVIEWERLIB_LOG) << "phelper deleted";

    if (parsingResult) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "calling updateWithGraph";
        updateWithGraph(newGraph);
    } else {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "parsing failed";
    }
    //   return parsingResult;
    //   if (m_readWrite && m_phase == Initial)
    //   {
    //     m_phase = Final;
    //     update();
    //   }
    //   else
    //   {
    qCDebug(KGRAPHVIEWERLIB_LOG) << "emiting readyToDisplay";
    Q_EMIT readyToDisplay();
    //   }
}

void DotGraph::slotDotRunningError(QProcess::ProcessError error)
{
    qCWarning(KGRAPHVIEWERLIB_LOG) << "DotGraph::slotDotRunningError" << error;
    switch (error) {
    case QProcess::FailedToStart:
        QMessageBox::critical(nullptr, i18n("Layout process failed"), i18n("Unable to start %1.", m_layoutCommand));
        break;
    case QProcess::Crashed:
        QMessageBox::critical(nullptr, i18n("Layout process failed"), i18n("%1 crashed.", m_layoutCommand));
        break;
    case QProcess::Timedout:
        QMessageBox::critical(nullptr, i18n("Layout process failed"), i18n("%1 timed out.", m_layoutCommand));
        break;
    case QProcess::WriteError:
        QMessageBox::critical(nullptr, i18n("Layout process failed"), i18n("Was not able to write data to the %1 process.", m_layoutCommand));
        break;
    case QProcess::ReadError:
        QMessageBox::critical(nullptr, i18n("Layout process failed"), i18n("Was not able to read data from the %1 process.", m_layoutCommand));
        break;
    default:
        QMessageBox::critical(nullptr, i18n("Layout process failed"), i18n("Unknown error running %1.", m_layoutCommand));
    }
}

unsigned int DotGraph::cellNumber(int x, int y)
{
    /*  qCDebug(KGRAPHVIEWERLIB_LOG) << "x= " << x << ", y= " << y << ", m_width= " << m_width << ", m_height= " << m_height << ", m_horizCellFactor= " << m_horizCellFactor << ", m_vertCellFactor= " << m_vertCellFactor  << ", m_wdhcf= " <<
     * m_wdhcf << ", m_hdvcf= " << m_hdvcf;*/

    unsigned int nx = (unsigned int)((x - (x % int(m_wdhcf))) / m_wdhcf);
    unsigned int ny = (unsigned int)((y - (y % int(m_hdvcf))) / m_hdvcf);
    /*  qCDebug(KGRAPHVIEWERLIB_LOG) << "nx = " << (unsigned int)(( x - ( x % int(m_wdhcf) ) ) / m_wdhcf);
      qCDebug(KGRAPHVIEWERLIB_LOG) << "ny = " << (unsigned int)(( y - ( y % int(m_hdvcf) ) ) / m_hdvcf);
      qCDebug(KGRAPHVIEWERLIB_LOG) << "res = " << ny * m_horizCellFactor + nx;*/

    unsigned int res = ny * m_horizCellFactor + nx;
    return res;
}

#define MAXCELLWEIGHT 800

void DotGraph::computeCells()
{
    return;
    qCDebug(KGRAPHVIEWERLIB_LOG) << m_width << m_height;
    m_horizCellFactor = m_vertCellFactor = 1;
    m_wdhcf = (int)ceil(((double)m_width) / m_horizCellFactor) + 1;
    m_hdvcf = (int)ceil(((double)m_height) / m_vertCellFactor) + 1;
    bool stop = true;
    do {
        stop = true;
        m_cells.clear();
        //     m_cells.resize(m_horizCellFactor * m_vertCellFactor);

        GraphNodeMap::iterator it, it_end;
        it = m_nodesMap.begin();
        it_end = m_nodesMap.end();
        for (; it != it_end; it++) {
            GraphNode *gn = *it;
            //       int cellNum = cellNumber(int(gn->x()), int(gn->y()));
            int cellNum = cellNumber(0, 0);
            qCDebug(KGRAPHVIEWERLIB_LOG) << "Found cell number " << cellNum;

            if (m_cells.size() <= cellNum) {
                m_cells.resize(cellNum + 1);
            }
            m_cells[cellNum].insert(gn);

            qCDebug(KGRAPHVIEWERLIB_LOG) << "after insert";
            if (m_cells[cellNum].size() > MAXCELLWEIGHT) {
                qCDebug(KGRAPHVIEWERLIB_LOG) << "cell number " << cellNum << " contains " << m_cells[cellNum].size() << " nodes";
                if ((m_width / m_horizCellFactor) > (m_height / m_vertCellFactor)) {
                    m_horizCellFactor++;
                    m_wdhcf = m_width / m_horizCellFactor;
                } else {
                    m_vertCellFactor++;
                    m_hdvcf = m_height / m_vertCellFactor;
                }
                qCDebug(KGRAPHVIEWERLIB_LOG) << "cell factor is now " << m_horizCellFactor << " / " << m_vertCellFactor;
                stop = false;
                break;
            }
        }
    } while (!stop);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "m_wdhcf=" << m_wdhcf << "; m_hdvcf=" << m_hdvcf;
    qCDebug(KGRAPHVIEWERLIB_LOG) << "finished";
}

QSet<GraphNode *> &DotGraph::nodesOfCell(unsigned int id)
{
    return m_cells[id];
}

void DotGraph::storeOriginalAttributes()
{
    for (GraphNode *node : nodes()) {
        node->storeOriginalAttributes();
    }
    for (GraphEdge *edge : edges()) {
        edge->storeOriginalAttributes();
    }
    for (GraphSubgraph *subgraph : subgraphs()) {
        subgraph->storeOriginalAttributes();
    }
    GraphElement::storeOriginalAttributes();
}

void DotGraph::saveTo(const QString &fileName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << fileName;
    m_dotFileName = fileName;
    GraphExporter exporter;
    exporter.writeDot(this, fileName);
}

void DotGraph::updateWithGraph(graph_t *newGraph)
{
    qCDebug(KGRAPHVIEWERLIB_LOG);

    // copy global graph render operations and attributes
    DotRenderOpVec ops;
    // decrease mem peak
    setRenderOperations(ops);

    if (agget(newGraph, (char *)"_draw_")) {
        parse_renderop(agget(newGraph, (char *)"_draw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "_draw_: element renderOperations size is now " << ops.size();
    }
    if (agget(newGraph, (char *)"_ldraw_")) {
        parse_renderop(agget(newGraph, (char *)"_ldraw_"), ops);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "_ldraw_: element renderOperations size is now " << ops.size();
    }

    setRenderOperations(ops);

    Agsym_t *attr = agnxtattr(newGraph, AGRAPH, nullptr);
    while (attr) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << agnameof(newGraph) << ":" << attr->name << agxget(newGraph, attr);
        m_attributes[QString::fromUtf8(attr->name)] = QString::fromUtf8(agxget(newGraph, attr));
        attr = agnxtattr(newGraph, AGRAPH, attr);
    }

    // copy subgraphs
    for (graph_t *sg = agfstsubg(newGraph); sg; sg = agnxtsubg(sg)) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "subgraph:" << agnameof(sg);
        const QString subgraphName = QString::fromUtf8(agnameof(sg));
        auto subgraphIt = subgraphs().find(subgraphName);
        if (subgraphIt != subgraphs().constEnd()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "known";
            GraphSubgraph *subgraph = subgraphIt.value();
            // ???
            //       nodes()[ngn->name]->setZ(ngn->z());
            subgraph->updateWithSubgraph(sg);
            if (subgraph->canvasElement()) {
                //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
            }
        } else {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "new";
            GraphSubgraph *newsg = new GraphSubgraph(sg);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new created";
            subgraphs().insert(QString::fromUtf8(agnameof(sg)), newsg);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new inserted";
        }
    }

    // copy nodes
    node_t *ngn = agfstnode(newGraph);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "first node:" << (void *)ngn;

    while (ngn)
    //   foreach (GraphNode* ngn, newGraph.nodes())
    {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "node " << agnameof(ngn);
        const QString nodeName = QString::fromUtf8(agnameof(ngn));
        auto nodeIt = nodes().find(nodeName);
        if (nodeIt != nodes().constEnd()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "known";
            GraphNode *node = nodeIt.value();
            // ???
            //       nodes()[ngn->name]->setZ(ngn->z());
            node->updateWithNode(ngn);
            if (node->canvasElement()) {
                //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
            }
        } else {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "new";
            GraphNode *newgn = new GraphNode(ngn);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new created";
            nodes().insert(nodeName, newgn);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new inserted";
        }

        // copy node edges
        edge_t *nge = agfstout(newGraph, ngn);
        while (nge) {
            //      qCDebug(KGRAPHVIEWERLIB_LOG) << "edge " << nge->id;
            const QString edgeName = QString::fromUtf8(agnameof(aghead(nge))) + QString::fromUtf8(agnameof(agtail(nge)));
            auto edgeIt = edges().find(edgeName);
            if (edgeIt != edges().constEnd()) {
                GraphEdge *edge = edgeIt.value();
                //        () << "edge known" << nge->id;
                //         edges()[nge->name]->setZ(nge->z());
                edge->updateWithEdge(nge);
                if (edge->canvasEdge()) {
                    //         edges()[nge->id()]->canvasEdge()->setGh(m_height);
                }
            } else {
                qCDebug(KGRAPHVIEWERLIB_LOG) << "new edge" << edgeName;
                {
                    GraphEdge *newEdge = new GraphEdge();
                    newEdge->setId(edgeName);
                    newEdge->updateWithEdge(nge);
                    const QString edgeTailName = QString::fromUtf8(agnameof(agtail(nge)));
                    if (elementNamed(edgeTailName) == nullptr) {
                        GraphNode *newgn = new GraphNode();
                        //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new created";
                        nodes().insert(edgeTailName, newgn);
                    }
                    newEdge->setFromNode(elementNamed(edgeTailName));
                    const QString edgeHeadName = QString::fromUtf8(agnameof(aghead(nge)));
                    if (elementNamed(edgeHeadName) == nullptr) {
                        GraphNode *newgn = new GraphNode();
                        //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new created";
                        nodes().insert(edgeHeadName, newgn);
                    }
                    newEdge->setToNode(elementNamed(edgeHeadName));
                    edges().insert(edgeName, newEdge);
                }
            }
            nge = agnxtedge(newGraph, nge, ngn);
        }
        ngn = agnxtnode(newGraph, ngn);
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << "Done";
    Q_EMIT readyToDisplay();
    computeCells();
}

void DotGraph::updateWithGraph(const DotGraph &newGraph)
{
    GraphElement::updateWithElement(newGraph);
    m_width = newGraph.width();
    m_height = newGraph.height();
    m_scale = newGraph.scale();
    m_directed = newGraph.directed();
    m_strict = newGraph.strict();
    computeCells();
    for (GraphSubgraph *nsg : newGraph.subgraphs()) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "subgraph" << nsg->id();
        auto subgraphIt = subgraphs().find(nsg->id());
        if (subgraphIt != subgraphs().constEnd()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "subgraph known" << nsg->id();
            GraphSubgraph *subgraph = subgraphIt.value();
            subgraph->updateWithSubgraph(*nsg);
            if (subgraph->canvasElement()) {
                //         subgraphs().value(nsg->id())->canvasElement()->setGh(m_height);
            }
        } else {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "new subgraph" << nsg->id();
            GraphSubgraph *newSubgraph = new GraphSubgraph();
            newSubgraph->updateWithSubgraph(*nsg);
            newSubgraph->setZ(0);
            subgraphs().insert(nsg->id(), newSubgraph);
        }
    }
    for (GraphNode *ngn : newGraph.nodes()) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "node " << ngn->id();
        auto nodeIt = nodes().find(ngn->id());
        if (nodeIt != nodes().constEnd()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "known";
            GraphNode *node = nodeIt.value();
            node->setZ(ngn->z());
            node->updateWithNode(*ngn);
            if (node->canvasElement()) {
                //         nodes()[ngn->id()]->canvasElement()->setGh(m_height);
            }
        } else {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "new";
            GraphNode *newgn = new GraphNode(*ngn);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new created";
            nodes().insert(ngn->id(), newgn);
            //       qCDebug(KGRAPHVIEWERLIB_LOG) << "new inserted";
        }
    }
    for (GraphEdge *nge : newGraph.edges()) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "edge " << nge->id();
        auto edgeIt = edges().find(nge->id());
        if (edgeIt != edges().constEnd()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "edge known" << nge->id();
            GraphEdge *edge= edgeIt.value();
            edge->setZ(nge->z());
            edge->updateWithEdge(*nge);
            if (edge->canvasEdge()) {
                //         edges()[nge->id()]->canvasEdge()->setGh(m_height);
            }
        } else {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "new edge" << nge->id();
            {
                GraphEdge *newEdge = new GraphEdge();
                newEdge->setId(nge->id());
                newEdge->updateWithEdge(*nge);
                newEdge->setFromNode(elementNamed(nge->fromNode()->id()));
                newEdge->setToNode(elementNamed(nge->toNode()->id()));
                edges().insert(nge->id(), newEdge);
            }
        }
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << "Done";
    computeCells();
}

void DotGraph::removeNodeNamed(const QString &nodeName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << nodeName;
    GraphNode *node = dynamic_cast<GraphNode *>(elementNamed(nodeName));
    if (node == nullptr) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "No such node " << nodeName;
        return;
    }

    GraphEdgeMap::iterator it, it_end;
    it = m_edgesMap.begin();
    it_end = m_edgesMap.end();
    while (it != it_end) {
        if (it.value()->fromNode() == node || it.value()->toNode() == node) {
            GraphEdge *edge = it.value();
            if (edge->canvasEdge()) {
                edge->canvasEdge()->hide();
                delete edge->canvasEdge();
                delete edge;
            }
            it = edges().erase(it);
        } else {
            ++it;
        }
    }

    if (node->canvasNode()) {
        node->canvasNode()->hide();
        delete node->canvasNode();
        node->setCanvasNode(nullptr);
    }
    nodes().remove(nodeName);
    delete node;
}

void DotGraph::removeNodeFromSubgraph(const QString &nodeName, const QString &subgraphName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << nodeName << subgraphName;
    GraphNode *node = dynamic_cast<GraphNode *>(elementNamed(nodeName));
    if (node == nullptr) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "No such node " << nodeName;
        return;
    }

    GraphSubgraph *subgraph = subgraphs()[subgraphName];
    if (subgraph == nullptr) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "No such subgraph " << subgraphName;
        return;
    }

    subgraph->removeElement(node);
    if (subgraph->content().isEmpty()) {
        removeSubgraphNamed(subgraphName);
    }
}

void DotGraph::removeSubgraphNamed(const QString &subgraphName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << subgraphName << " from " << subgraphs().keys();
    GraphSubgraph *subgraph = subgraphs()[subgraphName];

    if (subgraph == nullptr) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "Subgraph" << subgraphName << "not found";
        return;
    }
    GraphEdgeMap::iterator it, it_end;
    it = m_edgesMap.begin();
    it_end = m_edgesMap.end();
    while (it != it_end) {
        if (it.value()->fromNode() == subgraph || it.value()->toNode() == subgraph) {
            GraphEdge *edge = it.value();
            if (edge->canvasEdge()) {
                edge->canvasEdge()->hide();
                delete edge->canvasEdge();
                delete edge;
            }
            it = edges().erase(it);
        } else {
            ++it;
        }
    }

    if (subgraph->canvasSubgraph()) {
        subgraph->canvasSubgraph()->hide();
        delete subgraph->canvasSubgraph();
        subgraph->setCanvasSubgraph(nullptr);
    }
    for (GraphElement *element : subgraph->content()) {
        if (dynamic_cast<GraphNode *>(element)) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "Adding" << element->id() << "to main graph";
            nodes()[element->id()] = dynamic_cast<GraphNode *>(element);
        } else if (dynamic_cast<GraphSubgraph *>(element)) {
            subgraphs()[element->id()] = dynamic_cast<GraphSubgraph *>(element);
        } else {
            qCWarning(KGRAPHVIEWERLIB_LOG) << "Don't know how to handle" << element->id();
        }
    }
    subgraph->content().clear();
    subgraphs().remove(subgraphName);
    delete subgraph;
}

void DotGraph::removeEdge(const QString &id)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << id;
    GraphEdgeMap::iterator it = edges().begin();
    for (; it != edges().end(); it++) {
        GraphEdge *edge = it.value();
        if (edge->id() == id) {
            if (edge->canvasEdge()) {
                edge->canvasEdge()->hide();
                delete edge->canvasEdge();
                delete edge;
            }
            edges().remove(id);
            break;
        }
    }
}

void DotGraph::removeElement(const QString &id)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << id;
    GraphNodeMap::const_iterator itN = nodes().constBegin();
    for (; itN != nodes().constEnd(); itN++) {
        const GraphNode *node = itN.value();
        if (node->id() == id) {
            removeNodeNamed(id);
            return;
        }
    }
    GraphEdgeMap::const_iterator itE = edges().constBegin();
    for (; itE != edges().constEnd(); itE++) {
        const GraphEdge *edge = itE.value();
        if (edge->id() == id) {
            removeEdge(id);
            return;
        }
    }
    GraphSubgraphMap::const_iterator itS = subgraphs().constBegin();
    for (; itS != subgraphs().constEnd(); itS++) {
        const GraphSubgraph *subgraph = itS.value();
        if (subgraph->id() == id) {
            removeSubgraphNamed(id);
            return;
        }
    }
}

void DotGraph::setAttribute(const QString &elementId, const QString &attributeName, const QString &attributeValue)
{
    QMap<QString, QString> *attributes = nullptr;

    auto nodeIt = nodes().find(elementId);
    if (nodeIt != nodes().constEnd()) {
        attributes = &(nodeIt.value()->attributes());
    } else {
        auto edgeIt = edges().find(elementId);
        if (edgeIt != edges().constEnd()) {
            attributes = &(edgeIt.value()->attributes());
        } else {
            auto subgraphIt = subgraphs().find(elementId);
            if (subgraphIt != subgraphs().constEnd()) {
                attributes = &(subgraphIt.value()->attributes());
            }
        }
    }
    if (attributes) {
        (*attributes)[attributeName] = attributeValue;
    }
}

GraphElement *DotGraph::elementNamed(const QString &id)
{
    GraphElement *ret = nullptr;
    if ((ret = m_nodesMap.value(id, nullptr))) {
        return ret;
    }
    if ((ret = m_edgesMap.value(id, nullptr))) {
        return ret;
    }
    for (GraphSubgraph *subGraph : m_subgraphsMap) {
        if ((ret = subGraph->elementNamed(id))) {
            return ret;
        }
    }
    return nullptr;
}

void DotGraph::setGraphAttributes(QMap<QString, QString> attribs)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribs;
    attributes() = attribs;
}

void DotGraph::addNewNode(QMap<QString, QString> attribs)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribs;
    GraphNode *newNode = new GraphNode();
    newNode->attributes() = attribs;
    nodes().insert(newNode->id(), newNode);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "node added as" << newNode->id();
}

void DotGraph::addNewSubgraph(QMap<QString, QString> attribs)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribs;
    GraphSubgraph *newSG = new GraphSubgraph();
    newSG->attributes() = attribs;
    subgraphs()[newSG->id()] = newSG;
    qCDebug(KGRAPHVIEWERLIB_LOG) << "subgraph added as" << newSG->id();
}

void DotGraph::addNewNodeToSubgraph(QMap<QString, QString> attribs, QString subgraph)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribs << "to" << subgraph;
    GraphNode *newNode = new GraphNode();
    newNode->attributes() = attribs;
    subgraphs()[subgraph]->content().push_back(newNode);

    qCDebug(KGRAPHVIEWERLIB_LOG) << "node added as" << newNode->id() << "in" << subgraph;
}

void DotGraph::addExistingNodeToSubgraph(QMap<QString, QString> attribs, QString subgraph)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribs << "to" << subgraph;
    const QString id = attribs[QStringLiteral("id")];
    GraphNode *node = dynamic_cast<GraphNode *>(elementNamed(id));
    if (node == nullptr) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "No such node" << id;
        return;
    }
    if (nodes().contains(id)) {
        nodes().remove(id);
        node->attributes() = attribs;
        subgraphs()[subgraph]->content().push_back(node);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "node " << node->id() << " added in " << subgraph;
    } else {
        for (GraphSubgraph *gs : subgraphs()) {
            GraphElement *elt = nullptr;
            for (GraphElement *element : gs->content()) {
                if (element == node) {
                    elt = element;
                    break;
                }
            }
            if (elt) {
                qCDebug(KGRAPHVIEWERLIB_LOG) << "removing node " << elt->id() << " from " << gs->id();
                gs->removeElement(elt);
                subgraphs()[subgraph]->content().push_back(elt);
                qCDebug(KGRAPHVIEWERLIB_LOG) << "node " << elt->id() << " added in " << subgraph;
                break;
            }
        }
    }
}

void DotGraph::moveExistingNodeToMainGraph(QMap<QString, QString> attribs)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribs;
    const QString id = attribs[QStringLiteral("id")];
    GraphNode *node = dynamic_cast<GraphNode *>(elementNamed(id));
    if (node == nullptr) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "No such node" << id;
        return;
    } else if (nodes().contains(id)) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "Node" << id << "already in main graph";
        return;
    } else {
        for (GraphSubgraph *gs : subgraphs()) {
            bool found = false;
            for (GraphElement *element : gs->content()) {
                if (element == node) {
                    found = true;
                    break;
                }
            }
            if (found) {
                qCDebug(KGRAPHVIEWERLIB_LOG) << "removing node " << node->id() << " from " << gs->id();
                gs->removeElement(node);
                nodes()[node->id()] = node;
                qCDebug(KGRAPHVIEWERLIB_LOG) << "node " << node->id() << " moved to main graph";
                break;
            }
        }
    }
}

void DotGraph::addNewEdge(QString src, QString tgt, QMap<QString, QString> attribs)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << src << tgt << attribs;
    GraphEdge *newEdge = new GraphEdge();
    newEdge->attributes() = attribs;
    GraphElement *srcElement = elementNamed(src);
    if (srcElement == nullptr) {
        srcElement = elementNamed(QStringLiteral("cluster_") + src);
    }
    GraphElement *tgtElement = elementNamed(tgt);
    if (tgtElement == nullptr) {
        tgtElement = elementNamed(QStringLiteral("cluster_") + tgt);
    }

    if (srcElement == nullptr || tgtElement == nullptr) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << src << "or" << tgt << "missing";
        return;
    }
    auto it = attribs.find(QStringLiteral("id"));
    if (it != attribs.constEnd()) {
        newEdge->setId(*it);
    } else {
        newEdge->setId(src + tgt + QUuid::createUuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}')).remove(QLatin1Char('-')));
    }
    newEdge->setFromNode(srcElement);
    newEdge->setToNode(tgtElement);
    edges().insert(newEdge->id(), newEdge);
}

void DotGraph::removeAttribute(const QString &nodeName, const QString &attribName)
{
    GraphElement *element = elementNamed(nodeName);
    if (element) {
        element->removeAttribute(attribName);
    }
}

void DotGraph::renameNode(const QString &oldNodeName, const QString &newNodeName)
{
    if (oldNodeName != newNodeName) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "Renaming " << oldNodeName << " into " << newNodeName;
        GraphNode *node = nodes()[oldNodeName];
        nodes().remove(oldNodeName);
        node->setId(newNodeName);
        nodes()[newNodeName] = node;
    }
}

QString DotGraph::backColor() const
{
    auto it = m_attributes.find(QStringLiteral("bgcolor"));
    if (it != m_attributes.end()) {
        return *it;
    }

    return QString();
}

}

#include "moc_dotgraph.cpp"
