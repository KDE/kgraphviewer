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

void LoadAGraphThread::run()
{
  kDebug() << m_dotFileName;
  FILE* fp = fopen(m_dotFileName.toUtf8().data(), "r");
  if (!fp)
  {
      kError() << "Failed to open file " << m_dotFileName;
      return;
  }
  m_g = agread(fp, NULL);
  if (!m_g)
  {
      kError() << "Failed to read file, retrying to work around graphviz bug(?)";
      rewind(fp);
      m_g = agread(fp, NULL);
  }
  if (m_g==0)
  {
      kError() << "Failed to read file " << m_dotFileName;
  }
  fclose(fp);
}

void LoadAGraphThread::loadFile(const QString& dotFileName)
{
  kDebug();
  sem.acquire();
  m_dotFileName = dotFileName;
  m_g = NULL;
  start();
}
