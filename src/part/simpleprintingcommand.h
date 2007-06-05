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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/* This file was part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#ifndef KGVSIMPLEPRINTINGCOMMAND_H
#define KGVSIMPLEPRINTINGCOMMAND_H

#include "simpleprintingengine.h"
#include <dotgraphview.h>
#include <kgraphviewer_part.h>

class KGVSimplePrintingPageSetupBase;
class KgvPageLayoutSize;
class KGVSimplePrintPreviewWindow;

/*! @short A command for simple printing and print preview. 
 This class is instantiated in KGVMainWindowImpl so there's:
 - a single print preview window per part item regardless of a way how user invoked
    the 'print preview' command (using 'File->Print Preview' command or 'Print Preview' button 
    of the 'Page Setup' dialog)
 - a single printing engine per part item regardless of a way how user started
   (using 'File->Print' command or 'Print' button of the 'Page Setup' dialog)
*/
class KGVSimplePrintingCommand : public QObject
{
Q_OBJECT

public:
  KGVSimplePrintingCommand(DotGraphView* mainWin, int objectId, 
      QObject* parent = 0);
  ~KGVSimplePrintingCommand();

  inline KGVSimplePrintingEngine* engine() {return m_previewEngine;}

  void hidePageSetup();
  void hidePrintPreview();

public slots:
  bool print(const QString& aTitleText = QString::null);
  bool showPrintPreview(const QString& aTitleText = QString::null, bool reload = false);
  void showPageSetup(const QString& aTitleText = QString::null);

signals:
  //! connected to KGV Main Window
  void showPageSetupRequested();

protected slots:
  void slotShowPageSetupRequested();

protected:
  bool init(const QString& aTitleText = QString::null);

  KGVSimplePrintingEngine* m_previewEngine;
  DotGraphView* m_graphView;
  int m_objectId;
  KGVSimplePrintingSettings* m_settings;
  KGVSimplePrintPreviewWindow *m_previewWindow;
  bool m_printPreviewNeedsReloading : 1;
  QDialog* m_pageSetupDialog;
};

#endif
