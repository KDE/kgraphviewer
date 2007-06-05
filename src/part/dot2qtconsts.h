/***************************************************************************
 *   Copyright (C) 2005 by Gaël de Chalendar   *
 *   kleag@free.fr   *
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
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DOT2QTCONSTS_H
#define DOT2QTCONSTS_H

#include <map>
#include <qstring.h>
#include <qfont.h>

/**
@author Gaël de Chalendar
*/
class Dot2QtConsts
{
public:
  static Dot2QtConsts& changeable() {return *m_componentData;}
  static const Dot2QtConsts& componentData() {return *m_componentData;}
  
  QColor qtColor(const QString& dotColor) const;
  Qt::PenStyle qtPenStyle(const QString& dotLineStyle) const;
  QFont qtFont(const QString& dotFont) const;

private:
    Dot2QtConsts();

    ~Dot2QtConsts();

  static Dot2QtConsts* m_componentData;
  
  std::map< QString, Qt::PenStyle > m_penStyles;
  std::map< QString, QString > m_colors;
  std::map< QString, QColor > m_qcolors;
  std::map< QString, QFont > m_psFonts;
  
};

#endif
