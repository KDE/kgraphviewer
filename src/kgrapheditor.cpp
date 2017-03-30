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
#include "part/kgraphviewer_part.h"

#include <kshortcutsdialog.h>
#include <QFileDialog>
#include <kconfig.h>
#include <QUrl>
#include <QTabWidget>
#include <kparts/partmanager.h>
#include <kedittoolbar.h>
#include <QDebug>
#include <QStandardPaths>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <KPluginLoader>
#include <KService>
#include <QMessageBox>
#include <QStatusBar>
#include <kconfigdialog.h>
#include <QDebug>
#include <krecentfilesaction.h>
#include <ktoolbar.h>
#include <KActionCollection>
#include <klocalizedstring.h>
#include <QtDBus/QtDBus>
#include <QDockWidget>
#include <QLoggingCategory>
#include <QTreeWidget>

#include <iostream>

using namespace KGraphViewer;

static QLoggingCategory debugCategory("org.kde.kgraphviewer");

KGraphEditor::KGraphEditor() :
    KParts::MainWindow(),
    m_rfa(nullptr),
    m_currentPart(nullptr)
{
  // set the shell's ui resource file
  setXMLFile("kgrapheditorui.rc");

  m_widget = new QTabWidget(this);
  m_widget->setTabsClosable(true);
  connect(m_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(close(int)));
  connect(m_widget, SIGNAL(currentChanged(int)), this, SLOT(newTabSelectedSlot(int)));

  setCentralWidget(m_widget);

  QDockWidget* topLeftDockWidget = new QDockWidget(this);
  topLeftDockWidget->setObjectName(QStringLiteral("TopLeftDockWidget"));
  m_treeWidget = new KGraphEditorNodesTreeWidget(topLeftDockWidget);
  connect(m_treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),
          this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
  connect(m_treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),
          this,SLOT(slotItemClicked(QTreeWidgetItem*,int)));
  connect(m_treeWidget, SIGNAL(removeNode(QString)),
          this, SLOT(slotRemoveNode(QString)));
  connect(m_treeWidget, SIGNAL(addAttribute(QString)),
          this, SLOT(slotAddAttribute(QString)));
  connect(m_treeWidget, SIGNAL(removeAttribute(QString,QString)),
          this, SLOT(slotRemoveAttribute(QString,QString)));

//   m_treeWidget->setItemDelegate(new VariantDelegate(m_treeWidget));
  m_treeWidget->setColumnCount(2);
  topLeftDockWidget->setWidget(m_treeWidget);
  addDockWidget ( Qt::LeftDockWidgetArea, topLeftDockWidget );

  QDockWidget* bottomLeftDockWidget = new QDockWidget(this);
  bottomLeftDockWidget->setObjectName(QStringLiteral("BottomLeftDockWidget"));
  m_newElementAttributesWidget = new KGraphEditorElementTreeWidget(bottomLeftDockWidget);
  connect(m_newElementAttributesWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),
          this,SLOT(slotNewElementItemChanged(QTreeWidgetItem*,int)));
  connect(m_newElementAttributesWidget, SIGNAL(addAttribute(QString)),
          this, SLOT(slotAddNewElementAttribute(QString)));
  connect(m_newElementAttributesWidget, SIGNAL(removeAttribute(QString)),
          this, SLOT(slotRemoveNewElementAttribute(QString)));
  m_newElementAttributesWidget->setColumnCount(2);
  bottomLeftDockWidget->setWidget(m_newElementAttributesWidget);
  addDockWidget ( Qt::LeftDockWidgetArea, bottomLeftDockWidget );


  if (QDBusConnection::sessionBus().registerService( "org.kde.kgrapheditor" ))
  {
    qCDebug(debugCategory) << "Service Registered successfully";
    QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);

  }
  else
  {
    qCDebug(debugCategory) << "Failed to register service...";
  }

  // Create a KParts part manager, to handle part activation/deactivation
  m_manager = new KParts::PartManager( this );

  // When the manager says the active part changes, the window updates (recreates) the GUI
  connect( m_manager, SIGNAL(activePartChanged(KParts::Part*)),
           this, SLOT(createGUI(KParts::Part*)) );

  setupGUI(ToolBar | Keys | StatusBar | Save);

  // then, setup our actions
  setupActions();

  // this routine will find and load our Part.  it finds the Part by
  // name which is a bad idea usually.. but it's alright in this
  // case since our Part is made for this Shell

  // Creates the GUI with a null part to make appear the main app menus and tools
  createGUI(0);
}

KGraphEditor::~KGraphEditor()
{
}

void KGraphEditor::reloadPreviousFiles()
{
  QStringList previouslyOpenedFiles = KGraphEditorSettings::previouslyOpenedFiles();
  if ( (previouslyOpenedFiles.empty() == false)
       && (QMessageBox::question(this,
              i18n("Session Restore"),
              i18n("Do you want to reload files from previous session?")) == QMessageBox::Yes) )
  {
    QStringList::const_iterator it, it_end;
    it = previouslyOpenedFiles.constBegin(); it_end = previouslyOpenedFiles.constEnd();
    for (; it != it_end; it++)
    {
      openUrl(*it);
    }
  }
}

KParts::ReadOnlyPart *KGraphEditor::slotNewGraph()
{
  KPluginFactory *factory = KPluginLoader("kgraphviewerpart").factory();
  if (!factory)
  {
    // if we couldn't find our Part, we exit since the Shell by
    // itself can't do anything useful
    QMessageBox::critical(this, i18n("Unable to start"), i18n("Could not find the KGraphViewer part."));
    qApp->quit();
    // we return here, cause kapp->quit() only means "exit the
    // next time we enter the event loop...
    return nullptr;
  }
  KParts::ReadOnlyPart* part = factory->create<KParts::ReadOnlyPart>(this);
  KGraphViewerInterface *view = qobject_cast<KGraphViewerInterface *>(part);
  if (!view)
  {
    // This should not happen
    qWarning() << "Failed to get KPart" << endl;
    return nullptr;
  }
  view->setReadWrite();

    QWidget *w = part->widget();
    m_widget->addTab(w, QIcon::fromTheme("kgraphviewer"), "");
    m_widget->setCurrentWidget(w);
    createGUI(part);
    m_closeAction->setEnabled(true);

    m_manager->addPart(part, true);
    m_tabsPartsMap[w] = part;
    m_tabsFilesMap[w] = "";
    connect(this,SIGNAL(hide(KParts::Part*)),part,SLOT(slotHide(KParts::Part*)));
  slotSetActiveGraph(part);
  return part;
}

void KGraphEditor::openUrl(const QUrl& url)
{
  qCDebug(debugCategory) << url;
  KParts::ReadOnlyPart *part = slotNewGraph();

//   (KGraphEditorSettings::parsingMode() == "external")
//     ?kgv->setLayoutMethod(KGraphViewerInterface::ExternalProgram)
//     :kgv->setLayoutMethod(KGraphViewerInterface::InternalLibrary);

  QString label = url.path().section('/',-1,-1);
  // @TODO set label
  m_widget->setTabText(m_widget->currentIndex(), label);
  m_tabsFilesMap[m_widget->currentWidget()] = url.path();
  part->openUrl(url);

  if (m_rfa) {
    m_rfa->addUrl(url);
  }

  m_openedFiles.push_back(url.path());
}

void KGraphEditor::fileOpen()
{
  // this slot is called whenever the File->Open menu is selected,
  // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
  // button is clicked
  QFileDialog fileDialog(this);
  fileDialog.setMimeTypeFilters(QStringList(QStringLiteral("text/vnd.graphviz")));
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setFileMode(QFileDialog::ExistingFiles);
  if (fileDialog.exec() != QFileDialog::Accepted) {
    return;
  }

  foreach (const QUrl& url, fileDialog.selectedUrls()) {
    openUrl(url);
  }
}


void KGraphEditor::setupActions()
{
  // create our actions

  actionCollection()->addAction( KStandardAction::New, "file_new", this, SLOT(fileNew()) );
  actionCollection()->addAction( KStandardAction::Open, "file_open", this, SLOT(fileOpen()) );
  m_rfa = (KRecentFilesAction*) actionCollection()->addAction(KStandardAction::OpenRecent, "file_open_recent", this, SLOT(slotURLSelected(QUrl)) );
  m_rfa->loadEntries(KConfigGroup(KSharedConfig::openConfig(), "kgrapheditor"));
  actionCollection()->addAction( KStandardAction::Save, "file_save", this, SLOT(fileSave()) );
  actionCollection()->addAction( KStandardAction::SaveAs, "file_save_as", this, SLOT(fileSaveAs()) );

  m_closeAction = actionCollection()->addAction( KStandardAction::Close, "file_close", this, SLOT(close()) );
  m_closeAction->setWhatsThis(i18n("Closes the current file"));
  m_closeAction->setEnabled(false);

  actionCollection()->addAction( KStandardAction::Quit, "file_quit", qApp, SLOT(quit()) );

  m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), this);

  actionCollection()->addAction( KStandardAction::KeyBindings, "options_configure_keybinding", this, SLOT(optionsConfigureKeys()) );
//   KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), this);
  actionCollection()->addAction( KStandardAction::ConfigureToolbars, "options_configure_toolbars", this, SLOT(optionsConfigureToolbars()) );
  actionCollection()->addAction( KStandardAction::Preferences, "options_configure", this, SLOT(optionsConfigure()) );

  QAction* edit_new_vertex = actionCollection()->addAction( "edit_new_vertex" );
  edit_new_vertex->setText(i18n("Create a New Vertex"));
  edit_new_vertex->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kgraphviewerpart/pics/kgraphviewer-newnode.png")));
  connect( edit_new_vertex, SIGNAL(triggered(bool)), this, SLOT(slotEditNewVertex()) );

  QAction* edit_new_edge = actionCollection()->addAction( "edit_new_edge" );
  edit_new_edge->setText(i18n("Create a New Edge"));
  edit_new_edge->setIcon(QPixmap(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kgraphviewerpart/pics/kgraphviewer-newedge.png")));
  connect( edit_new_edge, SIGNAL(triggered(bool)), this, SLOT(slotEditNewEdge()) );
}

void KGraphEditor::closeEvent(QCloseEvent *event)
{
  KGraphEditorSettings::setPreviouslyOpenedFiles(m_openedFiles);
  m_rfa->saveEntries(KConfigGroup(KSharedConfig::openConfig(), "kgrapheditor"));

  KGraphEditorSettings::self()->save();
  KXmlGuiWindow::closeEvent(event);
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
    KConfigGroup conf(KConfigGroup(KSharedConfig::openConfig(), "kgrapheditor"));
    saveMainWindowSettings(conf);

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
  KgeConfigurationDialog* dialog = new KgeConfigurationDialog(this, "settings", KGraphEditorSettings::self());

  Ui::KGraphViewerPreferencesParsingWidget*  parsingWidget = dialog->m_parsingWidget;
  qCDebug(debugCategory) << KGraphEditorSettings::parsingMode();
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
  applyMainWindowSettings(KConfigGroup(KSharedConfig::openConfig(), "kgrapheditor"));
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
//   qWarning() << "Invalid reload on change mode value: " << value;
//     return;
//   }
//   kDebug() << "emiting";
//   emit(settingsChanged());
//   KGraphEditorSettings::save();
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
//   qWarning() << "Invalid OpenInExistingWindow value: " << value << endl;
//     return;
//   }
//
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphEditorSettings::save();
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
//   qWarning() << "Invalid ReopenPreviouslyOpenedFilesMode value: " << value << endl;
//     return;
//   }
//
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphEditorSettings::save();
// }


void KGraphEditor::slotURLSelected(const QUrl& url)
{
  openUrl(url);
}

void KGraphEditor::close(int index)
{
  QWidget *tab = m_widget->widget(index);
  m_openedFiles.removeAll(m_tabsFilesMap[tab]);
  m_widget->removeTab(index);
  tab->hide();
  KParts::ReadOnlyPart *part = m_tabsPartsMap[tab];
  m_manager->removePart(part);
  m_tabsPartsMap.remove(tab);
  m_tabsFilesMap.remove(tab);
  delete part; part = nullptr;
/*  delete tab;
  tab = nullptr;*/
  m_closeAction->setEnabled(m_widget->count() > 0);
}

void KGraphEditor::close()
{
  int currentPage = m_widget->currentIndex();
  if (currentPage != -1)
  {
    close(currentPage);
  }
}

void KGraphEditor::fileSave()
{
  QWidget* currentPage = m_widget->currentWidget();
  if (currentPage)
  {
    emit(saveTo(QUrl(m_tabsFilesMap[currentPage]).path()));
  }
}

void KGraphEditor::fileSaveAs()
{
  QWidget* currentPage = m_widget->currentWidget();
  if (currentPage)
  {
    QFileDialog fileDialog(currentPage, i18n("Save current graph"));
    fileDialog.setMimeTypeFilters(QStringList(QStringLiteral("text/vnd.graphviz")));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (fileDialog.exec() != QFileDialog::Accepted) {
      return;
    }
    const QString fileName = fileDialog.selectedFiles().at(0);
    if (fileName.isEmpty()) {
      return;
    }

    m_tabsFilesMap[currentPage] = fileName;
    emit(saveTo(fileName));
  }
}

void KGraphEditor::newTabSelectedSlot(int index)
{
//   qCDebug(debugCategory) << tab;
  emit(hide((KParts::Part*)(m_manager->activePart())));
  QWidget *tab = m_widget->widget(index);
  if (tab) {
    slotSetActiveGraph(m_tabsPartsMap[tab]);
    m_manager->setActivePart(m_tabsPartsMap[tab]);
  }
}

void KGraphEditor::slotSetActiveGraph(KParts::ReadOnlyPart *part)
{
  if (m_currentPart)
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
  if (m_currentPart == nullptr)
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
    qCDebug(debugCategory)<< "new item " << nodeId;
    QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)nullptr, QStringList(nodeId));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    QMap<QString,QString> attributes;//TODO = m_currentPart->nodeAtributes(nodeId);
    foreach (const QString& attrib, attributes.keys())
    {
      if (attrib != "_draw_" && attrib != "_ldraw_")
      {
        QStringList list(attrib);
        list << attributes[attrib];
        QTreeWidgetItem* child = new QTreeWidgetItem((QTreeWidget*)nullptr, list);
        child->setFlags(child->flags() | Qt::ItemIsEditable);
        item->addChild(child);
      }
    }
    items.append(item);
  }
  qCDebug(debugCategory) << "inserting";
  m_treeWidget->insertTopLevelItems(0, items);


  connect( m_currentPart, SIGNAL(graphLoaded()),
           this, SLOT(slotGraphLoaded()) );

  connect( m_currentPart, SIGNAL(newNodeAdded(QString)),
          this, SLOT(slotNewNodeAdded(QString)) );

  connect( m_currentPart, SIGNAL(newEdgeAdded(QString,QString)),
            this, SLOT(slotNewEdgeAdded(QString,QString)) );

  connect( m_currentPart, SIGNAL(removeElement(QString)),
            this, SLOT(slotRemoveElement(QString)) );

  connect( m_currentPart, SIGNAL(selectionIs(QList<QString>,QPoint)),
            this, SLOT(slotSelectionIs(QList<QString>,QPoint)) );

  connect( m_currentPart, SIGNAL(newEdgeFinished(QString,QString,QMap<QString, QString>)),
            this, SLOT(slotNewEdgeFinished(QString,QString,QMap<QString, QString>)) );

  connect( m_currentPart, SIGNAL(hoverEnter(QString)),
           this, SLOT(slotHoverEnter(QString)) );

  connect( m_currentPart, SIGNAL(hoverLeave(QString)),
          this, SLOT(slotHoverLeave(QString)) );
}

void KGraphEditor::slotNewNodeAdded(const QString& id)
{
  qCDebug(debugCategory) << id;
  update();
}

void KGraphEditor::slotNewEdgeAdded(const QString& ids, const QString& idt)
{
  qCDebug(debugCategory) << ids << idt;
  update();
}

void KGraphEditor::slotNewEdgeFinished( const QString& srcId, const QString& tgtId, const QMap<QString, QString>&attribs)
{
  qCDebug(debugCategory) << srcId << tgtId << attribs;
  emit saddNewEdge(srcId,tgtId,attribs);
  update();
}

void KGraphEditor::slotGraphLoaded()
{
  qCDebug(debugCategory);
  disconnect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
  disconnect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           this,SLOT(slotItemClicked(QTreeWidgetItem*,int)));

  QList<QString> nodesIds;//TODO = m_currentPart->nodesIds();
  QList<QTreeWidgetItem *> items;
  foreach (const QString& nodeId, nodesIds)
  {
    qCDebug(debugCategory)<< "item " << nodeId;
    QTreeWidgetItem* item;
    QList<QTreeWidgetItem*> existingItems = m_treeWidget->findItems(nodeId,Qt::MatchRecursive|Qt::MatchExactly);
    if (existingItems.isEmpty())
    {
      item = new QTreeWidgetItem((QTreeWidget*)nullptr, QStringList(nodeId));
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
        QTreeWidgetItem* child = new QTreeWidgetItem((QTreeWidget*)nullptr, list);
        child->setFlags(child->flags() | Qt::ItemIsEditable);
        item->addChild(child);
      }
    }
}
  qCDebug(debugCategory) << "inserting";
  m_treeWidget->insertTopLevelItems(0, items);
  connect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
           this,SLOT(slotItemChanged(QTreeWidgetItem*,int)));
  connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           this,SLOT(slotItemClicked(QTreeWidgetItem*,int)));
}

void KGraphEditor::slotItemChanged ( QTreeWidgetItem * item, int column )
{
  qCDebug(debugCategory) ;
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
    if (item->parent())
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
  qCDebug(debugCategory) << column;
  m_currentTreeWidgetItemText = item->text(0);

  QString nodeName = item->parent() ?
                        item->parent()->text(0) :
                        item->text(0);
  emit selectNode(nodeName);
}

void KGraphEditor::slotEditNewVertex()
{
  if (m_currentPart == nullptr)
  {
    qCDebug(debugCategory) << "new vertex: no part selected";
    return;
  }
  qCDebug(debugCategory) << "new vertex";
  emit(prepareAddNewElement(m_newElementAttributes));
}

void KGraphEditor::slotEditNewEdge()
{
  if (m_currentPart == nullptr)
  {
    qCDebug(debugCategory) << "new edge: no part selected";
    return;
  }
  qCDebug(debugCategory) << "new edge";
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
  emit removeAttribute(nodeName,attribName);
  emit update();
}

void KGraphEditor::slotNewElementItemChanged(QTreeWidgetItem* item ,int column)
{
  qCDebug(debugCategory);
  if (column == 0)
  {
    qWarning() << "Item id change not handled";
    return;
  }
  else if (column == 1)
  {
    m_newElementAttributes[item->text(0)] = item->text(1);
  }
  else
  {
    qWarning() << "Unknown column" << column;
    return;
  }
}

void KGraphEditor::slotAddNewElementAttribute(const QString& attrib)
{
  m_newElementAttributes[attrib] = QString();
}

void KGraphEditor::slotRemoveNewElementAttribute(const QString& attrib)
{
  m_newElementAttributes.remove(attrib);
}

void KGraphEditor::slotRemoveElement(const QString& id)
{
  qCDebug(debugCategory) << id;
  m_treeWidget->slotRemoveElement(id);
  emit(removeElement(id));
}

void KGraphEditor::slotSelectionIs(const QList<QString>& elements , const QPoint& p)
{
  qCDebug(debugCategory);
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
  if (value)
  {
    KGraphEditorSettings::setParsingMode("external");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphEditorSettings::self()->save();
}

void KGraphEditor::slotParsingModeInternalToggled(bool value)
{
  if (value)
  {
    KGraphEditorSettings::setParsingMode("internal");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphEditorSettings::self()->save();
}

void KGraphEditor::slotHoverEnter(const QString& id)
{
  qCDebug(debugCategory) << id;
  statusBar()->showMessage(id);
}

void KGraphEditor::slotHoverLeave(const QString& id)
{
  qCDebug(debugCategory) << id;
  statusBar()->showMessage("");
}
