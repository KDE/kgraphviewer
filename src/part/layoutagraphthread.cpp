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

#include <QMutex>

#include <kdebug.h>

static QMutex gv_mutex;

// Not sure just wrapping these two is really enough, but it
// fixed all crashes when loading many files at once for me.

int threadsafe_wrap_gvLayout(GVC_t *gvc, graph_t *g, const char *engine)
{
  QMutexLocker locker(&gv_mutex);
  return gvLayout(gvc, g, engine);
}

int threadsafe_wrap_gvRender(GVC_t *gvc, graph_t *g, const char *format, FILE *out)
{
  QMutexLocker locker(&gv_mutex);
  return gvRender(gvc, g, format, out);
}

LayoutAGraphThread::LayoutAGraphThread() : sem(1)
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
  if (!m_g)
  {
    kError() << "No graph loaded, skipping layout";
    return;
  }
  threadsafe_wrap_gvLayout(m_gvc, m_g, m_layoutCommand.toUtf8().data());
  threadsafe_wrap_gvRender(m_gvc, m_g, "xdot", NULL);
}

void LayoutAGraphThread::layoutGraph(graph_t* graph, const QString& layoutCommand)
{
  kDebug();
  sem.acquire();
  m_g = graph;
  m_layoutCommand = layoutCommand;
  start();
}

