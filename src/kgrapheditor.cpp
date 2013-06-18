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


#include "kgrapheditor.h"
#include "kgrapheditorConfigDialog.h"
#include "kgrapheditorsettings.h"
#include "KGraphEditorNodesTreeWidget.h"
#include "KGraphEditorElementTreeWidget.h"
#include "ui_preferencesReload.h"
#include "ui_preferencesParsing.h"
#include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesReopenPreviouslyOpenedFiles.h"
#include "part/dotgraph.h"
#include "part/dotgraphview.h"

#include <kshortcutsdialog.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <ktabwidget.h>
#include <kedittoolbar.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfigdialog.h>
#include <kiconloader.h>
#include <krecentfilesaction.h>
#include <ktoolbar.h>
#include <KActionCollection>

#include <QtDBus/QtDBus>
#include <QDockWidget>
#include <QTreeWidget>

#include <iostream>

using namespace KGraphViewer;

KGraphEditor::KGraphEditor() :
    KXmlGuiWindow(), 
    m_rfa(0),
    m_currentPart(0)
{
  // set the shell's ui resource file
  setXMLFile("kgrapheditorui.rc");

  m_widget = new KTabWidget(this);
  m_widget->setHoverCloseButton(true);
  connect(m_widget, SIGNAL(closeRequest(QWidget*)), this, SLOT(close(QWidget*)));
  connect(m_widget, SIGNAL(currentChanged(QWidget*)), this, SLOT(newTabSelectedSlot(QWidget*)));
  
  setCentralWidget(m_widget);

  QDockWidget* topLeftDockWidget = new QDockWidget(this);
  m_treeWidget = new KGraphEditorNodesTreeWidget(topLeftDockWidget);
  connect(m_treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),
          this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
  connect(m_treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this,SLOT(slotItemClicked(QTreeWidgetItem*,int)));
  connect(m_treeWidget, SIGNAL(removeNode(const QString&)),
          this, SLOT(slotRemoveNode(const QString&)));
  connect(m_treeWidget, SIGNAL(addAttribute(const QString&)),
          this, SLOT(slotAddAttribute(const QString&)));
  connect(m_treeWidget, SIGNAL(removeAttribute(const QString&,const QString&)),
          this, SLOT(slotRemoveAttribute(const QString&,const QString&)));

//   m_treeWidget->setItemDelegate(new VariantDelegate(m_treeWidget));
  m_treeWidget->setColumnCount(2);
  topLeftDockWidget->setWidget(m_treeWidget);
  addDockWidget ( Qt::LeftDockWidgetArea, topLeftDockWidget );

  QDockWidget* bottomLeftDockWidget = new QDockWidget(this);
  m_newElementAttributesWidget = new KGraphEditorElementTreeWidget(bottomLeftDockWidget);
  connect(m_newElementAttributesWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),
          this,SLOT(slotNewElementItemChanged(QTreeWidgetItem*,int)));
  connect(m_newElementAttributesWidget, SIGNAL(addAttribute()),
          this, SLOT(slotAddNewElementAttribute()));
  connect(m_newElementAttributesWidget, SIGNAL(removeAttribute(const QString&)),
          this, SLOT(slotRemoveNewElementAttribute(const QString&)));
  m_newElementAttributesWidget->setColumnCount(2);
  bottomLeftDockWidget->setWidget(m_newElementAttributesWidget);
  addDockWidget ( Qt::LeftDockWidgetArea, bottomLeftDockWidget );


  if (QDBusConnection::sessionBus().registerService( "org.kde.kgrapheditor" ))
  {
    kDebug() << "Service Registered successfuly";
    QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);
    
  }
  else
  {
    kDebug() << "Failed to register service...";
  }

  // then, setup our actions
  setupActions();

  // and a status bar
  statusBar()->show();
  createStandardStatusBarAction();

  // this routine will find and load our Part.  it finds the Part by
  // name which is a bad idea usually.. but it's alright in this
  // case since our Part is made for this Shell

  // Creates the GUI with a null part to make appear the main app menus and tools
  createGUI(0);
  setupGUI();  
  // apply the saved mainwindow settings, if any, and ask the mainwindow
  // to automatically save settings if changed: window size, toolbar
  // position, icon size, etc.
  setAutoSaveSettings();

}

KGraphEditor::~KGraphEditor()
{
  kDebug() ;
}

void KGraphEditor::reloadPreviousFiles()
{
  QStringList previouslyOpenedFiles = KGraphEditorSettings::previouslyOpenedFiles();
  if ( (previouslyOpenedFiles.empty() == false) 
       && (KMessageBox::questionYesNo(this, 
              i18n("Do you want to reload files from previous session?"),
              i18n("Session Restore"),
              KStandardGuiItem::yes(),
              KStandardGuiItem::no(),
              "reopenPreviouslyOpenedFilesMode"   ) == KMessageBox::Yes) )
  {
    QStringList::const_iterator it, it_end;
    it = previouslyOpenedFiles.constBegin(); it_end = previouslyOpenedFiles.constEnd();
    for (; it != it_end; it++)
    {
      openUrl(*it);
    }
  }
  
}

DotGraphView*  KGraphEditor::slotNewGraph()
{
  kDebug();
  DotGraphView* view = new DotGraphView(actionCollection(), m_widget);
  view->initEmpty();
  view->setReadWrite();

    m_widget-> insertTab(view, QIcon( DesktopIcon("kgraphviewer") ), "");
    m_widget->setCurrentPage(m_widget->indexOf(view));
//     createGUI(view);

    m_tabsPartsMap[m_widget->currentPage()] = view;
    m_tabsFilesMap[m_widget->currentPage()] = "";
//     connect(this,SIGNAL(hide(KParts::Part*)),view,SLOT(slotHide(KParts::Part*)));
  return view;
}

void KGraphEditor::openUrl(const KUrl& url)
{
  kDebug() << url;
  DotGraphView* part = slotNewGraph();
  
//   (KGraphEditorSettings::parsingMode() == "external")
//     ?kgv->setLayoutMethod(KGraphViewerInterface::ExternalProgram)
//     :kgv->setLayoutMethod(KGraphViewerInterface::InternalLibrary);

  QString label = url.url().section('/',-1,-1);
  // @TODO set label
  m_widget-> insertTab(part, QIcon( DesktopIcon("kgraphviewer") ), label);
  m_widget->setCurrentPage(m_widget->indexOf(part));
  m_tabsFilesMap[m_widget->currentPage()] = url.url();
  part->loadLibrary(url.url());

  m_openedFiles.push_back(url.url());
}

void KGraphEditor::fileOpen()
{
  kDebug() ;
  // this slot is called whenever the File->Open menu is selected,
  // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
  // button is clicked
  QStringList file_names = KFileDialog::getOpenFileNames(KUrl(QString()), QString("*.dot"), 0, QString::null);
  
  if (!file_names.empty())
  {
    foreach (const QString &fileName, file_names)
    {
      if (m_rfa != 0)
      {
        m_rfa->addUrl(fileName);
      }
      openUrl(fileName);
    }
  }
}


void KGraphEditor::setupActions()
{
  // create our actions

  actionCollection()->addAction( KStandardAction::New, "file_new", this, SLOT( fileNew() ) );
  actionCollection()->addAction( KStandardAction::Open, "file_open", this, SLOT( fileOpen() ) );
  m_rfa = (KRecentFilesAction*) actionCollection()->addAction(KStandardAction::OpenRecent, "file_open_recent", this, SLOT( slotURLSelected(const KUrl&) ) );
  m_rfa->loadEntries(KGlobal::config()->group("kgrapheditor"));
  actionCollection()->addAction( KStandardAction::Save, "file_save", this, SLOT( fileSave() ) );
  actionCollection()->addAction( KStandardAction::SaveAs, "file_save_as", this, SLOT( fileSaveAs() ) );

  actionCollection()->addAction( KStandardAction::Quit, "file_quit", this, SLOT( quit() ) );

  m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), this);

  actionCollection()->addAction( KStandardAction::KeyBindings, "options_configure_keybinding", this, SLOT( optionsConfigureKeys() ) );
//   KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), this);
  actionCollection()->addAction( KStandardAction::ConfigureToolbars, "options_configure_toolbars", this, SLOT( optionsConfigureToolbars() ) );
  actionCollection()->addAction( KStandardAction::Preferences, "options_configure", this, SLOT( optionsConfigure() ) );

  QAction* edit_new_vertex = actionCollection()->addAction( "edit_new_vertex" );
  edit_new_vertex->setText(i18n("Create a New Vertex"));
  edit_new_vertex->setIcon(QPixmap(KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newnode.png")));
  connect( edit_new_vertex, SIGNAL(triggered(bool)), this, SLOT( slotEditNewVertex() ) );

  QAction* edit_new_edge = actionCollection()->addAction( "edit_new_edge" );
  edit_new_edge->setText(i18n("Create a New Edge"));
  edit_new_edge->setIcon(QPixmap(KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newedge.png")));
  connect( edit_new_edge, SIGNAL(triggered(bool)), this, SLOT( slotEditNewEdge() ) );
}

bool KGraphEditor::queryExit()
{
  kDebug() ;
  KGraphEditorSettings::setPreviouslyOpenedFiles(m_openedFiles);
  m_rfa->saveEntries(KGlobal::config()->group("kgrapheditor"));

  KGraphEditorSettings::self()->writeConfig();
  return true;
}

void KGraphEditor::fileNew()
{
  // this slot is called whenever the File->New menu is selected,
  // the New shortcut is pressed (usually CTRL+N) or the New toolbar
  // button is clicked

  slotNewGraph();
}


void KGraphEditor::optionsShowToolbar()
{
  // this is all very cut and paste code for showing/hiding the
  // toolbar
  if (m_toolbarAction->isChecked())
      toolBar()->show();
  else
      toolBar()->hide();
}

void KGraphEditor::optionsShowStatusbar()
{
  // this is all very cut and paste code for showing/hiding the
  // statusbar
  if (m_statusbarAction->isChecked())
      statusBar()->show();
  else
      statusBar()->hide();
}

void KGraphEditor::optionsConfigureKeys()
{
  KShortcutsDialog::configure(actionCollection());
}

void KGraphEditor::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("kgrapheditor") );

  // use the standard toolbar editor
  KEditToolBar dlg(factory());
  connect(&dlg, SIGNAL(newToolbarConfig()),
          this, SLOT(applyNewToolbarConfig()));
  dlg.exec();
}

void KGraphEditor::optionsConfigure()
{
  //An instance of your dialog could be already created and could be cached, 
  //in which case you want to display the cached dialog instead of creating 
  //another one 
  if ( KgeConfigurationDialog::showDialog( "settings" ) )
    return; 
 
  //KConfigDialog didn't find an instance of this dialog, so lets create it : 
  KPageDialog::FaceType ft = KPageDialog::Auto;
  KgeConfigurationDialog* dialog = new KgeConfigurationDialog( this, "settings",
                                             KGraphEditorSettings::self(),ft );

  Ui::KGraphViewerPreferencesParsingWidget*  parsingWidget = dialog->m_parsingWidget;
  kDebug() << KGraphEditorSettings::parsingMode();
  if (KGraphEditorSettings::parsingMode() == "external")
  {
    parsingWidget->external->setChecked(true);
  }
  else if (KGraphEditorSettings::parsingMode() == "internal")
  {
    parsingWidget->internal->setChecked(true);
  }
  connect((QObject*)parsingWidget->external, SIGNAL(toggled(bool)), this, SLOT(slotParsingModeExternalToggled(bool)) );
  connect((QObject*)parsingWidget->internal, SIGNAL(toggled(bool)), this, SLOT(slotParsingModeInternalToggled(bool)) );

                                             
/*  KGraphViewerPreferencesReloadWidget*  reloadWidget =  
      new KGraphViewerPreferencesReloadWidget( 0, "KGraphViewer Settings" );
  if (KGraphEditorSettings::reloadOnChangeMode() == "yes")
  {
    reloadWidget->reloadOnChangeMode->setButton(0);
  }
  else if (KGraphEditorSettings::reloadOnChangeMode() == "no")
  {
    reloadWidget->reloadOnChangeMode->setButton(1);
  }
  else if (KGraphEditorSettings::reloadOnChangeMode() == "ask")
  {
    reloadWidget->reloadOnChangeMode->setButton(2);
  }
  
  connect((QObject*)reloadWidget->reloadOnChangeMode, SIGNAL(clicked(int)), this, SLOT(reloadOnChangeMode_pressed(int)) );
  
  dialog->addPage( reloadWidget, i18n("Reloading"), "kgraphreloadoptions", i18n("Reloading"), true); 
 
  KGraphViewerPreferencesOpenInExistingWindowWidget*  openingWidget =  
    new KGraphViewerPreferencesOpenInExistingWindowWidget( 0, "KGraphViewer Settings" );
  if (KGraphEditorSettings::openInExistingWindowMode() == "yes")
  {
    openingWidget->openInExistingWindowMode->setButton(0);
  }
  else if (KGraphEditorSettings::openInExistingWindowMode() == "no")
  {
    openingWidget->openInExistingWindowMode->setButton(1);
  }
  else if (KGraphEditorSettings::openInExistingWindowMode() == "ask")
  {
    openingWidget->openInExistingWindowMode->setButton(2);
  }
  
  connect((QObject*)openingWidget->openInExistingWindowMode, SIGNAL(clicked(int)), this, SLOT(openInExistingWindowMode_pressed(int)) );
  
  dialog->addPage( openingWidget, i18n("Opening"), "kgraphopeningoptions", i18n("Opening"), true); 
  
  KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget*  reopeningWidget =  
    new KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget( 0, "KGraphViewer Settings" );
  if (KGraphEditorSettings::reopenPreviouslyOpenedFilesMode() == "yes")
  {
    reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(0);
  }
  else if (KGraphEditorSettings::reopenPreviouslyOpenedFilesMode() == "no")
  {
    reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(1);
  }
  else if (KGraphEditorSettings::reopenPreviouslyOpenedFilesMode() == "ask")
  {
    reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(2);
  }
  
  connect((QObject*)reopeningWidget->reopenPreviouslyOpenedFilesMode, SIGNAL(clicked(int)), this, SLOT(reopenPreviouslyOpenedFilesMode_pressed(int)) );

  dialog->addPage( reopeningWidget, i18n("Session Management"), "kgraphreopeningoptions", i18n("Session Management"), true); 
  */
//   connect(dialog, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));

  dialog->show();
}

void KGraphEditor::applyNewToolbarConfig()
{
  applyMainWindowSettings(KGlobal::config()->group("kgrapheditor"));
}

// void KGraphViewer::reloadOnChangeMode_pressed(int value)
// {
//   kDebug() << "reloadOnChangeMode_pressed " << value;
//   switch (value)
//   {
//   case 0:
//     KGraphEditorSettings::setReloadOnChangeMode("yes");
//     break;
//   case 1:
//     KGraphEditorSettings::setReloadOnChangeMode("no");
//     break;
//   case 2:
//     KGraphEditorSettings::setReloadOnChangeMode("ask");
//     break;
//   default:
//   kError() << "Invalid reload on change mode value: " << value;
//     return;
//   }
//   kDebug() << "emiting";
//   emit(settingsChanged());
//   KGraphEditorSettings::writeConfig();
// }
// 
// void KGraphViewer::openInExistingWindowMode_pressed(int value)
// {
//   std::cerr << "openInExistingWindowMode_pressed " << value << std::endl;
//   switch (value)
//   {
//   case 0:
//     KGraphEditorSettings::setOpenInExistingWindowMode("yes");
//     break;
//   case 1:
//     KGraphEditorSettings::setOpenInExistingWindowMode("no");
//     break;
//   case 2:
//     KGraphEditorSettings::setOpenInExistingWindowMode("ask");
//     break;
//   default:
//   kError() << "Invalid OpenInExistingWindow value: " << value << endl;
//     return;
//   }
// 
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphEditorSettings::writeConfig();
// }
// 
// void KGraphViewer::reopenPreviouslyOpenedFilesMode_pressed(int value)
// {
//   std::cerr << "reopenPreviouslyOpenedFilesMode_pressed " << value << std::endl;
//   switch (value)
//   {
//   case 0:
//     KGraphEditorSettings::setReopenPreviouslyOpenedFilesMode("yes");
//     break;
//   case 1:
//     KGraphEditorSettings::setReopenPreviouslyOpenedFilesMode("no");
//     break;
//   case 2:
//     KGraphEditorSettings::setReopenPreviouslyOpenedFilesMode("ask");
//     break;
//   default:
//   kError() << "Invalid ReopenPreviouslyOpenedFilesMode value: " << value << endl;
//     return;
//   }
// 
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphEditorSettings::writeConfig();
// }


void KGraphEditor::slotURLSelected(const KUrl& url)
{
  openUrl(url);
}

void KGraphEditor::close(QWidget* tab)
{
  m_openedFiles.removeAll(m_tabsFilesMap[tab]);
  m_widget->removePage(tab);
  tab->hide();
  DotGraphView* part = m_tabsPartsMap[tab];
  m_tabsPartsMap.remove(tab);
  m_tabsFilesMap.remove(tab);
  delete part; part=0;
/*  delete tab;
  tab = 0;*/
}

void KGraphEditor::close()
{
  QWidget* currentPage = m_widget->currentPage();
  if (currentPage != 0)
  {
    close(currentPage);
  }
}

void KGraphEditor::fileSave()
{
  QWidget* currentPage = m_widget->currentPage();
  if (currentPage != 0)
  {
    emit(saveTo(QUrl(m_tabsFilesMap[currentPage]).path()));
  }
}

void KGraphEditor::fileSaveAs()
{
  QWidget* currentPage = m_widget->currentPage();
  if (currentPage != 0)
  {
    QString fileName = KFileDialog::getSaveFileName(KUrl(),
                "*.dot", currentPage,
                i18n("Save current graph to..."));
    m_tabsFilesMap[currentPage] = fileName;
    emit(saveTo(fileName));
  }
}

void KGraphEditor::newTabSelectedSlot(QWidget* tab)
{
//   kDebug() << tab;
//   emit(hide((KParts::Part*)(m_manager->activePart())));
  if (!m_tabsPartsMap.isEmpty())
  {
//     m_manager->setActivePart(m_tabsPartsMap[tab]);
  }
}

void KGraphEditor::slotSetActiveGraph( DotGraphView* part)
{
  kDebug();
  if (m_currentPart != 0)
  {
    disconnect(this,SIGNAL(prepareAddNewElement(QMap<QString,QString>)),m_currentPart,SLOT(prepareAddNewElement(QMap<QString,QString>)));
    disconnect(this,SIGNAL(prepareAddNewEdge(QMap<QString,QString>)),m_currentPart,SLOT(prepareAddNewEdge(QMap<QString,QString>)));
    disconnect(this,SIGNAL(saveTo(QString)),m_currentPart,SLOT(saveTo(QString)));
    disconnect(this,SIGNAL(removeNode(QString)),m_currentPart,SLOT(slotRemoveNode(QString)));
    disconnect(this,SIGNAL(addAttribute(QString)),m_currentPart,SLOT(slotAddAttribute(QString)));
    disconnect(this,SIGNAL(removeAttribute(QString,QString)),m_currentPart,SLOT(slotRemoveAttribute(QString,QString)));
    disconnect(this,SIGNAL(update()),m_currentPart,SLOT(slotUpdate()));
    disconnect(this,SIGNAL(selectNode(QString)),m_currentPart,SLOT(slotSelectNode(QString)));
    disconnect(this,SIGNAL(saddNewEdge(QString,QString,QMap<QString,QString>)),
           m_currentPart,SLOT(slotAddNewEdge(QString,QString,QMap<QString,QString>)));
    disconnect(this,SIGNAL(renameNode(QString,QString)),
            m_currentPart,SLOT(slotRenameNode(QString,QString)));
    disconnect(this,SIGNAL(setAttribute(QString,QString,QString)),m_currentPart,SLOT(slotSetAttribute(QString,QString,QString)));
  }
  m_currentPart = part;
  m_treeWidget->clear();
  if (m_currentPart == 0)
  {
    return;
  }
  connect(this,SIGNAL(prepareAddNewElement(QMap<QString,QString>)),part,SLOT(prepareAddNewElement(QMap<QString,QString>)));
  connect(this,SIGNAL(prepareAddNewEdge(QMap<QString,QString>)),part,SLOT(prepareAddNewEdge(QMap<QString,QString>)));
  connect(this,SIGNAL(saveTo(QString)),part,SLOT(saveTo(QString)));
  connect(this,SIGNAL(removeNode(QString)),part,SLOT(slotRemoveNode(QString)));
  connect(this,SIGNAL(addAttribute(QString)),part,SLOT(slotAddAttribute(QString)));
  connect(this,SIGNAL(removeAttribute(QString,QString)),part,SLOT(slotRemoveAttribute(QString,QString)));
  connect(this,SIGNAL(update()),part,SLOT(slotUpdate()));
  connect(this,SIGNAL(selectNode(QString)),part,SLOT(slotSelectNode(QString)));
  connect( this,SIGNAL(removeElement(QString)),m_currentPart,SLOT(slotRemoveElement(QString)));
  connect(this,SIGNAL(saddNewEdge(QString,QString,QMap<QString,QString>)),m_currentPart,SLOT(slotAddNewEdge(QString,QString,QMap<QString,QString>)));
  connect(this,SIGNAL(renameNode(QString,QString)),m_currentPart,SLOT(slotRenameNode(QString,QString)));
  connect(this,SIGNAL(setAttribute(QString,QString,QString)),m_currentPart,SLOT(slotSetAttribute(QString,QString,QString)));
  
  QList<QString> nodesIds;//TODO = m_currentPart->nodesIds();
  QList<QTreeWidgetItem *> items;
  foreach (const QString& nodeId, nodesIds)
  {
    kDebug()<< "new item " << nodeId;
    QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(nodeId));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    QMap<QString,QString> attributes;//TODO = m_currentPart->nodeAtributes(nodeId);
    foreach (const QString& attrib, attributes.keys())
    {
      if (attrib != "_draw_" && attrib != "_ldraw_")
      {
        QStringList list(attrib);
        list << attributes[attrib];
        QTreeWidgetItem* child = new QTreeWidgetItem((QTreeWidget*)0, list);
        child->setFlags(child->flags() | Qt::ItemIsEditable);
        item->addChild(child);
      }
    }
    items.append(item);
  }
  kDebug() << "inserting";
  m_treeWidget->insertTopLevelItems(0, items);


  connect( m_currentPart, SIGNAL( graphLoaded() ),
           this, SLOT( slotGraphLoaded() ) );

  connect( m_currentPart, SIGNAL( newNodeAdded(const QString&) ),
          this, SLOT( slotNewNodeAdded(const QString&) ) );

  connect( m_currentPart, SIGNAL( newEdgeAdded(const QString&, const QString&) ),
            this, SLOT( slotNewEdgeAdded(const QString&, const QString&) ) );

  connect( m_currentPart, SIGNAL( removeElement(const QString&) ),
            this, SLOT( slotRemoveElement(const QString&) ) );

  connect( m_currentPart, SIGNAL( selectionIs(const QList<QString>&, const QPoint&) ),
            this, SLOT( slotSelectionIs(const QList<QString>&, const QPoint&) ) );

  connect( m_currentPart, SIGNAL( newEdgeFinished( const QString&, const QString&, const QMap<QString, QString>&) ),
            this, SLOT( slotNewEdgeFinished( const QString&, const QString&, const QMap<QString, QString>&) ) );

  connect( m_currentPart, SIGNAL( hoverEnter(const QString&) ),
           this, SLOT( slotHoverEnter(const QString&) ) );
            
  connect( m_currentPart, SIGNAL( hoverLeave (const QString&) ),
          this, SLOT( slotHoverLeave(const QString&) ) );
}

void KGraphEditor::slotNewNodeAdded(const QString& id)
{
  kDebug() << id;
  update();
}

void KGraphEditor::slotNewEdgeAdded(const QString& ids, const QString& idt)
{
  kDebug() << ids << idt;
  update();
}

void KGraphEditor::slotNewEdgeFinished( const QString& srcId, const QString& tgtId, const QMap<QString, QString>&attribs)
{
  kDebug() << srcId << tgtId << attribs;
  emit saddNewEdge(srcId,tgtId,attribs);
  update();
}

void KGraphEditor::slotGraphLoaded()
{
  kDebug();
  disconnect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
  disconnect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           this,SLOT(slotItemClicked(QTreeWidgetItem*,int)));

  QList<QString> nodesIds;//TODO = m_currentPart->nodesIds();
  QList<QTreeWidgetItem *> items;
  foreach (const QString& nodeId, nodesIds)
  {
    kDebug()<< "item " << nodeId;
    QTreeWidgetItem* item;
    QList<QTreeWidgetItem*> existingItems = m_treeWidget->findItems(nodeId,Qt::MatchRecursive|Qt::MatchExactly);
    if (existingItems.isEmpty())
    {
      item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(nodeId));
      items.append(item);
    }
    else
    {
      item = existingItems[0];
    }
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    QMap<QString,QString> attributes; //TODO = m_currentPart->nodeAtributes(nodeId);
    QList<QString> keys = attributes.keys();
    for (int i=0; i < item->childCount();i++)
    {
      if (keys.contains(item->child(i)->text(0)))
      {
        item->child(i)->setText(1,attributes[item->child(i)->text(0)]);
        keys.removeAll(item->child(i)->text(0));
      }
    }
    foreach (const QString &attrib, keys)
    {
      if (attrib != "_draw_" && attrib != "_ldraw_")
      {
        QStringList list(attrib);
        list << attributes[attrib];
        QTreeWidgetItem* child = new QTreeWidgetItem((QTreeWidget*)0, list);
        child->setFlags(child->flags() | Qt::ItemIsEditable);
        item->addChild(child);
      }
    }
}
  kDebug() << "inserting";
  m_treeWidget->insertTopLevelItems(0, items);
  connect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
  connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           this,SLOT(slotItemClicked(QTreeWidgetItem*,int)));
}

void KGraphEditor::slotItemChanged ( QTreeWidgetItem * item, int column )
{
  kDebug() ;
  /* values column */
  if (column == 0)
  {
    QString oldNodeName = m_currentTreeWidgetItemText;
    QString newNodeName = item->text(0);
    emit(renameNode(oldNodeName,newNodeName));
  }
  else if (column == 1)
  {
    /* there is a parent ; it is an attribute line */
    if (item->parent() != 0)
    {
      QString nodeLabel = item->parent()->text(0);
      QString attributeName = item->text(0);
      QString attributeValue = item->text(1);
      emit (setAttribute(nodeLabel,attributeName,attributeValue));
    }
  }
  emit update();
}

void KGraphEditor::slotItemClicked ( QTreeWidgetItem * item, int column )
{
  kDebug() << column;
  m_currentTreeWidgetItemText = item->text(0);

  QString nodeName = item->parent() != 0 ?
                        item->parent()->text(0) :
                        item->text(0);
  emit selectNode(nodeName);
}

void KGraphEditor::slotEditNewVertex()
{
  kDebug() ;
  if (m_currentPart == 0)
  {
    return;
  }
  emit(prepareAddNewElement(m_newElementAttributes));
}

void KGraphEditor::slotEditNewEdge()
{
  kDebug() ;
  if (m_currentPart == 0)
  {
    return;
  }
  emit(prepareAddNewEdge(m_newElementAttributes));
}

void KGraphEditor::slotRemoveNode(const QString& nodeName)
{
  emit removeNode(nodeName);
  emit update();
}

void KGraphEditor::slotAddAttribute(const QString& attribName)
{
  emit addAttribute(attribName);
  emit update();
}

void KGraphEditor::slotRemoveAttribute(const QString& nodeName, const QString& attribName)
{
  kDebug();
  emit removeAttribute(nodeName,attribName);
  emit update();
}

void KGraphEditor::slotNewElementItemChanged(QTreeWidgetItem* item ,int column)
{
  kDebug();
  if (column == 0)
  {
    kError() << "Item id change not handled";
    return;
  }
  else if (column == 1)
  {
    m_newElementAttributes[item->text(0)] = item->text(1);
  }
  else
  {
    kError() << "Unknonw column" << column;
    return;
  }
}

void KGraphEditor::slotAddNewElementAttribute(const QString& attrib)
{
  kDebug();
  m_newElementAttributes[attrib] = QString();
}

void KGraphEditor::slotRemoveNewElementAttribute(const QString& attrib)
{
  kDebug();
  m_newElementAttributes.remove(attrib);
}

void KGraphEditor::slotRemoveElement(const QString& id)
{
  kDebug() << id;
  m_treeWidget->slotRemoveElement(id);
  emit(removeElement(id));
}

void KGraphEditor::slotSelectionIs(const QList<QString>& elements , const QPoint& p)
{
  kDebug();
  Q_UNUSED(p);
  QList<QTreeWidgetItem*> items = m_treeWidget->selectedItems();
  foreach (QTreeWidgetItem* item, items)
  {
    item->setSelected(false);
  }
  foreach (const QString &elementName, elements)
  {
    QList<QTreeWidgetItem*> items = m_treeWidget->findItems(elementName,Qt::MatchExactly,0);
    foreach (QTreeWidgetItem* item, items)
    {
      item->setSelected(true);
    }
  }
}

void KGraphEditor::slotParsingModeExternalToggled(bool value)
{
  kDebug();
  if (value)
  {
    KGraphEditorSettings::setParsingMode("external");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphEditorSettings::self()->writeConfig();
}

void KGraphEditor::slotParsingModeInternalToggled(bool value)
{
  kDebug();
  if (value)
  {
    KGraphEditorSettings::setParsingMode("internal");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphEditorSettings::self()->writeConfig();
}

void KGraphEditor::slotHoverEnter(const QString& id)
{
  kDebug() << id;
  statusBar()->showMessage(id);
}

void KGraphEditor::slotHoverLeave(const QString& id)
{
  kDebug() << id;
  statusBar()->showMessage("");
}


#include "kgrapheditor.moc"
