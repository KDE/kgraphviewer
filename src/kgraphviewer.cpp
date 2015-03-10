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
#include "part/kgraphviewer_interface.h"
#include "kgraphviewerConfigDialog.h"
#include "kgraphviewersettings.h"
#include "ui_preferencesParsing.h"
#include "ui_preferencesReload.h"
#include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesReopenPreviouslyOpenedFiles.h"

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
#include <KService>
#include <KPluginFactory>
#include <kmessagebox.h>
#include <QStatusBar>
#include <kconfigdialog.h>
#include <QIcon>
#include <krecentfilesaction.h>
#include <ktoolbar.h>
#include <KActionCollection>
#include <klocalizedstring.h>
#include <QtDBus/QtDBus>
#include <KParts/ReadOnlyPart>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <KColorScheme>
#include <QLoggingCategory>

static QLoggingCategory debugCategory("org.kde.kgraphviewer");

KGraphViewerWindow::KGraphViewerWindow()
    : KParts::MainWindow(),
      m_rfa(0)
{
  // set the shell's ui resource file
  setXMLFile("kgraphviewerui.rc");

//   std::cerr << "Creating tab widget" << std::endl;
  m_widget = new QTabWidget(this);
  m_widget->setTabsClosable(true);
  connect(m_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(close(int)));
  connect(m_widget, SIGNAL(currentChanged(int)), this, SLOT(newTabSelectedSlot(int)));
  
  setCentralWidget(m_widget);
  
  if (QDBusConnection::sessionBus().registerService( "org.kde.kgraphviewer" ))
  {
    qCDebug(debugCategory) << "Service Registered successfully";
    QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);
    
  }
  else
  {
    qCDebug(debugCategory) << "Failed to register service...";
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
  connect( m_manager, SIGNAL(activePartChanged(KParts::Part*)),
           this, SLOT(createGUI(KParts::Part*)) );
    
  // Creates the GUI with a null part to make appear the main app menus and tools
  createGUI(0);
  setupGUI();  
  // apply the saved mainwindow settings, if any, and ask the mainwindow
  // to automatically save settings if changed: window size, toolbar
  // position, icon size, etc.
  setAutoSaveSettings();
}

KGraphViewerWindow::~KGraphViewerWindow()
{
  KSharedConfig::Ptr config = KSharedConfig::openConfig();
  if (m_rfa != 0)
    m_rfa->saveEntries(KConfigGroup(config, "kgraphviewer recent files"));
}

void KGraphViewerWindow::reloadPreviousFiles()
{
  QStringList previouslyOpenedFiles = KGraphViewerSettings::previouslyOpenedFiles();
  if ( (previouslyOpenedFiles.empty() == false) 
       && (KMessageBox::questionYesNo(this, 
              i18n("Do you want to reload files from the previous session?"),
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
    KGraphViewerSettings::self()->save();
  }
  
}

void KGraphViewerWindow::openUrl(const QUrl& url)
{
  qCDebug(debugCategory) << url;
  KPluginFactory *factory = KPluginLoader("kgraphviewerpart").factory();
  if (!factory)
  {
    // if we couldn't find our Part, we exit since the Shell by
    // itself can't do anything useful
    KMessageBox::error(this, i18n("Could not find the KGraphViewer part."));
    qApp->quit();
    // we return here, cause kapp->quit() only means "exit the
    // next time we enter the event loop...
    return;
  }
    KParts::ReadOnlyPart* part = factory->create<KParts::ReadOnlyPart>("kgraphviewerpart", this);
    KGraphViewer::KGraphViewerInterface* kgv = qobject_cast<KGraphViewer::KGraphViewerInterface*>( part );
    if( ! kgv )
    {
      // This should not happen
      qWarning() << "Failed to get KPart" << endl;
      return;
    }
    kgv->setBackgroundColor(KGraphViewerSettings::backgroundColor());
    (KGraphViewerSettings::parsingMode() == "external")
        ?kgv->setLayoutMethod(KGraphViewer::KGraphViewerInterface::ExternalProgram)
        :kgv->setLayoutMethod(KGraphViewer::KGraphViewerInterface::InternalLibrary);

    if (part)
    {
      QString fileName = url.url();
      QString label = fileName.section('/',-1,-1);
      QWidget *w = part->widget();
      m_widget->addTab(w, QIcon::fromTheme("kgraphviewer"), label);
      m_widget->setCurrentWidget(w);
      createGUI(part);

      part->openUrl( url );
      
      if (m_rfa != 0)
      {
        m_rfa->addUrl(url);
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        m_rfa->saveEntries(KConfigGroup(config, "kgraphviewer recent files"));
      }

      m_openedFiles.push_back(fileName);
      m_manager->addPart( part, true );
      m_tabsPartsMap[w] = part;
      m_tabsFilesMap[w] = fileName;
      connect(this,SIGNAL(hide(KParts::Part*)),part,SLOT(slotHide(KParts::Part*)));
      connect(part,SIGNAL(close()),this,SLOT(close()));

      connect(part, SIGNAL(hoverEnter(QString)), this, SLOT(slotHoverEnter(QString)));
      connect(part, SIGNAL(hoverLeave(QString)), this, SLOT(slotHoverLeave(QString)));
    }
}

void KGraphViewerWindow::fileOpen()
{
  qCDebug(debugCategory) ;
  // this slot is called whenever the File->Open menu is selected,
  // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
  // button is clicked
  QStringList file_names = QFileDialog::getOpenFileNames(
      this,
      i18n("Select DOT File"),
      QString(),
      QString("*.dot"));
  
  if (!file_names.empty())
  {
    QStringList::const_iterator it, it_end;
    it = file_names.constBegin(); it_end = file_names.constEnd();
    for (; it != it_end; it++)
    {
      openUrl(*it);
    }
  }
}

void KGraphViewerWindow::setupActions()
{
  // create our actions
  QAction* newAction = actionCollection()->addAction( KStandardAction::New, "file_new", this, SLOT(fileNew()) );
  newAction->setWhatsThis(i18n("Opens a new empty KGraphViewer window."));
  
  QAction* openAction = actionCollection()->addAction( KStandardAction::Open, "file_open", this, SLOT(fileOpen()) );
  openAction->setWhatsThis(i18n("Shows the file open dialog to choose a GraphViz dot file to open."));
  
  m_rfa = KStandardAction::openRecent(this, SLOT(slotURLSelected(QUrl)), this);
  actionCollection()->addAction(m_rfa->objectName(),m_rfa);
  m_rfa->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));

  KSharedConfig::Ptr config = KSharedConfig::openConfig();
  m_rfa->loadEntries(KConfigGroup(config, "kgraphviewer recent files"));
  
  QAction* quitAction = actionCollection()->addAction( KStandardAction::Quit, "file_quit", qApp, SLOT(quit()) );
  quitAction->setWhatsThis(i18n("Quits KGraphViewer."));
  
  m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), this);
  m_statusbarAction->setWhatsThis(i18n("Shows or hides the status bar."));
  
  QAction* kbAction = actionCollection()->addAction( KStandardAction::KeyBindings, "options_configure_keybinding", this, SLOT(optionsConfigureKeys()) );
  kbAction->setWhatsThis(i18n("Configure the bindings between keys and actions."));
  
  QAction* ctAction = actionCollection()->addAction( KStandardAction::ConfigureToolbars, "options_configure_toolbars", this, SLOT(optionsConfigureToolbars()) );
  ctAction->setWhatsThis(i18n("Toolbar configuration."));
  
  QAction* configureAction = actionCollection()->addAction( KStandardAction::Preferences, "options_configure", this, SLOT(optionsConfigure()) );
  configureAction->setWhatsThis(i18n("Main KGraphViewer configuration options."));
}

void KGraphViewerWindow::closeEvent(QCloseEvent* event)
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

void KGraphViewerWindow::optionsConfigureKeys()
{
  KShortcutsDialog::configure(actionCollection());
}

void KGraphViewerWindow::optionsConfigureToolbars()
{
  KConfigGroup group(KConfigGroup(KSharedConfig::openConfig(), "kgraphviewer"));
  KMainWindow::saveMainWindowSettings(group);

  // use the standard toolbar editor
  KEditToolBar dlg(factory());
  connect(&dlg, SIGNAL(newToolbarConfig()),
          this, SLOT(applyNewToolbarConfig()));
  dlg.exec();
}

void KGraphViewerWindow::optionsConfigure()
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
  connect(dialog,SIGNAL(backgroundColorChanged(QColor)),this,SLOT(slotBackgroundColorChanged(QColor)));
  Ui::KGraphViewerPreferencesParsingWidget*  parsingWidget = dialog->parsingWidget;
  qCDebug(debugCategory) << KGraphViewerSettings::parsingMode();
  if (KGraphViewerSettings::parsingMode() == "external")
  {
    parsingWidget->external->setChecked(true);
  }
  else if (KGraphViewerSettings::parsingMode() == "internal")
  {
    parsingWidget->internal->setChecked(true);
  }
  connect((QObject*)parsingWidget->external, SIGNAL(toggled(bool)), this, SLOT(slotParsingModeExternalToggled(bool)) );
  connect((QObject*)parsingWidget->internal, SIGNAL(toggled(bool)), this, SLOT(slotParsingModeInternalToggled(bool)) );

  Ui::KGraphViewerPreferencesReloadWidget*  reloadWidget = dialog->reloadWidget;
  qCDebug(debugCategory) << KGraphViewerSettings::reloadOnChangeMode();
  if (KGraphViewerSettings::reloadOnChangeMode() == "true")
  {
    reloadWidget->yes->setChecked(true);
  }
  else if (KGraphViewerSettings::reloadOnChangeMode() == "false")
  {
    reloadWidget->no->setChecked(true);
  }
  else // if (KGraphViewerSettings::reloadOnChangeMode() == "ask")
  {
    reloadWidget->ask->setChecked(true);
  }
  
  connect((QObject*)reloadWidget->yes, SIGNAL(toggled(bool)), this, SLOT(slotReloadOnChangeModeYesToggled(bool)) );
  connect((QObject*)reloadWidget->no, SIGNAL(toggled(bool)), this, SLOT(slotReloadOnChangeModeNoToggled(bool)) );
  connect((QObject*)reloadWidget->ask, SIGNAL(toggled(bool)), this, SLOT(slotReloadOnChangeModeAskToggled(bool)) );

  Ui::KGraphViewerPreferencesOpenInExistingWindowWidget*  openingWidget = dialog->openingWidget;
  if (KGraphViewerSettings::openInExistingWindowMode() == "true")
  {
    openingWidget->yes->setChecked(true);
  }
  else if (KGraphViewerSettings::openInExistingWindowMode() == "false")
  {
    openingWidget->no->setChecked(true);
  }
  else // if (KGraphViewerSettings::openInExistingWindowMode() == "ask")
  {
    openingWidget->ask->setChecked(true);
  }
  
  connect((QObject*)openingWidget->yes, SIGNAL(toggled(bool)), this, SLOT(slotOpenInExistingWindowModeYesToggled(bool)) );
  connect((QObject*)openingWidget->no, SIGNAL(toggled(bool)), this, SLOT(slotOpenInExistingWindowModeNoToggled(bool)) );
  connect((QObject*)openingWidget->ask, SIGNAL(toggled(bool)), this, SLOT(slotOpenInExistingWindowModeAskToggled(bool)) );
  
  Ui::KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget*  reopeningWidget = dialog->reopeningWidget;
  if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "true")
  {
    reopeningWidget->yes->setChecked(true);
  }
  else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "false")
  {
    reopeningWidget->no->setChecked(true);
  }
  else // if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "ask")
  {
    reopeningWidget->ask->setChecked(true);
  }
  
  connect((QObject*)reopeningWidget->yes, SIGNAL(toggled(bool)), this, SLOT(slotReopenPreviouslyOpenedFilesModeYesToggled(bool)) );
  connect((QObject*)reopeningWidget->no, SIGNAL(toggled(bool)), this, SLOT(slotReopenPreviouslyOpenedFilesModeNoToggled(bool)) );
  connect((QObject*)reopeningWidget->ask, SIGNAL(toggled(bool)), this, SLOT(slotReopenPreviouslyOpenedFilesModeAskToggled(bool)) );
  
  connect(dialog, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));

  dialog->show();
}

void KGraphViewerWindow::applyNewToolbarConfig()
{
  applyMainWindowSettings(KSharedConfig::openConfig()->group("kgraphviewer"));
}

void KGraphViewerWindow::slotReloadOnChangeModeYesToggled(bool value)
{
  qCDebug(debugCategory);
  if (value)
  {
    KGraphViewerSettings::setReloadOnChangeMode("true");
  }
  //   qCDebug(debugCategory) << "emitting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReloadOnChangeModeNoToggled(bool value)
{
  qCDebug(debugCategory);
  if (value)
  {
    KGraphViewerSettings::setReloadOnChangeMode("false");
  }
  //   qCDebug(debugCategory) << "emitting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReloadOnChangeModeAskToggled(bool value)
{
  qCDebug(debugCategory);
  if (value)
  {
    KGraphViewerSettings::setReloadOnChangeMode("ask");
  }
  //   qCDebug(debugCategory) << "emitting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotOpenInExistingWindowModeYesToggled(bool value)
{
  qCDebug(debugCategory) << value;
  if (value)
  {
    KGraphViewerSettings::setOpenInExistingWindowMode("true");
  }
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotOpenInExistingWindowModeNoToggled(bool value)
{
  qCDebug(debugCategory) << value;
  if (value)
  {
    KGraphViewerSettings::setOpenInExistingWindowMode("false");
  }
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotOpenInExistingWindowModeAskToggled(bool value)
{
  qCDebug(debugCategory) << value;
  if (value)
  {
    KGraphViewerSettings::setOpenInExistingWindowMode("ask");
  }
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeYesToggled(bool value)
{
  qCDebug(debugCategory) << value;
  if (value)
  {
    KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("true");
  }
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeNoToggled(bool value)
{
  qCDebug(debugCategory) << value;
  if (value)
  {
    KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("false");
  }
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotReopenPreviouslyOpenedFilesModeAskToggled(bool value)
{
  qCDebug(debugCategory) << value;
  if (value)
  {
    KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("ask");
  }
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotParsingModeExternalToggled(bool value)
{
  qCDebug(debugCategory);
  if (value)
  {
    KGraphViewerSettings::setParsingMode("external");
  }
  //   qCDebug(debugCategory) << "emitting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->save();
}

void KGraphViewerWindow::slotParsingModeInternalToggled(bool value)
{
  qCDebug(debugCategory);
  if (value)
  {
    KGraphViewerSettings::setParsingMode("internal");
  }
  //   qCDebug(debugCategory) << "emitting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->save();
}



void KGraphViewerWindow::slotURLSelected(const QUrl& url)
{
  openUrl(url);
}

void KGraphViewerWindow::close(int index)
{
  QWidget *tab = m_widget->widget(index);
  m_openedFiles.removeAll(m_tabsFilesMap[tab]);
  m_widget->removeTab(index);
  tab->hide();
  KParts::Part* part = m_tabsPartsMap[tab];
  m_manager->removePart(part);
  m_tabsPartsMap.remove(tab);
  m_tabsFilesMap.remove(tab);
  delete part; part=0;
/*  delete tab;
  tab = 0;*/
}

void KGraphViewerWindow::close()
{
  int currentPage = m_widget->currentIndex();
  if (currentPage != -1)
  {
    close(currentPage);
  }
}

void KGraphViewerWindow::newTabSelectedSlot(int index)
{
  emit(hide((KParts::Part*)(m_manager->activePart())));
  if (!m_tabsPartsMap.isEmpty())
  {
    QWidget *tab = m_widget->widget(index);
    m_manager->setActivePart(m_tabsPartsMap[tab]);
  }
}

void KGraphViewerWindow::slotHoverEnter(const QString& id)
{
  qCDebug(debugCategory) << id;
  statusBar()->showMessage(id);
}

void KGraphViewerWindow::slotHoverLeave(const QString& id)
{
  qCDebug(debugCategory) << id;
  statusBar()->showMessage("");
}

void KGraphViewerWindow::slotBackgroundColorChanged(const QColor&)
{
  qCDebug(debugCategory);
  foreach(KParts::Part* part, m_tabsPartsMap)
  {
    KGraphViewer::KGraphViewerInterface* kgv = qobject_cast<KGraphViewer::KGraphViewerInterface*>( part );
    if( ! kgv )
    {
      // This should not happen
      return;
    }
    kgv->setBackgroundColor(KGraphViewerSettings::backgroundColor());
  }
}

#include "kgraphviewer.moc"
