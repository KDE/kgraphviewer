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
#include "kgraphviewerlib_debug.h"

#include <QDebug>

void LoadAGraphThread::run()
{
  qCDebug(KGRAPHVIEWERLIB_LOG) << m_dotFileName;
  FILE* fp = fopen(m_dotFileName.toUtf8().data(), "r");
  if (!fp)
  {
      qCWarning(KGRAPHVIEWERLIB_LOG) << "Failed to open file " << m_dotFileName;
      return;
  }
  m_g = agread(fp, nullptr);
  if (!m_g)
  {
      qCWarning(KGRAPHVIEWERLIB_LOG) << "Failed to read file, retrying to work around graphviz bug(?)";
      rewind(fp);
      m_g = agread(fp, nullptr);
  }
  if (m_g == nullptr)
  {
      qCWarning(KGRAPHVIEWERLIB_LOG) << "Failed to read file " << m_dotFileName;
  }
  fclose(fp);
}

void LoadAGraphThread::loadFile(const QString& dotFileName)
{
  sem.acquire();
  m_dotFileName = dotFileName;
  m_g = nullptr;
  start();
}
