/* This file is part of KGraphViewer.
   Copyright (C) 2005-2006 Gaël de Chalendar <kleag@free.fr>

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

/* This file was part of the KDE project
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#ifndef KGVSIMPLEPRINTINGSETTINGS_H
#define KGVSIMPLEPRINTINGSETTINGS_H

#include "KgvPageLayoutDia.h"

namespace KGraphViewer
{

enum FittingModes
{
  NaturalSize = 0,
  FitToOnePage,
  FitToSeveralPages
};

//! @short Settings data for simple printing engine.
class KGVSimplePrintingSettings
{
public:
  KGVSimplePrintingSettings();
  ~KGVSimplePrintingSettings();

  static KGVSimplePrintingSettings load();
  void save();

  KgvPageLayout pageLayout;
  KgvUnit::Unit unit;
  QFont pageTitleFont;
  bool addPageNumbers;
  bool addDateAndTime;
  bool addTableBorders;
  FittingModes fittingMode;
  bool fitToOnePage;
  unsigned int horizFitting;
  unsigned int vertFitting;
  bool chainedFittings;
};

}

#endif
