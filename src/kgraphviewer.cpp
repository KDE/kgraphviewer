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

#include "kgraphviewer.h"
#include "kgraphviewerConfigDialog.h"
#include "kgraphviewer_debug.h"
#include "kgraphviewersettings.h"
#include "part/kgraphviewer_interface.h"
#include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesParsing.h"
#include "ui_preferencesReload.h"
#include "ui_preferencesReopenPreviouslyOpenedFiles.h"

#include <kwidgetsaddons_version.h>
#include <KActionCollection>
#include <KColorScheme>
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>
#include <KService>
#include <QDBusConnection>
#include <QDebug>
#include <QFileDialog>
#include <QIcon>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabWidget>
#include <QUrl>
#include <iostream>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kedittoolbar.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <krecentfilesaction.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <stdio.h>
#include <stdlib.h>

KGraphViewerWindow::KGraphViewerWindow()
    : KParts::MainWindow()
    , m_rfa(nullptr)
{
    // set the shell's ui resource file
    setXMLFile("kgraphviewerui.rc");

    //   std::cerr << "Creating tab widget" << std::endl;
    m_widget = new QTabWidget(this);
    m_widget->setTabsClosable(true);
    connect(m_widget, &QTabWidget::tabCloseRequested, this, static_cast<void (KGraphViewerWindow::*)(int)>(&KGraphViewerWindow::close));
    connect(m_widget, &QTabWidget::currentChanged, this, &KGraphViewerWindow::newTabSelectedSlot);

    setCentralWidget(m_widget);

    if (QDBusConnection::sessionBus().registerService("org.kde.kgraphviewer")) {
        qCDebug(KGRAPHVIEWER_LOG) << "Service Registered successfully";
        QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);

    } else {
        qCDebug(KGRAPHVIEWER_LOG) << "Failed to register service...";
    }

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell

    // Create a KParts part manager, to handle part activation/deactivation
    m_manager = new KParts::PartManager(this);

    // When the manager says the active part changes, the window updates (recreates) the GUI
    connect(m_manager, &KParts::PartManager::activePartChanged, this, &KGraphViewerWindow::createGUI);

    setupGUI(ToolBar | Keys | StatusBar | Save);

    // then, setup our actions
    setupActions();

    // Creates the GUI with a null part to make appear the main app menus and tools
    createGUI(nullptr);
}

KGraphViewerWindow::~KGraphViewerWindow()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    if (m_rfa)
        m_rfa->saveEntries(KConfigGroup(config, "kgraphviewer recent files"));

    // delete partsmanager explicitly, to avoid activePartChanged being emitted from here
    delete m_manager;
}

void KGraphViewerWindow::reloadPreviousFiles()
{
    QStringList previouslyOpenedFiles = KGraphViewerSettings::previouslyOpenedFiles();
    if ((previouslyOpenedFiles.empty() == false) &&
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        (KMessageBox::questionTwoActions(this,
#else
        (KMessageBox::questionYesNo(this,
#endif
                                    i18n("Do you want to reload files from the previous session?"),
                                    i18n("Session Restore"),
                                    KGuiItem(i18nc("@action:button", "Reload"), QStringLiteral("document-open")),
                                    KGuiItem(i18nc("@action:button", "Do Not Reload"), QStringLiteral("dialog-cancel")),
                                    "reopenPreviouslyOpenedFilesMode")
#if KWIDGETSADDONS_VERSION >= QT_VERSION_CHECK(5, 100, 0)
        == KMessageBox::PrimaryAction)) {
#else
        == KMessageBox::Yes)) {
#endif
        QStringList::const_iterator it, it_end;
        it = previouslyOpenedFiles.constBegin();
        it_end = previouslyOpenedFiles.constEnd();
        for (; it != it_end; it++) {
            openUrl(*it);
        }
        KGraphViewerSettings::self()->save();
    }
}

void KGraphViewerWindow::openUrl(const QUrl &url)
{
    qCDebug(KGRAPHVIEWER_LOG) << url;
    KPluginFactory *factory = KPluginFactory::loadFactory(KPluginMetaData("kgraphviewerpart")).plugin;
    if (!factory) {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not find the KGraphViewer part."));
        qApp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }
    KParts::ReadOnlyPart *part = factory->create<KParts::ReadOnlyPart>(this);
    KGraphViewer::KGraphViewerInterface *kgv = qobject_cast<KGraphViewer::KGraphViewerInterface *>(part);
    if (!kgv) {
        // This should not happen
        qCWarning(KGRAPHVIEWER_LOG) << "Failed to get KPart";
        return;
    }
    kgv->setBackgroundColor(KGraphViewerSettings::backgroundColor());
    (KGraphViewerSettings::parsingMode() == "external") ? kgv->setLayoutMethod(KGraphViewer::KGraphViewerInterface::ExternalProgram) : kgv->setLayoutMethod(KGraphViewer::KGraphViewerInterface::InternalLibrary);

    if (part) {
        QString fileName = url.url();
        QWidget *w = part->widget();

        part->openUrl(url);

        if (m_rfa) {
            m_rfa->addUrl(url);
            KSharedConfig::Ptr config = KSharedConfig::openConfig();
            m_rfa->saveEntries(KConfigGroup(config, "kgraphviewer recent files"));
        }

        m_openedFiles.push_back(fileName);
        m_tabsPartsMap[w] = part;
        m_tabsFilesMap[w] = fileName;
        connect(this, SIGNAL(hide(KParts::Part *)), part, SLOT(slotHide(KParts::Part *)));

        connect(part, SIGNAL(hoverEnter(QString)), this, SLOT(slotHoverEnter(QString)));
        connect(part, SIGNAL(hoverLeave(QString)), this, SLOT(slotHoverLeave(QString)));

        m_manager->addPart(part, true);
        const QString label = fileName.section('/', -1, -1);
        m_widget->addTab(w, QIcon::fromTheme("kgraphviewer"), label);
        m_widget->setCurrentWidget(w);
        m_closeAction->setEnabled(true);
    }
}

void KGraphViewerWindow::fileOpen()
{
    qCDebug(KGRAPHVIEWER_LOG);
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

    foreach (const QUrl &url, fileDialog.selectedUrls()) {
        openUrl(url);
    }
}

void KGraphViewerWindow::setupActions()
{
    // create our actions
    QAction *newAction = actionCollection()->addAction(KStandardAction::New, "file_new", this, SLOT(fileNew()));
    newAction->setWhatsThis(i18n("Opens a new empty KGraphViewer window."));

    QAction *openAction = actionCollection()->addAction(KStandardAction::Open, "file_open", this, SLOT(fileOpen()));
    openAction->setWhatsThis(i18n("Shows the file open dialog to choose a Graphviz DOT file to open."));

    m_rfa = KStandardAction::openRecent(this, SLOT(slotURLSelected(QUrl)), this);
    actionCollection()->addAction(m_rfa->objectName(), m_rfa);
    m_rfa->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    m_rfa->loadEntries(KConfigGroup(config, "kgraphviewer recent files"));

    m_closeAction = actionCollection()->addAction(KStandardAction::Close, "file_close", this, SLOT(close()));
    m_closeAction->setWhatsThis(i18n("Closes the current file"));
    m_closeAction->setEnabled(false);

    QAction *quitAction = actionCollection()->addAction(KStandardAction::Quit, "file_quit", qApp, SLOT(quit()));
    quitAction->setWhatsThis(i18n("Quits KGraphViewer."));

    m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), this);
    m_statusbarAction->setWhatsThis(i18n("Shows or hides the status bar."));

    QAction *ctAction = actionCollection()->addAction(KStandardAction::ConfigureToolbars, "options_configure_toolbars", this, SLOT(optionsConfigureToolbars()));
    ctAction->setWhatsThis(i18n("Toolbar configuration."));

    QAction *configureAction = actionCollection()->addAction(KStandardAction::Preferences, "options_configure", this, SLOT(optionsConfigure()));
    configureAction->setWhatsThis(i18n("Main KGraphViewer configuration options."));
}

void KGraphViewerWindow::closeEvent(QCloseEvent *event)
{
    KGraphViewerSettings::setPreviouslyOpenedFiles(m_openedFiles);
    KGraphViewerSettings::self()->save();
    KParts::MainWindow::closeEvent(event);
}

void KGraphViewerWindow::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    (new KGraphViewerWindow())->show();
}

void KGraphViewerWindow::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void KGraphViewerWindow::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void KGraphViewerWindow::optionsConfigureToolbars()
{
    KConfigGroup group(KConfigGroup(KSharedConfig::openConfig(), "kgraphviewer"));
    KMainWindow::saveMainWindowSettings(group);

    // use the standard toolbar editor
    KEditToolBar dlg(factory());
    connect(&dlg, &KEditToolBar::newToolBarConfig, this, &KGraphViewerWindow::applyNewToolbarConfig);
    dlg.exec();
}

void KGraphViewerWindow::optionsConfigure()
{
    // An instance of your dialog could be already created and could be cached,
    // in which case you want to display the cached dialog instead of creating
    // another one
    if (KgvConfigurationDialog::showDialog("settings"))
        return;

    // KConfigDialog didn't find an instance of this dialog, so lets create it :
    KgvConfigurationDialog *dialog = new KgvConfigurationDialog(this, "settings", KGraphViewerSettings::self());
    connect(dialog, &KgvConfigurationDialog::backgroundColorChanged, this, &KGraphViewerWindow::slotBackgroundColorChanged);
    Ui::KGraphViewerPreferencesParsingWidget *parsingWidget = dialog->parsingWidget;
    qCDebug(KGRAPHVIEWER_LOG) << KGraphViewerSettings::parsingMode();
    if (KGraphViewerSettings::parsingMode() == "external") {
        parsingWidget->external->setChecked(true);
    } else if (KGraphViewerSettings::parsingMode() == "internal") {
        parsingWidget->internal->setChecked(true);
    }
    connect(parsingWidget->external, &QRadioButton::toggled, this, &KGraphViewerWindow::slotParsingModeExternalToggled);
    connect(parsingWidget->internal, &QRadioButton::toggled, this, &KGraphViewerWindow::slotParsingModeInternalToggled);

    Ui::KGraphViewerPreferencesReloadWidget *reloadWidget = dialog->reloadWidget;
    qCDebug(KGRAPHVIEWER_LOG) << KGraphViewerSettings::reloadOnChangeMode();
    if (KGraphViewerSettings::reloadOnChangeMode() == "true") {
        reloadWidget->yes->setChecked(true);
    } else if (KGraphViewerSettings::reloadOnChangeMode() == "false") {
        reloadWidget->no->setChecked(true);
    } else // if (KGraphViewerSettings::reloadOnChangeMode() == "ask")
    {
        reloadWidget->ask->setChecked(true);
    }

    connect(reloadWidget->yes, &QRadioButton::toggled, this, &KGraphViewerWindow::slotReloadOnChangeModeYesToggled);
    connect(reloadWidget->no, &QRadioButton::toggled, this, &KGraphViewerWindow::slotReloadOnChangeModeNoToggled);
    connect(reloadWidget->ask, &QRadioButton::toggled, this, &KGraphViewerWindow::slotReloadOnChangeModeAskToggled);

    Ui::KGraphViewerPreferencesOpenInExistingWindowWidget *openingWidget = dialog->openingWidget;
    if (KGraphViewerSettings::openInExistingWindowMode() == "true") {
        openingWidget->yes->setChecked(true);
    } else if (KGraphViewerSettings::openInExistingWindowMode() == "false") {
        openingWidget->no->setChecked(true);
    } else // if (KGraphViewerSettings::openInExistingWindowMode() == "ask")
    {
        openingWidget->ask->setChecked(true);
    }

    connect(openingWidget->yes, &QRadioButton::toggled, this, &KGraphViewerWindow::slotOpenInExistingWindowModeYesToggled);
    connect(openingWidget->no, &QRadioButton::toggled, this, &KGraphViewerWindow::slotOpenInExistingWindowModeNoToggled);
    connect(openingWidget->ask, &QRadioButton::toggled, this, &KGraphViewerWindow::slotOpenInExistingWindowModeAskToggled);

    Ui::KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget *reopeningWidget = dialog->reopeningWidget;
    if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "true") {
        reopeningWidget->yes->setChecked(true);
    } else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "false") {
        reopeningWidget->no->setChecked(true);
    } else // if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "ask")
    {
        reopeningWidget->ask->setChecked(true);
    }

    connect(reopeningWidget->yes, &QRadioButton::toggled, this, &KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeYesToggled);
    connect(reopeningWidget->no, &QRadioButton::toggled, this, &KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeNoToggled);
    connect(reopeningWidget->ask, &QRadioButton::toggled, this, &KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeAskToggled);

    dialog->show();
}

void KGraphViewerWindow::applyNewToolbarConfig()
{
    applyMainWindowSettings(KSharedConfig::openConfig()->group("kgraphviewer"));
}

void KGraphViewerWindow::slotReloadOnChangeModeYesToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG);
    if (value) {
        KGraphViewerSettings::setReloadOnChangeMode("true");
    }
    //   qCDebug(KGRAPHVIEWER_LOG) << "emitting";
    //   emit(settingsChanged());
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReloadOnChangeModeNoToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG);
    if (value) {
        KGraphViewerSettings::setReloadOnChangeMode("false");
    }
    //   qCDebug(KGRAPHVIEWER_LOG) << "emitting";
    //   emit(settingsChanged());
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReloadOnChangeModeAskToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG);
    if (value) {
        KGraphViewerSettings::setReloadOnChangeMode("ask");
    }
    //   qCDebug(KGRAPHVIEWER_LOG) << "emitting";
    //   emit(settingsChanged());
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotOpenInExistingWindowModeYesToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG) << value;
    if (value) {
        KGraphViewerSettings::setOpenInExistingWindowMode("true");
    }
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotOpenInExistingWindowModeNoToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG) << value;
    if (value) {
        KGraphViewerSettings::setOpenInExistingWindowMode("false");
    }
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotOpenInExistingWindowModeAskToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG) << value;
    if (value) {
        KGraphViewerSettings::setOpenInExistingWindowMode("ask");
    }
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeYesToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG) << value;
    if (value) {
        KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("true");
    }
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeNoToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG) << value;
    if (value) {
        KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("false");
    }
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeAskToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG) << value;
    if (value) {
        KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("ask");
    }
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotParsingModeExternalToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG);
    if (value) {
        KGraphViewerSettings::setParsingMode("external");
    }
    //   qCDebug(KGRAPHVIEWER_LOG) << "emitting";
    //   emit(settingsChanged());
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotParsingModeInternalToggled(bool value)
{
    qCDebug(KGRAPHVIEWER_LOG);
    if (value) {
        KGraphViewerSettings::setParsingMode("internal");
    }
    //   qCDebug(KGRAPHVIEWER_LOG) << "emitting";
    //   emit(settingsChanged());
    KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotURLSelected(const QUrl &url)
{
    openUrl(url);
}

void KGraphViewerWindow::close(int index)
{
    QWidget *tab = m_widget->widget(index);
    m_openedFiles.removeAll(m_tabsFilesMap[tab]);
    m_widget->removeTab(index);
    tab->hide();
    KParts::Part *part = m_tabsPartsMap[tab];
    m_manager->removePart(part);
    m_tabsPartsMap.remove(tab);
    m_tabsFilesMap.remove(tab);
    delete part;
    part = nullptr;
    /*  delete tab;
      tab = nullptr;*/
    m_closeAction->setEnabled(m_widget->count() > 0);
}

void KGraphViewerWindow::close()
{
    int currentPage = m_widget->currentIndex();
    if (currentPage != -1) {
        close(currentPage);
    }
}

void KGraphViewerWindow::newTabSelectedSlot(int index)
{
    emit(hide((KParts::Part *)(m_manager->activePart())));

    QWidget *tab = m_widget->widget(index);
    if (tab) {
        m_manager->setActivePart(m_tabsPartsMap[tab]);
    }
}

void KGraphViewerWindow::slotHoverEnter(const QString &id)
{
    qCDebug(KGRAPHVIEWER_LOG) << id;
    statusBar()->showMessage(id);
}

void KGraphViewerWindow::slotHoverLeave(const QString &id)
{
    qCDebug(KGRAPHVIEWER_LOG) << id;
    statusBar()->showMessage("");
}

void KGraphViewerWindow::slotBackgroundColorChanged(const QColor &)
{
    qCDebug(KGRAPHVIEWER_LOG);
    foreach (KParts::Part *part, m_tabsPartsMap) {
        KGraphViewer::KGraphViewerInterface *kgv = qobject_cast<KGraphViewer::KGraphViewerInterface *>(part);
        if (!kgv) {
            // This should not happen
            return;
        }
        kgv->setBackgroundColor(KGraphViewerSettings::backgroundColor());
    }
}

#include "moc_kgraphviewer.cpp"
