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

#include "loadagraphthread.h"

#include <kdebug.h>
#include <kurl.h>

#include <QFile>

void LoadAGraphThread::run()
{
  kDebug() << m_dotFileName;

  // FIXME: Fix this for remote files, work-around for crash in agread
  KUrl url(m_dotFileName);
  if (!url.isLocalFile()) {
    kWarning() << "Loading remote files not supported";
    return;
  }

  if (!QFile(url.toLocalFile()).exists()) {
    kWarning() << "File does not exist:" << url.toLocalFile();
    return;
  }

  const QString sanitizedFilename = url.path();
  m_gvc = gvContext();
  FILE* fp = fopen(sanitizedFilename.toUtf8().data(), "r");
  m_g = agread(fp);
}

void LoadAGraphThread::loadFile(const QString& dotFileName)
{
  kDebug();
  if (isRunning())
    return;

  m_g = 0;
  m_gvc = 0;
  m_dotFileName = dotFileName;
  start();
}
