/***************************************************************************
 *   Copyright (C) 2005 by Gaël de Chalendar   *
 *   Gael.de-Chalendar@cea.fr   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/
#include <kdebug.h> 
#include <qcolor.h>

#include "dot2qtconsts.h"
#include "dotgrammar.h"
#include "canvasnode.h"

static const struct {
    const char * input;
    const char * roman;
    const char * italic;
    const char * bold;
    const char * boldItalic;
    const char * light;
    const char * lightItalic;
} postscriptFontNames[] = {
    { "arial", "Arial", 0, 0, 0, 0, 0 },
    { "avantgarde", "AvantGarde-Book", 0, 0, 0, 0, 0 },
    { "charter", "CharterBT-Roman", 0, 0, 0, 0, 0 },
    { "garamond", "Garamond-Regular", 0, 0, 0, 0, 0 },
    { "gillsans", "GillSans", 0, 0, 0, 0, 0 },
    { "helvetica",
      "Helvetica", "Helvetica-Oblique",
      "Helvetica-Bold", "Helvetica-BoldOblique",
      "Helvetica-Outline", "Helvetica-OutlineOblique" },
    { "palatino",
      "Palatino", "Palatino-Italic",
      "Palatino-Bold", "Palatino-BoldItalic",
      "Palatino", "Palatino-Italic" },
    { "times",
      "Times-Roman", "Times-Italic",
      "Times-Bold", "Times-BoldItalic",
      "Times-Outline", "Times-Italic" },
    { "new century schoolbook", "NewCenturySchlbk-Roman", 0, 0, 0, 0, 0 },
    { "symbol", "Symbol", "Symbol", "Symbol", "Symbol", "Symbol", "Symbol" },
    { "terminal", "Courier", 0, 0, 0, 0, 0 },
    { "times new roman", "TimesNewRoman", 0, 0, 0, 0, 0 },
    { "utopia", "Utopia-Regular", 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 }
};

Dot2QtConsts* Dot2QtConsts::m_instance = new Dot2QtConsts();

Dot2QtConsts::Dot2QtConsts()
{
  m_penStyles["solid"] = Qt::SolidLine;
  m_penStyles["dashed"] = Qt::DashLine;
  m_penStyles["dotted"] = Qt::DotLine;
  m_penStyles["invis"] = Qt::NoPen;

  m_colors["crimson"] = "#DC143C";
  m_colors["hot_pink"] = "#FF69B4";
  m_colors["light_yellow"] = "#FFFFE0";
  m_colors["slate_blue"] = "#6A5ACD";

  uint i = 0;
  while (postscriptFontNames[i].input != 0)
  {
    QFont font(postscriptFontNames[i].input);
    m_psFonts[postscriptFontNames[i].roman] = font;
    if (postscriptFontNames[i].italic != 0)
    {
      QFont italic = font; italic.setItalic(true);
      m_psFonts[postscriptFontNames[i].italic] = italic;
    }
    if (postscriptFontNames[i].bold != 0)
    {
      QFont bold = font; bold.setBold(true);
      m_psFonts[postscriptFontNames[i].bold] = bold;
    }
    if (postscriptFontNames[i].boldItalic != 0)
    {
      QFont boldItalic = font; 
      boldItalic.setItalic(true);
      boldItalic.setBold(true);
      m_psFonts[postscriptFontNames[i].boldItalic] = boldItalic;
    }
    if (postscriptFontNames[i].light != 0)
    {
      QFont light = font; light.setWeight(QFont::Light);
      m_psFonts[postscriptFontNames[i].light] = light;
    }
    if (postscriptFontNames[i].lightItalic != 0)
    {
      QFont lightItalic = font; 
      lightItalic.setWeight(QFont::Light);
      lightItalic.setItalic(true);
      m_psFonts[postscriptFontNames[i].lightItalic] = lightItalic;
    }
    i++;
  }
}


Dot2QtConsts::~Dot2QtConsts()
{}


QColor Dot2QtConsts::qtColor(const QString& dotColor) const
{
  QColor color;
  if (parse_numeric_color(dotColor.ascii(), color))
  {
    return color;
  }
  else
  {
    QColor res(dotColor);
    if (res.isValid())
    {
      return res;
    }
    else
    {
        if (m_colors.find(dotColor) != m_colors.end())
        {
            res = QColor((*m_colors.find(dotColor)).second);
            if (res.isValid())
            {
                return res;
            }
            else
            {
                kdWarning() << "Unknown dot color '" << dotColor << "'. returning Qt black" << endl;
                return Qt::black;
            }
        }
        else
        {
          kdWarning() << "Unknown dot color '" << dotColor << "'. returning Qt black" << endl;
          return Qt::black;
        }
    }
  }
}

Qt::PenStyle Dot2QtConsts::qtPenStyle(const QString& dotLineStyle) const
{
  if (m_penStyles.find(dotLineStyle) != m_penStyles.end())
    return (*(m_penStyles.find(dotLineStyle))).second;
  else 
  {
    if (dotLineStyle.left(12) != "setlinewidth")
      kdWarning() << "Unknown dot line style '" << dotLineStyle << "'. returning Qt solid line" << endl;
    return Qt::SolidLine;
  }
}

QFont Dot2QtConsts::qtFont(const QString& dotFont) const
{
  if (m_psFonts.find(dotFont) != m_psFonts.end())
    return (*(m_psFonts.find(dotFont))).second;
  else
  {
    kdWarning() << "Unknown dot font '" << dotFont << "'. returning Qt default." << endl;
    return QFont(QFont::substitute(dotFont));
  }
}

