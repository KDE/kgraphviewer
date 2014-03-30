/*
    This file is part of KGraphViewer.
    Copyright (C) 2010  Gael de Chalendar <kleag@free.fr>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "layoutagraphthread.h"

#include <kdebug.h>

LayoutAGraphThread::LayoutAGraphThread()
{
  m_gvc = gvContext();
}

LayoutAGraphThread::~LayoutAGraphThread()
{
  gvFreeContext(m_gvc);
}

void LayoutAGraphThread::run()
{
  kDebug();
  gvLayout(m_gvc, m_g, m_layoutCommand.toUtf8().data());
  gvRender (m_gvc, m_g, "xdot", NULL);
}

void LayoutAGraphThread::layoutGraph(graph_t* graph, const QString& layoutCommand)
{
  kDebug();
  if (isRunning())
    return;
  m_g = graph;
  m_layoutCommand = layoutCommand;
  start();
}

