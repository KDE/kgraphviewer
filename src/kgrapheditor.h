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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef _KGRAPHEDITOR_H_
#define _KGRAPHEDITOR_H_

#include <kapplication.h>
#include <kparts/mainwindow.h>
#include <ktabwidget.h>
#include <kaction.h>
#include <krecentfilesaction.h>

class QTreeWidget;
class QTreeWidgetItem;

class KToggleAction;

class kgraphviewerPart;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Gael de Chalendar <kleag@free.fr>
 */
class KGraphEditor : public KParts::MainWindow
{
  Q_OBJECT
public:
  /**
    * Default Constructor
    */
  KGraphEditor();

  /**
    * Default Destructor
    */
  virtual ~KGraphEditor();

  /**
    * Use this method to load whatever file/URL you have
    */
  void openUrl(const KUrl& url);
 
  void reloadPreviousFiles();

protected:

  bool queryExit(); 

signals:
  void hide(KParts::Part* part);
  void prepareAddNewElement();
  void prepareAddNewEdge();
  void setReadWrite();
  void saveTo(const QString& fileName);

public slots:
  /**
    * Use this method to load whatever file/URL you have
    */
  void openUrl(const QString& url) {openUrl(KUrl(url));}

  void slotSetActiveGraph( KParts::Part* part);

/*public slots:
  void reloadOnChangeMode_pressed(int value);
  void openInExistingWindowMode_pressed(int value);
  void reopenPreviouslyOpenedFilesMode_pressed(int value);*/
  
private slots:
  void fileNew();
  void fileOpen();
  void fileSave();
  void fileSaveAs();
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
  void slotItemChanged ( QTreeWidgetItem * item, int column );
  void slotEditNewVertex();
  void slotEditNewEdge();

private:
  void setupAccel();
  void setupActions();
    
private:
  QDockWidget* m_leftDockWidget;
  QTreeWidget *m_treeWidget;
  KTabWidget* m_widget;
  KRecentFilesAction* m_rfa;
  KParts::PartManager* m_manager;
  
  KToggleAction *m_toolbarAction;
  KToggleAction *m_statusbarAction;

  QStringList m_openedFiles;
  
  QMap<QWidget*, KParts::Part*> m_tabsPartsMap;
  QMap<QWidget*, QString> m_tabsFilesMap;
  kgraphviewerPart* m_currentPart;
};

#endif // _KGRAPHEDITOR_H_
