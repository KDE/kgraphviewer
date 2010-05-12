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
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

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

namespace KGraphViewer
{
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
  bool print(const QString& aTitleText = QString());
  bool showPrintPreview(const QString& aTitleText = QString(), bool reload = false);
  void showPageSetup(const QString& aTitleText = QString());

signals:
  //! connected to KGV Main Window
  void showPageSetupRequested();

protected slots:
  void slotShowPageSetupRequested();

protected:
  bool init(const QString& aTitleText = QString());

  KGVSimplePrintingEngine* m_previewEngine;
  DotGraphView* m_graphView;
  int m_objectId;
  KGVSimplePrintingSettings* m_settings;
  KGVSimplePrintPreviewWindow *m_previewWindow;
  bool m_printPreviewNeedsReloading : 1;
  QDialog* m_pageSetupDialog;
};

}

#endif
