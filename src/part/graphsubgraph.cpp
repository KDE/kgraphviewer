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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
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
    m_label(""), 
    m_id(""),
    m_style(DOT_DEFAULT_STYLE), 
    m_lineColor(DOT_DEFAULT_LINECOLOR), 
    m_backColor(DOT_DEFAULT_BACKCOLOR),
    m_fontSize(DOT_DEFAULT_FONTSIZE),
    m_fontName(DOT_DEFAULT_FONTNAME),
    m_fontColor(DOT_DEFAULT_FONTCOLOR),
    m_z(1),
    m_renderOperations()
{
}

