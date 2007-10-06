/* This file is part of KGraphViewer.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

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
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#include "simpleprintingsettings.h"

// #include <core/keximainwindow.h>
// #include <kexiutils/utils.h>

#include <kapplication.h>
// #include <kiconloader.h>
#include <klocale.h>
// #include <kfontdialog.h>
// #include <kurllabel.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// #include <qimage.h>
// #include <qlabel.h>
// #include <qlayout.h>
// #include <qpainter.h>
// #include <qcheckbox.h>
// #include <qwhatsthis.h>
// #include <qpaintdevicemetrics.h>

// #include <kprinter.h>


KGVSimplePrintingSettings::KGVSimplePrintingSettings()
{
	pageLayout = KgvPageLayout::standardLayout();
	addPageNumbers = true;
	addDateAndTime = true;
	addTableBorders = false;
	pageTitleFont = kapp->font();
	pageTitleFont.setPointSizeFloat( (double)QFontInfo(pageTitleFont).pointSize()*1.5 );
	pageTitleFont.setBold(true);
  fittingMode = FitToOnePage;
  fitToOnePage = true;
  horizFitting = 0;
  vertFitting = 0;
  chainedFittings = true;
}

KGVSimplePrintingSettings::~KGVSimplePrintingSettings()
{
}

KGVSimplePrintingSettings KGVSimplePrintingSettings::load()
{
	KGVSimplePrintingSettings settings; //this will set defaults

	KConfig *config = kapp->sessionConfig();
  KConfigGroup simplegroup = config->group("Simple Printing");
  
// 	if (simplegroup.hasKey("pageTitleFont"))
// 		settings.pageTitleFont = simplegroup.readFontEntry("pageTitleFont");
//! @todo system default?
	if (simplegroup.hasKey("pageFormat"))
		settings.pageLayout.format = KgvPageFormat::formatFromString( 
			simplegroup.readEntry("pageFormat" ) );
	if (simplegroup.readEntry("pageOrientation", "portrait").toLower()=="landscape")
		settings.pageLayout.orientation = PG_LANDSCAPE;
	else
		settings.pageLayout.orientation = PG_PORTRAIT;
	if (simplegroup.hasKey("pageWidth"))
		settings.pageLayout.ptWidth = simplegroup.readEntry("pageWidth",0.0);
	if (simplegroup.hasKey("pageHeight"))
		settings.pageLayout.ptHeight = simplegroup.readEntry("pageHeight",0.0);
	if (simplegroup.hasKey("pageLeftMargin"))
		settings.pageLayout.ptLeft = simplegroup.readEntry("pageLeftMargin",0.0);
	if (simplegroup.hasKey("pageRightMargin"))
		settings.pageLayout.ptRight = simplegroup.readEntry("pageRightMargin",0.0);
	if (simplegroup.hasKey("pageTopMargin"))
		settings.pageLayout.ptTop = simplegroup.readEntry("pageTopMargin",0.0);
	if (simplegroup.hasKey("pageBottomMargin"))
		settings.pageLayout.ptBottom = simplegroup.readEntry("pageBottomMargin", 0.0);
	settings.addPageNumbers = simplegroup.readEntry("addPageNumbersToPage", true);
	settings.addDateAndTime = simplegroup.readEntry("addDateAndTimePage", true);
  settings.addTableBorders = simplegroup.readEntry("addTableBorders", false);
  if (simplegroup.hasKey("fittingMode") && simplegroup.readEntry("fittingMode",0) <= FitToSeveralPages)
    settings.fittingMode = FittingModes(simplegroup.readEntry("fittingMode",0));
//   std::cerr << "fittingMode after loading: " << settings.fittingMode << std::endl;
  settings.fitToOnePage = settings.fittingMode==FitToOnePage?true:false;
  if (simplegroup.hasKey("horizFitting"))
    settings.horizFitting = simplegroup.readEntry("horizFitting",0);
  if (simplegroup.hasKey("vertFitting"))
    settings.vertFitting = simplegroup.readEntry("vertFitting",0);
  settings.chainedFittings = simplegroup.readEntry("chainedFittings", true);
  return settings;
}

void KGVSimplePrintingSettings::save()
{
	KConfig *config = kapp->sessionConfig();
  KConfigGroup simplegroup = config->group("Simple Printing");

	simplegroup.writeEntry( "pageTitleFont", pageTitleFont );
	simplegroup.writeEntry( "pageFormat", KgvPageFormat::formatString( pageLayout.format ) );
	simplegroup.writeEntry("pageOrientation",
		pageLayout.orientation == PG_PORTRAIT ? "portrait" : "landscape");
	simplegroup.writeEntry("pageWidth", pageLayout.ptWidth);
	simplegroup.writeEntry("pageHeight", pageLayout.ptHeight);
	simplegroup.writeEntry("pageLeftMargin", pageLayout.ptLeft);
	simplegroup.writeEntry("pageRightMargin", pageLayout.ptRight);
	simplegroup.writeEntry("pageTopMargin", pageLayout.ptTop);
	simplegroup.writeEntry("pageBottomMargin", pageLayout.ptBottom);
	simplegroup.writeEntry("addPageNumbersToPage", addPageNumbers);
  simplegroup.writeEntry("addDateAndTimePage", addDateAndTime);
  simplegroup.writeEntry("addTableBorders", addTableBorders);
  simplegroup.writeEntry("fittingMode", (int)fittingMode);
  simplegroup.writeEntry("horizFitting", horizFitting);
  simplegroup.writeEntry("vertFitting", vertFitting);
  simplegroup.writeEntry("chainedFittings", chainedFittings);
  config->sync();
}

