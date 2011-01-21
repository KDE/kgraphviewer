/*
    Copyright (C) 2010 Kevin Funk <krf@electrostorm.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "utils.h"

#include <kgraphviz/graphio.h>
#include <kgraphviz/dotgraph.h>
#include <kgraphviz/graphsubgraph.h>

#include <qtest_kde.h>

#include <KDebug>

#define TEST_FILENAME "/tmp/kgraphviz-test.dot"

class GraphIOTests : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void testImportDirectedAcyclicGraph();
  void testImportSubGraph();
  void testImportDirectedSelfLoopedGraph();
  void testImportUndirectedSelfLoopedGraph();

  void testImportGraphFromInvalidFile();

  void testExportImportGraph();

  void testUpdateDotGraph();
};

using namespace KGraphViz;

static DotGraph* loadGraphFromFile(const QString& fileName)
{
  GraphIO io;
  io.loadFromDotFile(fileName);
  const bool ret = QTest::kWaitForSignal(&io, SIGNAL(finished()), 100);
  if (!ret) {
    qWarning() << "Loading failed";
    return 0;
  }

  return io.readData();
}

void GraphIOTests::testImportDirectedAcyclicGraph()
{
  DotGraph* graph = loadGraphFromFile("examples/directed-acyclic-graph.dot");
  QVERIFY(graph != 0);
  QVERIFY(graph->nodes().size() == 4);
  QVERIFY(graph->edges().size() == 3);
}

void GraphIOTests::testImportSubGraph()
{
  DotGraph* graph = loadGraphFromFile("examples/subgraph.dot");
  QVERIFY(graph != 0);

  // test root
  QCOMPARE(graph->nodes().size(), 2); // 2 local ones
  QCOMPARE(graph->edges().size(), 13);
  QCOMPARE(graph->subgraphs().size(), 2);

  // test first subgraph
  GraphSubgraph* subgraph1 = graph->subgraphs()["cluster_0"];
  QVERIFY(subgraph1 != 0);
  QCOMPARE(subgraph1->content().size(), 4);

  // test second subgraph
  GraphSubgraph* subgraph2 = graph->subgraphs()["cluster_1"];
  QVERIFY(subgraph2 != 0);
  QCOMPARE(subgraph2->content().size(), 4);
}

void GraphIOTests::testImportDirectedSelfLoopedGraph()
{
  DotGraph* graph = loadGraphFromFile("examples/directed-self-looped-graph.dot");
  QVERIFY(graph != 0);
  QCOMPARE(graph->edges().size(), 4);
  QCOMPARE(graph->nodes().size(), 3);
}

void GraphIOTests::testImportUndirectedSelfLoopedGraph()
{
  DotGraph* graph = loadGraphFromFile("examples/undirected-self-looped-graph.dot");
  QVERIFY(graph != 0);
  QCOMPARE(graph->edges().size(), 4);
  QCOMPARE(graph->nodes().size(), 3);
}

void GraphIOTests::testImportGraphFromInvalidFile()
{
  DotGraph* graph = loadGraphFromFile("examples/DOES_NOT_EXIST.abc");
  QVERIFY(graph == 0);
}

void GraphIOTests::testExportImportGraph()
{
  // populate and save graph to file
  {
    // populate
    QMap <QString, QString> map;
    DotGraph graph;
    map["id"] = "State1";
    graph.addNewNode(map);
    map["id"] = "State2";
    graph.addNewNode(map);
    map.clear();
    graph.addNewEdge("State1", "State2", map);

    // save back
    GraphIO io;
    io.saveToDotFile(&graph, TEST_FILENAME);
  }

  // read graph from dot file and check attributes
  {
    DotGraph* graph = loadGraphFromFile(TEST_FILENAME);
    QVERIFY(graph != 0);
    QVERIFY(graph->nodes().size() == 2);
    QVERIFY(graph->edges().size() == 1);
  }

  // cleanup
  QFile::remove(TEST_FILENAME);
}

void GraphIOTests::testUpdateDotGraph()
{
  // populate
  QMap <QString, QString> map;
  DotGraph graph;
  map["id"] = "State1";
  graph.addNewNode(map);
  map["id"] = "State2";
  graph.addNewNode(map);
  map.clear();
  graph.addNewEdge("State1", "State2", map);

  GraphIO io;
  io.updateDot(&graph);
  QSignalSpy spy(&io, SIGNAL(finished()));
  QVERIFY(QTest::kWaitForSignal(&io, SIGNAL(finished()), 3000) == true);

  DotGraph* updatedGraph = io.readData();
  QVERIFY(updatedGraph != 0);
  QVERIFY(Utilities::haveSameElementCount(&graph, updatedGraph) == true);
}

QTEST_KDEMAIN_CORE(GraphIOTests)
#include "graphiotests.moc"
