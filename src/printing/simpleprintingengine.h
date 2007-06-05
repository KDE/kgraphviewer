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

#ifndef KGVSIMPLEPRINTINGENGINE_H
#define KGVSIMPLEPRINTINGENGINE_H

#include "KgvPageLayoutDia.h"

#include <qpaintdevicemetrics.h>
#include <qfontmetrics.h>
#include <qfont.h>

#include "dotgraphview.h"

class KGVSimplePrintingSettings;

/*! @short An engine painting data on pages using QPainter.
 The engine allows for random access to any page. */
class KGVSimplePrintingEngine : public QObject
{
Q_OBJECT

public:
  KGVSimplePrintingEngine( KGVSimplePrintingSettings* settings, QObject* parent );
  ~KGVSimplePrintingEngine();

  bool init(DotGraphView& data,
    const QString& titleText, QString& errorMessage);

  void setTitleText(const QString& titleText);

  //! Calculates pafe count that can be later obtained using pagesCount().
  //! Page count can depend on \a painter (printer/screen) and on printing settings.
  void calculatePagesCount(QPainter& painter);

  bool done();
  void clear();
  KGVSimplePrintingSettings* settings() { return m_settings; }

  //! \return true when all records has been painted
  bool eof() const { return m_eof; }

  //! \return number of pages. Can be used after calculatePagesCount().
  uint pagesCount() { return m_pagesCount; }

  uint maxHorizFit() const;
  uint maxVertFit() const;

	inline DotGraphView* data() {return m_data;}


public slots:
  /*! Paints a page number \a pageNumber (counted from 0) on \a painter.
    If \a paint is false, drawings are only computed but not painted, 
    so this can be used for calculating page number before printing or previewing. */
  void paintPage(int pageNumber, QPainter& painter, bool paint = true);

protected:
  KGVSimplePrintingSettings* m_settings;

  QFont m_mainFont;
  QPaintDeviceMetrics m_pdm;
  int m_dpiX, m_dpiY;
  uint m_pageWidth, m_pageHeight;
  //QFontMetrics m_headerFM, m_mainFM;
  DotGraphView* m_data;
  QString m_headerText;
  QString m_dateTimeText;
  uint m_dateTimeWidth;
  QRect m_headerTextRect;
  int m_mainLineSpacing;
  int m_footerHeight;
  uint m_pagesCount;
  bool m_eof;
  bool m_paintInitialized; //!< used by paintPage()
  double leftMargin;
  double rightMargin;
  double topMargin;
  double bottomMargin;
  double m_fx, m_fy;
  
  QPixmap m_painting;
};

#endif
