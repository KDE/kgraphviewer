#include <kgraphviz/dotgraph.h>

#include <QtTest/QtTest>

class DotGraphTests : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void testCreateGraph();
};

using namespace KGraphViewer;

void DotGraphTests::testCreateGraph()
{
  DotGraph graph;

  QVERIFY(graph.nodes().size() == 0);
  QVERIFY(graph.edges().size() == 0);
  QVERIFY(graph.subgraphs().size() == 0);
}

QTEST_MAIN(DotGraphTests)
#include "dotgraphtests.moc"*/
