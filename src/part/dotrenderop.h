/* This file is part of KGraphViewer.
   Copyright (C) 2007 Gael de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef DOT_RENDEROP_H
#define DOT_RENDEROP_H

#include <QString>
#include <QList>
#include <QTextStream>

/**
 * members are interpreted in function of render operations definitions given at:
 * @URL http://www.graphviz.org/cvs/doc/info/output.html#d:dot
 */
struct DotRenderOp
{
  QString renderop;
  QList< int > integers;
  QString str;
};

typedef QList< DotRenderOp > DotRenderOpVec;

#endif
