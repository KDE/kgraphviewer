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

/*
 * Graph Subgraph
 */

#include "canvassubgraph.h"
#include "graphsubgraph.h"
#include "dotdefaults.h"

//
// GraphSubgraph
//

GraphSubgraph::GraphSubgraph() :
    m_cs(0),
    m_visible(false),
    m_id(""),
    m_style(DOT_DEFAULT_STYLE), 
    m_lineColor(DOT_DEFAULT_LINECOLOR), 
    m_backColor(DOT_DEFAULT_BACKCOLOR),
    m_fontSize(DOT_DEFAULT_FONTSIZE),
    m_fontName(DOT_DEFAULT_FONTNAME),
    m_fontColor(DOT_DEFAULT_FONTCOLOR),
    m_label(""), 
    m_z(1),
    m_renderOperations()
{
}

