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
    QObject(),
    m_z(1.0),
    m_renderOperations(),
    m_attributes()
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

GraphElement::GraphElement(const GraphElement& element) : QObject(),
  m_attributes()
{
  QMapIterator<QString,QString> it(element.m_attributes);
  while (it.hasNext())
  {
    m_attributes[it.key()] = it.value();
  }
}

void GraphElement::updateWith(const GraphElement& element)
{
  bool modified = false;
  QMapIterator<QString,QString> it(element.attributes());
  while (it.hasNext())
  {
    if (!m_attributes.contains(it.key()) || m_attributes[it.key()] != it.value())
    {
      m_attributes[it.key()] = it.value();
      modified = true;
    }
  }
  if (modified)
  {
    emit changed();
  }
}


QString GraphElement::backColor() const
{
  if (m_attributes.find("fillcolor") != m_attributes.end())
  {
    return m_attributes["fillcolor"];
  }
  else if ( (m_attributes.find("color") != m_attributes.end())
    && (m_attributes["style"] == "filled") )
  {
    return m_attributes["color"];
  }
  else
  {
    return DOT_DEFAULT_NODE_BACKCOLOR;
  }
}

QTextStream& operator<<(QTextStream& s, const GraphElement& n)
{
  QMap<QString,QString>::const_iterator it, it_end;
  it = n.attributes().begin(); it_end = n.attributes().end();
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

#include "graphelement.moc"
