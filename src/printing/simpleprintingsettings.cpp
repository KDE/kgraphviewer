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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

	KConfig *config = kapp->config();
	config->setGroup("Simple Printing");
	if (config->hasKey("pageTitleFont"))
		settings.pageTitleFont = config->readFontEntry("pageTitleFont");
//! @todo system default?
	if (config->hasKey("pageFormat"))
		settings.pageLayout.format = KgvPageFormat::formatFromString( 
			config->readEntry("pageFormat" ) );
	if (config->readEntry("pageOrientation", "portrait").lower()=="landscape")
		settings.pageLayout.orientation = PG_LANDSCAPE;
	else
		settings.pageLayout.orientation = PG_PORTRAIT;
	if (config->hasKey("pageWidth"))
		settings.pageLayout.ptWidth = config->readDoubleNumEntry("pageWidth");
	if (config->hasKey("pageHeight"))
		settings.pageLayout.ptHeight = config->readDoubleNumEntry("pageHeight");
	if (config->hasKey("pageLeftMargin"))
		settings.pageLayout.ptLeft = config->readDoubleNumEntry("pageLeftMargin");
	if (config->hasKey("pageRightMargin"))
		settings.pageLayout.ptRight = config->readDoubleNumEntry("pageRightMargin");
	if (config->hasKey("pageTopMargin"))
		settings.pageLayout.ptTop = config->readDoubleNumEntry("pageTopMargin");
	if (config->hasKey("pageBottomMargin"))
		settings.pageLayout.ptBottom = config->readDoubleNumEntry("pageBottomMargin");
	settings.addPageNumbers = config->readBoolEntry("addPageNumbersToPage", true);
	settings.addDateAndTime = config->readBoolEntry("addDateAndTimePage", true);
  settings.addTableBorders = config->readBoolEntry("addTableBorders", false);
  if (config->hasKey("fittingMode") && config->readUnsignedNumEntry("fittingMode") <= FitToSeveralPages)
    settings.fittingMode = FittingModes(config->readUnsignedNumEntry("fittingMode"));
//   std::cerr << "fittingMode after loading: " << settings.fittingMode << std::endl;
  settings.fitToOnePage = settings.fittingMode==FitToOnePage?true:false;
  if (config->hasKey("horizFitting"))
    settings.horizFitting = config->readUnsignedNumEntry("horizFitting");
  if (config->hasKey("vertFitting"))
    settings.vertFitting = config->readUnsignedNumEntry("vertFitting");
  settings.chainedFittings = config->readBoolEntry("chainedFittings", true);
  return settings;
}

void KGVSimplePrintingSettings::save()
{
	KConfig *config = kapp->config();
	config->setGroup("Simple Printing");
	config->writeEntry( "pageTitleFont", pageTitleFont );
	config->writeEntry( "pageFormat", KgvPageFormat::formatString( pageLayout.format ) );
	config->writeEntry("pageOrientation", 
		pageLayout.orientation == PG_PORTRAIT ? "portrait" : "landscape");
	config->writeEntry("pageWidth", pageLayout.ptWidth);
	config->writeEntry("pageHeight", pageLayout.ptHeight);
	config->writeEntry("pageLeftMargin", pageLayout.ptLeft);
	config->writeEntry("pageRightMargin", pageLayout.ptRight);
	config->writeEntry("pageTopMargin", pageLayout.ptTop);
	config->writeEntry("pageBottomMargin", pageLayout.ptBottom);
	config->writeEntry("addPageNumbersToPage", addPageNumbers);
  config->writeEntry("addDateAndTimePage", addDateAndTime);
  config->writeEntry("addTableBorders", addTableBorders);
  config->writeEntry("fittingMode", fittingMode);
  config->writeEntry("horizFitting", horizFitting);
  config->writeEntry("vertFitting", vertFitting);
  config->writeEntry("chainedFittings", chainedFittings);
  config->sync();
}

