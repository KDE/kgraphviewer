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


#ifndef _KGRAPHEDITOR_H_
#define _KGRAPHEDITOR_H_

#include <kapplication.h>
#include <kparts/mainwindow.h>
#include <ktabwidget.h>
#include <kaction.h>
#include <krecentfilesaction.h>

#include <graphviz/gvc.h>

class QTreeWidget;
class QTreeWidgetItem;

class KToggleAction;

class kgraphviewerPart;
class KGraphEditorNodesTreeWidget;
class KGraphEditorElementTreeWidget;

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

Q_SIGNALS:
  void hide(KParts::Part* part);
  void prepareAddNewElement(QMap<QString,QString> attribs);
  void prepareAddNewEdge(QMap<QString,QString> attribs);
  void setReadWrite();
  void saveTo(const QString& fileName);

  void selectNode(const QString&);
  void removeNode(const QString&);
  void removeElement(const QString&);
  void addAttribute(const QString&);
  void removeAttribute(const QString&,const QString&);
  void update();
  void saddNewEdge(QString src, QString tgt, QMap<QString,QString> attribs);

public Q_SLOTS:
  /**
    * Use this method to load whatever file/URL you have
    */
  void openUrl(const QString& url) {openUrl(KUrl(url));}

  void slotSetActiveGraph( KParts::Part* part);

  void slotGraphLoaded();
  
  void slotRemoveNode(const QString&);
  void slotAddAttribute(const QString&);
  void slotRemoveAttribute(const QString&,const QString&);

  void slotNewElementItemChanged(QTreeWidgetItem*,int);
  void slotAddNewElementAttribute(const QString&);
  void slotRemoveNewElementAttribute(const QString&);

  void slotNewNodeAdded(const QString& id);
  void slotNewEdgeAdded(const QString& ids, const QString& idt);
  /*public slots:
  void reloadOnChangeMode_pressed(int value);
  void openInExistingWindowMode_pressed(int value);
  void reopenPreviouslyOpenedFilesMode_pressed(int value);*/
  void slotRemoveElement(const QString& id);
  void slotSelectionIs(const QList<QString>&, const QPoint&p);
  void slotNewEdgeFinished( const QString&, const QString&, const QMap<QString, QString>&);

private Q_SLOTS:
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
  void slotItemClicked ( QTreeWidgetItem * item, int column );
  void slotEditNewVertex();
  void slotEditNewEdge();

  void slotParsingModeExternalToggled(bool value);
  void slotParsingModeInternalToggled(bool value);
  
  void slotHoverEnter(const QString&);
  void slotHoverLeave(const QString&);

  KParts::ReadOnlyPart* slotNewGraph();
  
private:
  void setupAccel();
  void setupActions();
    
private:
  KGraphEditorNodesTreeWidget* m_treeWidget;
  KGraphEditorElementTreeWidget* m_newElementAttributesWidget;
  KTabWidget* m_widget;
  KRecentFilesAction* m_rfa;
  KParts::PartManager* m_manager;
  
  KToggleAction *m_toolbarAction;
  KToggleAction *m_statusbarAction;

  QStringList m_openedFiles;
  
  QMap<QWidget*, KParts::Part*> m_tabsPartsMap;
  QMap<QWidget*, QString> m_tabsFilesMap;
  kgraphviewerPart* m_currentPart;

  QMap<QString, QString> m_newElementAttributes;

  QString m_currentTreeWidgetItemText;
};

#endif // _KGRAPHEDITOR_H_
