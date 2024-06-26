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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

#ifndef DOT_RENDEROP_H
#define DOT_RENDEROP_H

// Qt
#include <QList>
#include <QString>

/**
 * members are interpreted in function of render operations definitions given at:
 * @URL https://graphviz.org/docs/outputs/canon/
 */
struct DotRenderOp {
    QString renderop;
    QList<int> integers;
    QString str;
};

typedef QList<DotRenderOp> DotRenderOpVec;

#endif
