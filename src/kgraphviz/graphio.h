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

#ifndef GRAPHIO_H
#define GRAPHIO_H

#include <QObject>

#include "kgraphviz_export.h"

namespace KGraphViz
{

class DotGraph;
class GraphIOPrivate;

class KGRAPHVIZ_EXPORT GraphIO : public QObject
{
  Q_OBJECT

public:
  explicit GraphIO(QObject* parent = 0);
  virtual ~GraphIO();

  DotGraph* readData();

  void loadFromDotFile(const QString& fileName, const QString& layoutCommand = QString());
  void saveToDotFile(const DotGraph* graph, const QString& fileName);

  static QString internalLayoutCommandForFile(const QString& fileName);

Q_SIGNALS:
  void finished();
  void error(QString);

private:
  Q_DECLARE_PRIVATE(GraphIO)
  GraphIOPrivate* const d_ptr;
};

}

#endif // GRAPHIO_H
