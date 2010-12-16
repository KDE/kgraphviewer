/*
    Copyright (C) 2010 Kevin Funk <krf@electrostorm.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef GRAPHELEMENT_P_H
#define GRAPHELEMENT_P_H

#include "graphelement.h"

namespace KGraphViz
{

class GraphElementPrivate
{
public:
  GraphElementPrivate();
  GraphElementPrivate(const GraphElementPrivate& other);
  
  ~GraphElementPrivate();

  bool m_selected;
  double m_z;

  DotRenderOpVec m_renderOperations;
  CanvasElement* m_canvasElement;
};

}

#endif