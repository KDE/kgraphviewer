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

#include <math.h>

#include <kdebug.h>
#include <kconfig.h>

#include "graphelement.h"
#include "dotdefaults.h"

GraphElement::GraphElement() :
    QMap<QString,QString>(),
    m_z(1.0),
    m_renderOperations()
{
/*  label("");
  id("");
  style(DOT_DEFAULT_STYLE);
  shape(DOT_DEFAULT_SHAPE);
  lineColor(DOT_DEFAULT_LINECOLOR);
  backColor(DOT_DEFAULT_BACKCOLOR);
  fontSize(DOT_DEFAULT_FONTSIZE);
  fontName(DOT_DEFAULT_FONTNAME);
  fontColor(DOT_DEFAULT_FONTCOLOR);
  url("");
  shapeFile("");*/
}



QTextStream& operator<<(QTextStream& s, const GraphElement& n)
{
  GraphElement::const_iterator it, it_end;
  it = n.begin(); it_end = n.end();
  for (;it != it_end; it++)
  {
    if (!it.value().isEmpty())
    {
      if (it.key() == "label")
      {
        QString label = it.value();
        label.replace(QRegExp("\n"),"\\n");
        s << it.key() << "=\"" << label << "\",";
      }
      else
      {
          s << it.key() << "=\"" << it.value() << "\",";
      }
    }
  }
  return s;
}
