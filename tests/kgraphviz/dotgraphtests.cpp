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

#include <kgraphviz/dotgraph.h>

#include <qtest_kde.h>

class DotGraphTests : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void testCreateEmptyGraph();
  void testCreateSelfLoopedGraph();
};

using namespace KGraphViz;

void DotGraphTests::testCreateEmptyGraph()
{
  DotGraph graph;

  QCOMPARE(graph.nodes().size(), 0);
  QCOMPARE(graph.edges().size(), 0);
  QCOMPARE(graph.subgraphs().size(), 0);
}

void DotGraphTests::testCreateSelfLoopedGraph()
{
  DotGraph graph;

  graph.addNewNode("A");
  graph.addNewEdge("A", "A");
  QCOMPARE(graph.edges().size(), 1);

  graph.addNewEdge("A", "A");
  QCOMPARE(graph.edges().size(), 2);
}

QTEST_KDEMAIN_CORE(DotGraphTests)
#include "dotgraphtests.moc"
