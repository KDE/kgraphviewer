/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gaël de Chalendar <kleag@free.fr>

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


#ifndef _KGRAPHVIEWER_H_
#define _KGRAPHVIEWER_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kparts/mainwindow.h>
#include <ktabwidget.h>
#include <kaction.h>

#include "dcopkgraphvieweriface.h"

class KToggleAction;
/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Gaël de Chalendar <kleag@free.fr>
 */
class kgraphviewer : public KParts::MainWindow, virtual public DCOPKGraphViewerIface
{
  Q_OBJECT
public:
  /**
    * Default Constructor
    */
  kgraphviewer();

  /**
    * Default Destructor
    */
  virtual ~kgraphviewer();

  /**
    * Use this method to load whatever file/URL you have
    */
  void openURL(const KURL& url);

  void reloadPreviousFiles();

protected:

  bool queryExit(); 

signals:
  void hide(KParts::Part* part);

    
/*public slots:
  void reloadOnChangeMode_pressed(int value);
  void openInExistingWindowMode_pressed(int value);
  void reopenPreviouslyOpenedFilesMode_pressed(int value);*/
  
private slots:
  void fileNew();
  void fileOpen();
  void close(QWidget* tab);
  void close();
  void slotURLSelected(const KURL&);
  void optionsShowToolbar();
  void optionsShowStatusbar();
  void optionsConfigureKeys();
  void optionsConfigureToolbars();
  void optionsConfigure();
  void newTabSelectedSlot(QWidget* tab);
    
  void applyNewToolbarConfig();

private:
  void setupAccel();
  void setupActions();
    
private:
  KTabWidget* m_widget;
  KRecentFilesAction* m_rfa;
  KParts::PartManager* m_manager;
  
  KToggleAction *m_toolbarAction;
  KToggleAction *m_statusbarAction;

  QStringList m_openedFiles;
  
  QMap<QWidget*, KParts::Part*> m_tabsPartsMap;
  QMap<QWidget*, QString> m_tabsFilesMap;
};

#endif // _KGRAPHVIEWER_H_
