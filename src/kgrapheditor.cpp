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

// app
#include "KGraphEditorElementTreeWidget.h"
#include "KGraphEditorNodesTreeWidget.h"
#include "kgrapheditorConfigDialog.h"
#include "kgrapheditor_debug.h"
#include "kgrapheditorsettings.h"
// #include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesParsing.h"
// #include "ui_preferencesReload.h"
// #include "ui_preferencesReopenPreviouslyOpenedFiles.h"
// KGraphViewerInterface
#include <part/kgraphviewer_interface.h>
// KF
#include <KActionCollection>
#include <KStandardAction>
#include <KParts/PartManager>
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>
#include <KEditToolBar>
#include <KToolBar>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KLocalizedString>
// Qt
#include <QDBusConnection>
#include <QFileDialog>
#include <QStatusBar>
#include <QMessageBox>
#include <QDockWidget>

using namespace KGraphViewer;

KGraphEditor::KGraphEditor()
    : KParts::MainWindow()
    , m_rfa(nullptr)
    , m_currentPart(nullptr)
{
    // set the shell's ui resource file
    setXMLFile(QStringLiteral("kgrapheditorui.rc"));

    m_widget = new QTabWidget(this);
    m_widget->setTabsClosable(true);
    connect(m_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(close(int)));
    connect(m_widget, SIGNAL(currentChanged(int)), this, SLOT(newTabSelectedSlot(int)));

    setCentralWidget(m_widget);

    QDockWidget *topLeftDockWidget = new QDockWidget(this);
    topLeftDockWidget->setObjectName(QStringLiteral("TopLeftDockWidget"));
    m_treeWidget = new KGraphEditorNodesTreeWidget(topLeftDockWidget);
    connect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(slotItemChanged(QTreeWidgetItem *, int)));
    connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(slotItemClicked(QTreeWidgetItem *, int)));
    connect(m_treeWidget, SIGNAL(removeNode(QString)), this, SLOT(slotRemoveNode(QString)));
    connect(m_treeWidget, SIGNAL(addAttribute(QString)), this, SLOT(slotAddAttribute(QString)));
    connect(m_treeWidget, SIGNAL(removeAttribute(QString, QString)), this, SLOT(slotRemoveAttribute(QString, QString)));

    //   m_treeWidget->setItemDelegate(new VariantDelegate(m_treeWidget));
    m_treeWidget->setColumnCount(2);
    topLeftDockWidget->setWidget(m_treeWidget);
    addDockWidget(Qt::LeftDockWidgetArea, topLeftDockWidget);

    QDockWidget *bottomLeftDockWidget = new QDockWidget(this);
    bottomLeftDockWidget->setObjectName(QStringLiteral("BottomLeftDockWidget"));
    m_newElementAttributesWidget = new KGraphEditorElementTreeWidget(bottomLeftDockWidget);
    connect(m_newElementAttributesWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(slotNewElementItemChanged(QTreeWidgetItem *, int)));
    connect(m_newElementAttributesWidget, SIGNAL(addAttribute(QString)), this, SLOT(slotAddNewElementAttribute(QString)));
    connect(m_newElementAttributesWidget, SIGNAL(removeAttribute(QString)), this, SLOT(slotRemoveNewElementAttribute(QString)));
    m_newElementAttributesWidget->setColumnCount(2);
    bottomLeftDockWidget->setWidget(m_newElementAttributesWidget);
    addDockWidget(Qt::LeftDockWidgetArea, bottomLeftDockWidget);

    if (QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.kgrapheditor"))) {
        qCDebug(KGRAPHEDITOR_LOG) << "Service Registered successfully";
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllSlots);

    } else {
        qCDebug(KGRAPHEDITOR_LOG) << "Failed to register service...";
    }

    // Create a KParts part manager, to handle part activation/deactivation
    m_manager = new KParts::PartManager(this);

    // When the manager says the active part changes, the window updates (recreates) the GUI
    connect(m_manager, SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(createGUI(KParts::Part *)));

    setupGUI(ToolBar | Keys | StatusBar | Save);

    // then, setup our actions
    setupActions();

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell

    // Creates the GUI with a null part to make appear the main app menus and tools
    createGUI(nullptr);
}

KGraphEditor::~KGraphEditor()
{
}

void KGraphEditor::reloadPreviousFiles()
{
    QStringList previouslyOpenedFiles = KGraphEditorSettings::previouslyOpenedFiles();
    if ((previouslyOpenedFiles.empty() == false) && (QMessageBox::question(this, i18n("Session Restore"), i18n("Do you want to reload files from previous session?")) == QMessageBox::Yes)) {
        QStringList::const_iterator it, it_end;
        it = previouslyOpenedFiles.constBegin();
        it_end = previouslyOpenedFiles.constEnd();
        for (; it != it_end; it++) {
            openUrl(*it);
        }
    }
}

KParts::ReadOnlyPart *KGraphEditor::slotNewGraph()
{
    KPluginFactory *factory = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("kf6/parts/kgraphviewerpart"))).plugin;
    if (!factory) {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        QMessageBox::critical(this, i18n("Unable to start"), i18n("Could not find the KGraphViewer part."));
        qApp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return nullptr;
    }
    KParts::ReadOnlyPart *part = factory->create<KParts::ReadOnlyPart>(this);
    KGraphViewerInterface *view = qobject_cast<KGraphViewerInterface *>(part);
    if (!view) {
        // This should not happen
        qCWarning(KGRAPHEDITOR_LOG) << "Failed to get KPart" << Qt::endl;
        return nullptr;
    }
    view->setReadWrite();

    QWidget *w = part->widget();

    m_tabsPartsMap[w] = part;
    m_tabsFilesMap[w] = QString();
    connect(this, SIGNAL(hide(KParts::Part *)), part, SLOT(slotHide(KParts::Part *)));

    m_manager->addPart(part, true);

    m_widget->addTab(w, QIcon::fromTheme(QStringLiteral("kgraphviewer")), QString());
    m_widget->setCurrentWidget(w);
    m_closeAction->setEnabled(true);
    return part;
}

void KGraphEditor::openUrl(const QUrl &url)
{
    qCDebug(KGRAPHEDITOR_LOG) << url;
    KParts::ReadOnlyPart *part = slotNewGraph();

    //   (KGraphEditorSettings::parsingMode() == "external")
    //     ?kgv->setLayoutMethod(KGraphViewerInterface::ExternalProgram)
    //     :kgv->setLayoutMethod(KGraphViewerInterface::InternalLibrary);

    QString label = url.path().section(QLatin1Char('/'), -1, -1);
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

    for (const QUrl &url : fileDialog.selectedUrls()) {
        openUrl(url);
    }
}

void KGraphEditor::setupActions()
{
    // create our actions

    actionCollection()->addAction(KStandardAction::New, QStringLiteral("file_new"), this, SLOT(fileNew()));
    actionCollection()->addAction(KStandardAction::Open, QStringLiteral("file_open"), this, SLOT(fileOpen()));
    m_rfa = (KRecentFilesAction *)actionCollection()->addAction(KStandardAction::OpenRecent, QStringLiteral("file_open_recent"), this, SLOT(slotURLSelected(QUrl)));
    m_rfa->loadEntries(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("kgrapheditor")));
    actionCollection()->addAction(KStandardAction::Save, QStringLiteral("file_save"), this, SLOT(fileSave()));
    actionCollection()->addAction(KStandardAction::SaveAs, QStringLiteral("file_save_as"), this, SLOT(fileSaveAs()));

    m_closeAction = actionCollection()->addAction(KStandardAction::Close, QStringLiteral("file_close"), this, SLOT(close()));
    m_closeAction->setWhatsThis(i18n("Closes the current file"));
    m_closeAction->setEnabled(false);

    actionCollection()->addAction(KStandardAction::Quit, QStringLiteral("file_quit"), qApp, SLOT(quit()));

    m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), this);

    actionCollection()->addAction(KStandardAction::ConfigureToolbars, QStringLiteral("options_configure_toolbars"), this, SLOT(optionsConfigureToolbars()));
    actionCollection()->addAction(KStandardAction::Preferences, QStringLiteral("options_configure"), this, SLOT(optionsConfigure()));

    QAction *edit_new_vertex = actionCollection()->addAction(QStringLiteral("edit_new_vertex"));
    edit_new_vertex->setText(i18n("Create a New Vertex"));
    edit_new_vertex->setIcon(QPixmap(QStringLiteral(":/kgraphviewerpart/pics/newnode.png")));
    connect(edit_new_vertex, SIGNAL(triggered(bool)), this, SLOT(slotEditNewVertex()));

    QAction *edit_new_edge = actionCollection()->addAction(QStringLiteral("edit_new_edge"));
    edit_new_edge->setText(i18n("Create a New Edge"));
    edit_new_edge->setIcon(QPixmap(QStringLiteral(":/kgraphviewerpart/pics/newedge.png")));
    connect(edit_new_edge, SIGNAL(triggered(bool)), this, SLOT(slotEditNewEdge()));
}

void KGraphEditor::closeEvent(QCloseEvent *event)
{
    KGraphEditorSettings::setPreviouslyOpenedFiles(m_openedFiles);
    m_rfa->saveEntries(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("kgrapheditor")));

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

void KGraphEditor::optionsConfigureToolbars()
{
    KConfigGroup conf(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("kgrapheditor")));
    saveMainWindowSettings(conf);

    // use the standard toolbar editor
    KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void KGraphEditor::optionsConfigure()
{
    // An instance of your dialog could be already created and could be cached,
    // in which case you want to display the cached dialog instead of creating
    // another one
    if (KgeConfigurationDialog::showDialog(QStringLiteral("settings")))
        return;

    // KConfigDialog didn't find an instance of this dialog, so lets create it :
    KgeConfigurationDialog *dialog = new KgeConfigurationDialog(this, QStringLiteral("settings"), KGraphEditorSettings::self());

    Ui::KGraphViewerPreferencesParsingWidget *parsingWidget = dialog->m_parsingWidget;
    qCDebug(KGRAPHEDITOR_LOG) << KGraphEditorSettings::parsingMode();
    if (KGraphEditorSettings::parsingMode() == QStringLiteral("external")) {
        parsingWidget->external->setChecked(true);
    } else if (KGraphEditorSettings::parsingMode() == QStringLiteral("internal")) {
        parsingWidget->internal->setChecked(true);
    }
    connect((QObject *)parsingWidget->external, SIGNAL(toggled(bool)), this, SLOT(slotParsingModeExternalToggled(bool)));
    connect((QObject *)parsingWidget->internal, SIGNAL(toggled(bool)), this, SLOT(slotParsingModeInternalToggled(bool)));

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
    applyMainWindowSettings(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("kgrapheditor")));
}

// void KGraphViewer::reloadOnChangeMode_pressed(int value)
// {
//   qCDebug(KGRAPHEDITOR_LOG) << "reloadOnChangeMode_pressed " << value;
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
//   qCWarning(KGRAPHEDITOR_LOG) << "Invalid reload on change mode value: " << value;
//     return;
//   }
//   qCDebug(KGRAPHEDITOR_LOG) << "emiting";
//   Q_EMIT settingsChanged();
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
//   qCWarning(KGRAPHEDITOR_LOG) << "Invalid OpenInExistingWindow value: " << value << endl;
//     return;
//   }
//
//   std::cerr << "emiting" << std::endl;
//   Q_EMIT settingsChanged();
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
//   qCWarning(KGRAPHEDITOR_LOG) << "Invalid ReopenPreviouslyOpenedFilesMode value: " << value << endl;
//     return;
//   }
//
//   std::cerr << "emiting" << std::endl;
//   Q_EMIT settingsChanged();
//   KGraphEditorSettings::save();
// }

void KGraphEditor::slotURLSelected(const QUrl &url)
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
    delete part;
    part = nullptr;
    /*  delete tab;
      tab = nullptr;*/
    m_closeAction->setEnabled(m_widget->count() > 0);
}

void KGraphEditor::close()
{
    int currentPage = m_widget->currentIndex();
    if (currentPage != -1) {
        close(currentPage);
    }
}

void KGraphEditor::fileSave()
{
    QWidget *currentPage = m_widget->currentWidget();
    if (currentPage) {
        Q_EMIT saveTo(QUrl(m_tabsFilesMap[currentPage]).path());
    }
}

void KGraphEditor::fileSaveAs()
{
    QWidget *currentPage = m_widget->currentWidget();
    if (currentPage) {
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
        Q_EMIT saveTo(fileName);
    }
}

void KGraphEditor::newTabSelectedSlot(int index)
{
    //   qCDebug(KGRAPHEDITOR_LOG) << tab;
    Q_EMIT hide((KParts::Part *)(m_manager->activePart()));
    QWidget *tab = m_widget->widget(index);
    if (tab) {
        slotSetActiveGraph(m_tabsPartsMap[tab]);
        m_manager->setActivePart(m_tabsPartsMap[tab]);
    }
}

void KGraphEditor::slotSetActiveGraph(KParts::ReadOnlyPart *part)
{
    if (m_currentPart) {
        disconnect(this, SIGNAL(prepareAddNewElement(QMap<QString, QString>)), m_currentPart, SLOT(prepareAddNewElement(QMap<QString, QString>)));
        disconnect(this, SIGNAL(prepareAddNewEdge(QMap<QString, QString>)), m_currentPart, SLOT(prepareAddNewEdge(QMap<QString, QString>)));
        disconnect(this, SIGNAL(saveTo(QString)), m_currentPart, SLOT(saveTo(QString)));
        disconnect(this, SIGNAL(removeNode(QString)), m_currentPart, SLOT(slotRemoveNode(QString)));
        disconnect(this, SIGNAL(addAttribute(QString)), m_currentPart, SLOT(slotAddAttribute(QString)));
        disconnect(this, SIGNAL(removeAttribute(QString, QString)), m_currentPart, SLOT(slotRemoveAttribute(QString, QString)));
        disconnect(this, SIGNAL(update()), m_currentPart, SLOT(slotUpdate()));
        disconnect(this, SIGNAL(selectNode(QString)), m_currentPart, SLOT(slotSelectNode(QString)));
        disconnect(this, SIGNAL(saddNewEdge(QString, QString, QMap<QString, QString>)), m_currentPart, SLOT(slotAddNewEdge(QString, QString, QMap<QString, QString>)));
        disconnect(this, SIGNAL(renameNode(QString, QString)), m_currentPart, SLOT(slotRenameNode(QString, QString)));
        disconnect(this, SIGNAL(setAttribute(QString, QString, QString)), m_currentPart, SLOT(slotSetAttribute(QString, QString, QString)));
    }
    m_currentPart = part;
    m_treeWidget->clear();
    if (m_currentPart == nullptr) {
        return;
    }
    connect(this, SIGNAL(prepareAddNewElement(QMap<QString, QString>)), part, SLOT(prepareAddNewElement(QMap<QString, QString>)));
    connect(this, SIGNAL(prepareAddNewEdge(QMap<QString, QString>)), part, SLOT(prepareAddNewEdge(QMap<QString, QString>)));
    connect(this, SIGNAL(saveTo(QString)), part, SLOT(saveTo(QString)));
    connect(this, SIGNAL(removeNode(QString)), part, SLOT(slotRemoveNode(QString)));
    connect(this, SIGNAL(addAttribute(QString)), part, SLOT(slotAddAttribute(QString)));
    connect(this, SIGNAL(removeAttribute(QString, QString)), part, SLOT(slotRemoveAttribute(QString, QString)));
    connect(this, SIGNAL(update()), part, SLOT(slotUpdate()));
    connect(this, SIGNAL(selectNode(QString)), part, SLOT(slotSelectNode(QString)));
    connect(this, SIGNAL(removeElement(QString)), m_currentPart, SLOT(slotRemoveElement(QString)));
    connect(this, SIGNAL(saddNewEdge(QString, QString, QMap<QString, QString>)), m_currentPart, SLOT(slotAddNewEdge(QString, QString, QMap<QString, QString>)));
    connect(this, SIGNAL(renameNode(QString, QString)), m_currentPart, SLOT(slotRenameNode(QString, QString)));
    connect(this, SIGNAL(setAttribute(QString, QString, QString)), m_currentPart, SLOT(slotSetAttribute(QString, QString, QString)));

    QList<QString> nodesIds; // TODO = m_currentPart->nodesIds();
    QList<QTreeWidgetItem *> items;
    for (const QString &nodeId : nodesIds) {
        qCDebug(KGRAPHEDITOR_LOG) << "new item " << nodeId;
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(nodeId));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        QMap<QString, QString> attributes; // TODO = m_currentPart->nodeAtributes(nodeId);
        for (const QString &attrib : attributes.keys()) {
            if (attrib != QLatin1String("_draw_") && attrib != QLatin1String("_ldraw_")) {
                QStringList list(attrib);
                list << attributes[attrib];
                QTreeWidgetItem *child = new QTreeWidgetItem((QTreeWidget *)nullptr, list);
                child->setFlags(child->flags() | Qt::ItemIsEditable);
                item->addChild(child);
            }
        }
        items.append(item);
    }
    qCDebug(KGRAPHEDITOR_LOG) << "inserting";
    m_treeWidget->insertTopLevelItems(0, items);

    connect(m_currentPart, SIGNAL(graphLoaded()), this, SLOT(slotGraphLoaded()));

    connect(m_currentPart, SIGNAL(newNodeAdded(QString)), this, SLOT(slotNewNodeAdded(QString)));

    connect(m_currentPart, SIGNAL(newEdgeAdded(QString, QString)), this, SLOT(slotNewEdgeAdded(QString, QString)));

    connect(m_currentPart, SIGNAL(removeElement(QString)), this, SLOT(slotRemoveElement(QString)));

    connect(m_currentPart, SIGNAL(selectionIs(QList<QString>, QPoint)), this, SLOT(slotSelectionIs(QList<QString>, QPoint)));

    connect(m_currentPart, SIGNAL(newEdgeFinished(QString, QString, QMap<QString, QString>)), this, SLOT(slotNewEdgeFinished(QString, QString, QMap<QString, QString>)));

    connect(m_currentPart, SIGNAL(hoverEnter(QString)), this, SLOT(slotHoverEnter(QString)));

    connect(m_currentPart, SIGNAL(hoverLeave(QString)), this, SLOT(slotHoverLeave(QString)));
}

void KGraphEditor::slotNewNodeAdded(const QString &id)
{
    qCDebug(KGRAPHEDITOR_LOG) << id;
    update();
}

void KGraphEditor::slotNewEdgeAdded(const QString &ids, const QString &idt)
{
    qCDebug(KGRAPHEDITOR_LOG) << ids << idt;
    update();
}

void KGraphEditor::slotNewEdgeFinished(const QString &srcId, const QString &tgtId, const QMap<QString, QString> &attribs)
{
    qCDebug(KGRAPHEDITOR_LOG) << srcId << tgtId << attribs;
    Q_EMIT saddNewEdge(srcId, tgtId, attribs);
    update();
}

void KGraphEditor::slotGraphLoaded()
{
    qCDebug(KGRAPHEDITOR_LOG);
    disconnect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(slotItemChanged(QTreeWidgetItem *, int)));
    disconnect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(slotItemClicked(QTreeWidgetItem *, int)));

    QList<QString> nodesIds; // TODO = m_currentPart->nodesIds();
    QList<QTreeWidgetItem *> items;
    for (const QString &nodeId : nodesIds) {
        qCDebug(KGRAPHEDITOR_LOG) << "item " << nodeId;
        QTreeWidgetItem *item;
        QList<QTreeWidgetItem *> existingItems = m_treeWidget->findItems(nodeId, Qt::MatchRecursive | Qt::MatchExactly);
        if (existingItems.isEmpty()) {
            item = new QTreeWidgetItem((QTreeWidget *)nullptr, QStringList(nodeId));
            items.append(item);
        } else {
            item = existingItems[0];
        }
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        QMap<QString, QString> attributes; // TODO = m_currentPart->nodeAtributes(nodeId);
        QList<QString> keys = attributes.keys();
        for (int i = 0; i < item->childCount(); i++) {
            if (keys.contains(item->child(i)->text(0))) {
                item->child(i)->setText(1, attributes[item->child(i)->text(0)]);
                keys.removeAll(item->child(i)->text(0));
            }
        }
        for (const QString &attrib : keys) {
            if (attrib != QLatin1String("_draw_") && attrib != QLatin1String("_ldraw_")) {
                QStringList list(attrib);
                list << attributes[attrib];
                QTreeWidgetItem *child = new QTreeWidgetItem((QTreeWidget *)nullptr, list);
                child->setFlags(child->flags() | Qt::ItemIsEditable);
                item->addChild(child);
            }
        }
    }
    qCDebug(KGRAPHEDITOR_LOG) << "inserting";
    m_treeWidget->insertTopLevelItems(0, items);
    connect(m_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(slotItemChanged(QTreeWidgetItem *, int)));
    connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(slotItemClicked(QTreeWidgetItem *, int)));
}

void KGraphEditor::slotItemChanged(QTreeWidgetItem *item, int column)
{
    qCDebug(KGRAPHEDITOR_LOG);
    /* values column */
    if (column == 0) {
        QString oldNodeName = m_currentTreeWidgetItemText;
        QString newNodeName = item->text(0);
        Q_EMIT renameNode(oldNodeName, newNodeName);
    } else if (column == 1) {
        /* there is a parent ; it is an attribute line */
        if (item->parent()) {
            QString nodeLabel = item->parent()->text(0);
            QString attributeName = item->text(0);
            QString attributeValue = item->text(1);
            Q_EMIT setAttribute(nodeLabel, attributeName, attributeValue);
        }
    }
    Q_EMIT update();
}

void KGraphEditor::slotItemClicked(QTreeWidgetItem *item, int column)
{
    qCDebug(KGRAPHEDITOR_LOG) << column;
    m_currentTreeWidgetItemText = item->text(0);

    QString nodeName = item->parent() ? item->parent()->text(0) : item->text(0);
    Q_EMIT selectNode(nodeName);
}

void KGraphEditor::slotEditNewVertex()
{
    if (m_currentPart == nullptr) {
        qCDebug(KGRAPHEDITOR_LOG) << "new vertex: no part selected";
        return;
    }
    qCDebug(KGRAPHEDITOR_LOG) << "new vertex";
    Q_EMIT prepareAddNewElement(m_newElementAttributes);
}

void KGraphEditor::slotEditNewEdge()
{
    if (m_currentPart == nullptr) {
        qCDebug(KGRAPHEDITOR_LOG) << "new edge: no part selected";
        return;
    }
    qCDebug(KGRAPHEDITOR_LOG) << "new edge";
    Q_EMIT prepareAddNewEdge(m_newElementAttributes);
}

void KGraphEditor::slotRemoveNode(const QString &nodeName)
{
    Q_EMIT removeNode(nodeName);
    Q_EMIT update();
}

void KGraphEditor::slotAddAttribute(const QString &attribName)
{
    Q_EMIT addAttribute(attribName);
    Q_EMIT update();
}

void KGraphEditor::slotRemoveAttribute(const QString &nodeName, const QString &attribName)
{
    Q_EMIT removeAttribute(nodeName, attribName);
    Q_EMIT update();
}

void KGraphEditor::slotNewElementItemChanged(QTreeWidgetItem *item, int column)
{
    qCDebug(KGRAPHEDITOR_LOG);
    if (column == 0) {
        qCWarning(KGRAPHEDITOR_LOG) << "Item id change not handled";
        return;
    } else if (column == 1) {
        m_newElementAttributes[item->text(0)] = item->text(1);
    } else {
        qCWarning(KGRAPHEDITOR_LOG) << "Unknown column" << column;
        return;
    }
}

void KGraphEditor::slotAddNewElementAttribute(const QString &attrib)
{
    m_newElementAttributes[attrib] = QString();
}

void KGraphEditor::slotRemoveNewElementAttribute(const QString &attrib)
{
    m_newElementAttributes.remove(attrib);
}

void KGraphEditor::slotRemoveElement(const QString &id)
{
    qCDebug(KGRAPHEDITOR_LOG) << id;
    m_treeWidget->slotRemoveElement(id);
    Q_EMIT removeElement(id);
}

void KGraphEditor::slotSelectionIs(const QList<QString> &elements, const QPoint &p)
{
    qCDebug(KGRAPHEDITOR_LOG);
    Q_UNUSED(p);
    QList<QTreeWidgetItem *> items = m_treeWidget->selectedItems();
    for (QTreeWidgetItem *item : items) {
        item->setSelected(false);
    }
    for (const QString &elementName : elements) {
        QList<QTreeWidgetItem *> items = m_treeWidget->findItems(elementName, Qt::MatchExactly, 0);
        for (QTreeWidgetItem *item : items) {
            item->setSelected(true);
        }
    }
}

void KGraphEditor::slotParsingModeExternalToggled(bool value)
{
    if (value) {
        KGraphEditorSettings::setParsingMode(QStringLiteral("external"));
    }
    //   qCDebug(KGRAPHEDITOR_LOG) << "emiting";
    //   Q_EMIT settingsChanged();
    KGraphEditorSettings::self()->save();
}

void KGraphEditor::slotParsingModeInternalToggled(bool value)
{
    if (value) {
        KGraphEditorSettings::setParsingMode(QStringLiteral("internal"));
    }
    //   qCDebug(KGRAPHEDITOR_LOG) << "emiting";
    //   Q_EMIT settingsChanged();
    KGraphEditorSettings::self()->save();
}

void KGraphEditor::slotHoverEnter(const QString &id)
{
    qCDebug(KGRAPHEDITOR_LOG) << id;
    statusBar()->showMessage(id);
}

void KGraphEditor::slotHoverLeave(const QString &id)
{
    qCDebug(KGRAPHEDITOR_LOG) << id;
    statusBar()->showMessage(QString());
}

#include "moc_kgrapheditor.cpp"
