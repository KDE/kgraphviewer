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
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/
#ifndef DOT2QTCONSTS_H
#define DOT2QTCONSTS_H

#include <map>
#include <qstring.h>

/**
@author Gaël de Chalendar
*/
class Dot2QtConsts
{
public:
  static Dot2QtConsts& changeable() {return *m_instance;}
  static const Dot2QtConsts& instance() {return *m_instance;}
  
  QColor qtColor(const QString& dotColor) const;
  Qt::PenStyle qtPenStyle(const QString& dotLineStyle) const;
  QFont qtFont(const QString& dotFont) const;

private:
    Dot2QtConsts();

    ~Dot2QtConsts();

  static Dot2QtConsts* m_instance;
  
  std::map< QString, Qt::PenStyle > m_penStyles;
  std::map< QString, QString > m_colors;
  std::map< QString, QFont > m_psFonts;
  
};

#endif
