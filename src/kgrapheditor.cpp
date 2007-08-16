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


#include "kgrapheditor.h"
#include "kgrapheditorConfigDialog.h"
#include "kgrapheditorsettings.h"
#include "ui_preferencesReload.h"
#include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesReopenPreviouslyOpenedFiles.h"
#include "part/dotgraph.h"
#include "part/kgraphviewer_part.h"

#include <kshortcutsdialog.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <ktabwidget.h>
#include <kparts/partmanager.h>
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
    KParts::MainWindow(),
    m_rfa(0),
    m_currentPart(0)
{
  qDebug() << k_funcinfo << "essai1";
  qDebug() << "essai2";
  // set the shell's ui resource file
  setXMLFile("kgrapheditorui.rc");

  m_widget = new KTabWidget(this);
  m_widget->setHoverCloseButton(true);
  connect(m_widget, SIGNAL(closeRequest(QWidget*)), this, SLOT(close(QWidget*)));
  connect(m_widget, SIGNAL(currentChanged(QWidget*)), this, SLOT(newTabSelectedSlot(QWidget*)));
  
  setCentralWidget(m_widget);

  m_leftDockWidget = new QDockWidget(this);
  m_treeWidget = new QTreeWidget(m_leftDockWidget);
  connect(m_treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
//   m_treeWidget->setItemDelegate(new VariantDelegate(m_treeWidget));
  m_treeWidget->setColumnCount(2);
  m_leftDockWidget->setWidget(m_treeWidget);
  addDockWidget ( Qt::LeftDockWidgetArea, m_leftDockWidget );

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

 // Create a KParts part manager, to handle part activation/deactivation
  m_manager = new KParts::PartManager( this );
  
  // When the manager says the active part changes, the window updates (recreates) the GUI
  connect( m_manager, SIGNAL( activePartChanged( KParts::Part * ) ),
           this, SLOT( createGUI( KParts::Part * ) ) );
    
  // Creates the GUI with a null part to make appear the main app menus and tools
  createGUI(0);
  setupGUI();  
  // apply the saved mainwindow settings, if any, and ask the mainwindow
  // to automatically save settings if changed: window size, toolbar
  // position, icon size, etc.
  setAutoSaveSettings();

  connect( m_manager, SIGNAL( activePartChanged( KParts::Part * ) ),
           this, SLOT( slotSetActiveGraph( KParts::Part * ) ) );
}

KGraphEditor::~KGraphEditor()
{
  kDebug() << k_funcinfo;
}

void KGraphEditor::reloadPreviousFiles()
{
  QStringList previouslyOpenedFiles = KGraphViewerSettings::previouslyOpenedFiles();
  if ( (previouslyOpenedFiles.empty() == false) 
       && (KMessageBox::questionYesNo(this, 
              i18n("Do you want to reload files from previous session ?"),
              i18n("Reload Confirmation"),
              KStandardGuiItem::yes(),
              KStandardGuiItem::no(),
              "reopenPreviouslyOpenedFilesMode"   ) == KMessageBox::Yes) )
  {
    QStringList::const_iterator it, it_end;
    it = previouslyOpenedFiles.begin(); it_end = previouslyOpenedFiles.end();
    for (; it != it_end; it++)
    {
      openUrl(*it);
    }
  }
  
}

void KGraphEditor::openUrl(const KUrl& url)
{
  kDebug() << k_funcinfo << url;
  KLibFactory *factory = KLibLoader::self()->factory("kgraphviewerpart");
  if (factory)
  {
    KParts::ReadOnlyPart* part = static_cast<KParts::ReadOnlyPart*>(factory->create(this, "kgraphviewerpart"));
    
    if (part)
    {
      connect(this,SIGNAL(setReadWrite()),part,SLOT(setReadWrite()));
      emit(setReadWrite());
      
      QString label = url.url().section('/',-1,-1);
      m_widget-> insertTab(part->widget(), QIcon( DesktopIcon("kgraphviewer") ), label);
      m_widget->setCurrentPage(m_widget->indexOf(part->widget()));
      createGUI(part);
      part->openUrl( url );
      m_openedFiles.push_back(url.url());
      m_manager->addPart( part, true );
      m_tabsPartsMap[m_widget->currentPage()] = part;
      m_tabsFilesMap[m_widget->currentPage()] = url.url();
      connect(this,SIGNAL(hide(KParts::Part*)),part,SLOT(slotHide(KParts::Part*)));

    }
  }
  else
  {
    // if we couldn't find our Part, we exit since the Shell by
    // itself can't do anything useful
    KMessageBox::error(this, i18n("Could not find our part."));
    kapp->quit();
    // we return here, cause kapp->quit() only means "exit the
    // next time we enter the event loop...
    return;
  }
}

void KGraphEditor::fileOpen()
{
  kDebug() << k_funcinfo;
  // this slot is called whenever the File->Open menu is selected,
  // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
  // button is clicked
  QStringList file_names = KFileDialog::getOpenFileNames(KUrl(QString::null), QString("*.dot"), 0, QString::null);
  
  if (!file_names.empty())
  {
    foreach (QString fileName, file_names)
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
  kDebug() << k_funcinfo;
  KGraphViewerSettings::setPreviouslyOpenedFiles(m_openedFiles);
  m_rfa->saveEntries(KGlobal::config()->group("kgrapheditor"));

  // 
  //@TODO to port
  //KGraphViewerSettings::writeConfig();
  return true;
}

void KGraphEditor::fileNew()
{
  // this slot is called whenever the File->New menu is selected,
  // the New shortcut is pressed (usually CTRL+N) or the New toolbar
  // button is clicked

  (new KGraphEditor)->show();
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
  if ( KgvConfigurationDialog::showDialog( "settings" ) ) 
    return; 
 
  //KConfigDialog didn't find an instance of this dialog, so lets create it : 
  KPageDialog::FaceType ft = KPageDialog::Auto;
  KgvConfigurationDialog* dialog = new KgvConfigurationDialog( this, "settings", 
                                             KGraphViewerSettings::self(),ft ); 
  
/*  KGraphViewerPreferencesReloadWidget*  reloadWidget =  
      new KGraphViewerPreferencesReloadWidget( 0, "KGraphViewer Settings" );
  if (KGraphViewerSettings::reloadOnChangeMode() == "yes")
  {
    reloadWidget->reloadOnChangeMode->setButton(0);
  }
  else if (KGraphViewerSettings::reloadOnChangeMode() == "no")
  {
    reloadWidget->reloadOnChangeMode->setButton(1);
  }
  else if (KGraphViewerSettings::reloadOnChangeMode() == "ask")
  {
    reloadWidget->reloadOnChangeMode->setButton(2);
  }
  
  connect((QObject*)reloadWidget->reloadOnChangeMode, SIGNAL(clicked(int)), this, SLOT(reloadOnChangeMode_pressed(int)) );
  
  dialog->addPage( reloadWidget, i18n("Reloading"), "kgraphreloadoptions", i18n("Reloading"), true); 
 
  KGraphViewerPreferencesOpenInExistingWindowWidget*  openingWidget =  
    new KGraphViewerPreferencesOpenInExistingWindowWidget( 0, "KGraphViewer Settings" );
  if (KGraphViewerSettings::openInExistingWindowMode() == "yes")
  {
    openingWidget->openInExistingWindowMode->setButton(0);
  }
  else if (KGraphViewerSettings::openInExistingWindowMode() == "no")
  {
    openingWidget->openInExistingWindowMode->setButton(1);
  }
  else if (KGraphViewerSettings::openInExistingWindowMode() == "ask")
  {
    openingWidget->openInExistingWindowMode->setButton(2);
  }
  
  connect((QObject*)openingWidget->openInExistingWindowMode, SIGNAL(clicked(int)), this, SLOT(openInExistingWindowMode_pressed(int)) );
  
  dialog->addPage( openingWidget, i18n("Opening"), "kgraphopeningoptions", i18n("Opening"), true); 
  
  KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget*  reopeningWidget =  
    new KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget( 0, "KGraphViewer Settings" );
  if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "yes")
  {
    reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(0);
  }
  else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "no")
  {
    reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(1);
  }
  else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "ask")
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
//     KGraphViewerSettings::setReloadOnChangeMode("yes");
//     break;
//   case 1:
//     KGraphViewerSettings::setReloadOnChangeMode("no");
//     break;
//   case 2:
//     KGraphViewerSettings::setReloadOnChangeMode("ask");
//     break;
//   default:
//   kError() << "Invalid reload on change mode value: " << value;
//     return;
//   }
//   kDebug() << "emiting";
//   emit(settingsChanged());
//   KGraphViewerSettings::writeConfig();
// }
// 
// void KGraphViewer::openInExistingWindowMode_pressed(int value)
// {
//   std::cerr << "openInExistingWindowMode_pressed " << value << std::endl;
//   switch (value)
//   {
//   case 0:
//     KGraphViewerSettings::setOpenInExistingWindowMode("yes");
//     break;
//   case 1:
//     KGraphViewerSettings::setOpenInExistingWindowMode("no");
//     break;
//   case 2:
//     KGraphViewerSettings::setOpenInExistingWindowMode("ask");
//     break;
//   default:
//   kError() << "Invalid OpenInExistingWindow value: " << value << endl;
//     return;
//   }
// 
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphViewerSettings::writeConfig();
// }
// 
// void KGraphViewer::reopenPreviouslyOpenedFilesMode_pressed(int value)
// {
//   std::cerr << "reopenPreviouslyOpenedFilesMode_pressed " << value << std::endl;
//   switch (value)
//   {
//   case 0:
//     KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("yes");
//     break;
//   case 1:
//     KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("no");
//     break;
//   case 2:
//     KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("ask");
//     break;
//   default:
//   kError() << "Invalid ReopenPreviouslyOpenedFilesMode value: " << value << endl;
//     return;
//   }
// 
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphViewerSettings::writeConfig();
// }


void KGraphEditor::slotURLSelected(const KUrl& url)
{
  openUrl(url);
}

void KGraphEditor::close(QWidget* tab)
{
  m_openedFiles.remove(m_tabsFilesMap[tab]);
  m_widget->removePage(tab);
  tab->hide();
  KParts::Part* part = m_tabsPartsMap[tab];
  m_manager->removePart(part);
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

void KGraphEditor::newTabSelectedSlot(QWidget* tab)
{
//   kDebug() << k_funcinfo << tab;
  emit(hide((KParts::Part*)(m_manager->activePart())));
  if (!m_tabsPartsMap.isEmpty())
  {
    m_manager->setActivePart(m_tabsPartsMap[tab]);
  }
}

void KGraphEditor::slotSetActiveGraph( KParts::Part* part)
{
  kDebug() << k_funcinfo;
  if (m_currentPart != 0)
  {
    disconnect(this,SIGNAL(prepareAddNewElement()),part,SLOT(prepareAddNewElement()));
    disconnect(this,SIGNAL(prepareAddNewEdge()),part,SLOT(prepareAddNewEdge()));
    disconnect(this,SIGNAL(saveTo(const QString&)),part,SLOT(saveTo(const QString&)));
  }
  m_currentPart = ((kgraphviewerPart*) part);
  m_treeWidget->clear();
  if (m_currentPart == 0)
  {
    return;
  }
  connect(this,SIGNAL(prepareAddNewElement()),part,SLOT(prepareAddNewElement()));
  connect(this,SIGNAL(prepareAddNewEdge()),part,SLOT(prepareAddNewEdge()));
  connect(this,SIGNAL(saveTo(const QString&)),part,SLOT(saveTo(const QString&)));
  DotGraph* graph = m_currentPart->graph();
  QList<QTreeWidgetItem *> items;
  GraphNodeMap& nodesMap = graph->nodes();
  foreach (GraphNode* node, nodesMap)
  {
    kDebug() << k_funcinfo << "new item " << node->id();
    QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(node->id()));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    foreach (QString attrib, node->attributes().keys())
    {
      if (attrib != "_draw_" && attrib != "_ldraw_")
      {
        QStringList list(attrib);
        list << node->attributes()[attrib];
        QTreeWidgetItem* child = new QTreeWidgetItem((QTreeWidget*)0, list);
        child->setFlags(child->flags() | Qt::ItemIsEditable);
        item->addChild(child);
      }
    }
    items.append(item);
  }
  kDebug() << k_funcinfo << "inserting";
  m_treeWidget->insertTopLevelItems(0, items);
}

void KGraphEditor::slotItemChanged ( QTreeWidgetItem * item, int column )
{
  kDebug() << k_funcinfo;
  DotGraph* graph = m_currentPart->graph();
  /* values column */
  if (column == 1)
  {
    /* there is a parent ; it is an attribute line */
    if (item->parent() != 0)
    {
      QString nodeLabel = item->parent()->text(0);
      QString attributeName = item->text(0);
      QString attributeValue = item->text(1);
      graph->nodes()[nodeLabel]->attributes()[attributeName] = attributeValue;
    }
  }
}

void KGraphEditor::slotEditNewVertex()
{
  kDebug() << k_funcinfo;
  if (m_currentPart == 0)
  {
    return;
  }
  emit(prepareAddNewElement());
}

void KGraphEditor::slotEditNewEdge()
{
  kDebug() << k_funcinfo;
  if (m_currentPart == 0)
  {
    return;
  }
  emit(prepareAddNewEdge());
}

#include "kgrapheditor.moc"
