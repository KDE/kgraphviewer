/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gaël de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Graph Node
 */

#include <math.h>

#include <kdebug.h>
#include <kconfig.h>

#include "dotgraphview.h"
#include "pannerview.h"
#include "canvasedge.h"
#include "graphnode.h"
#include "dotdefaults.h"

//
// GraphEdgeList
//

GraphEdgeList::GraphEdgeList()
    : _sortCallerPos(true)
{}

int GraphEdgeList::compareItems(Item item1, Item item2)
{
    CanvasEdge* e1 = ((GraphEdge*)item1)->canvasEdge();
    CanvasEdge* e2 = ((GraphEdge*)item2)->canvasEdge();

    // edges without arrow visualizations are sorted as low
    if (!e1) return -1;
    if (!e2) return 1;

    int dx1, dy1, dx2, dy2;
    int x, y;
    if (_sortCallerPos) 
    {
//       e1->controlPoints().point(0,&x,&y);
//       e2->controlPoints().point(0,&dx1,&dy1);
      dx1 -= x; dy1 -= y;
    }
    else 
    {
//       QPointArray a1 = e1->controlPoints();
//       QPointArray a2 = e2->controlPoints();
//       a1.point(a1.count()-2,&x,&y);
//       a2.point(a2.count()-1,&dx2,&dy2);
      dx2 -= x; dy2 -= y;
    }
    double at1 = atan2(double(dx1), double(dy1));
    double at2 = atan2(double(dx2), double(dy2));

    return (at1 < at2) ? 1:-1;
}

//
// GraphNode
//

GraphNode::GraphNode() :
    m_id(""),
    m_x(0), m_y(0), m_w(0), m_h(0),
    m_style(DOT_DEFAULT_STYLE), 
    m_shape(DOT_DEFAULT_SHAPE), 
    m_lineColor(DOT_DEFAULT_LINECOLOR), 
    m_backColor(DOT_DEFAULT_BACKCOLOR),
    m_fontSize(DOT_DEFAULT_FONTSIZE),
    m_fontName(DOT_DEFAULT_FONTNAME),
    m_fontColor(DOT_DEFAULT_FONTCOLOR),
    m_label(""), m_z(1),
    m_shapeFile(""), m_url("")
{
    m_visible = false;
    _lastCallerIndex = _lastCallingIndex = -1;

    callers.setSortCallerPos(false);
    callings.setSortCallerPos(true);
    _lastFromCaller = true;
}

void GraphNode::setCalling(GraphEdge* e)
{
    _lastCallingIndex = callings.findRef(e);
    _lastFromCaller = false;
}

void GraphNode::setCaller(GraphEdge* e)
{
    _lastCallerIndex = callers.findRef(e);
    _lastFromCaller = true;
}

