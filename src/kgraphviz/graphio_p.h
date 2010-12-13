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

#ifndef GRAPHIO_P_H
#define GRAPHIO_P_H

#include "graphio.h"

#include <QProcess>

class KGraphViz::GraphIOPrivate
  : public QObject
{
  Q_OBJECT

public:
  explicit GraphIOPrivate(QObject* parent = 0);
  virtual ~GraphIOPrivate();

  void reset();

  DotGraph* m_dotGraph;

  QProcess m_process;

Q_SIGNALS:
  void finished();
  void error(QString);

private Q_SLOTS:
  void processFinished(int, QProcess::ExitStatus);
};

#endif
