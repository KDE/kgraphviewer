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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef _KGRAPHVIEWER_H_
#define _KGRAPHVIEWER_H_

//#include <config-kgraphviewer.h>

#include <kapplication.h>
#include <kparts/mainwindow.h>
#include <ktabwidget.h>
#include <kaction.h>
#include <krecentfilesaction.h>


class KToggleAction;
/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Gaël de Chalendar <kleag@free.fr>
 */
class KGraphViewer : public KParts::MainWindow 
{
  Q_OBJECT
public:
  /**
    * Default Constructor
    */
  KGraphViewer();

  /**
    * Default Destructor
    */
  virtual ~KGraphViewer();

  /**
    * Use this method to load whatever file/URL you have
    */
  void openUrl(const KUrl& url);
 
  void reloadPreviousFiles();

protected:

  bool queryExit(); 

signals:
  void hide(KParts::Part* part);

    
public slots:
  /**
    * Use this method to load whatever file/URL you have
    */
  void openUrl(const QString& url) {openUrl(KUrl(url));}

/*public slots:
  void reloadOnChangeMode_pressed(int value);
  void openInExistingWindowMode_pressed(int value);
  void reopenPreviouslyOpenedFilesMode_pressed(int value);*/
  
private slots:
  void fileNew();
  void fileOpen();
  void close(QWidget* tab);
  void close();
  void slotURLSelected(const KUrl&);
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
