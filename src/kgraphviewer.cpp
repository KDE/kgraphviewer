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
#include "kgraphviewersettings.h"
#include "ui_preferencesParsing.h"
#include "ui_preferencesReload.h"
#include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesReopenPreviouslyOpenedFiles.h"

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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

KGraphViewer::KGraphViewer()
    : KParts::MainWindow(),
      m_rfa(0)
{
  // set the shell's ui resource file
  setXMLFile("kgraphviewerui.rc");

//   std::cerr << "Creating tab widget" << std::endl;
  m_widget = new KTabWidget(this);
  m_widget->setHoverCloseButton(true);
  connect(m_widget, SIGNAL(closeRequest(QWidget*)), this, SLOT(close(QWidget*)));
  connect(m_widget, SIGNAL(currentChanged(QWidget*)), this, SLOT(newTabSelectedSlot(QWidget*)));
  
  setCentralWidget(m_widget);
  
  if (QDBusConnection::sessionBus().registerService( "org.kde.kgraphviewer" ))
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
}

KGraphViewer::~KGraphViewer()
{
  KSharedConfig::Ptr config = KGlobal::config();
  if (m_rfa != 0)
    m_rfa->saveEntries(KConfigGroup(config, "kgraphviewer recent files"));
}

void KGraphViewer::reloadPreviousFiles()
{
  QStringList previouslyOpenedFiles = KGraphViewerSettings::previouslyOpenedFiles();
  if ( (previouslyOpenedFiles.empty() == false) 
       && (KMessageBox::questionYesNo(this, 
              i18n("Do you want to reload files from the previous session?"),
              i18n("Reload Confirmation"),
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
    KGraphViewerSettings::self()->writeConfig();
  }
  
}

void KGraphViewer::openUrl(const KUrl& url)
{
  kDebug() << url;
  KLibFactory *factory = KLibLoader::self()->factory("kgraphviewerpart");
  if (factory)
  {
    KParts::ReadOnlyPart* part = static_cast<KParts::ReadOnlyPart*>(factory->create(this, "kgraphviewerpart"));
    
    if (part)
    {
      QString label = url.url().section('/',-1,-1);
      m_widget-> insertTab(part->widget(), QIcon( DesktopIcon("kgraphviewer") ), label);
      m_widget->setCurrentPage(m_widget->indexOf(part->widget()));
      createGUI(part);
      
      (KGraphViewerSettings::parsingMode() == "external")
        ?openUrlCommand(url,part)
        :openUrlLibrary(url,part);

        m_openedFiles.push_back(url.url());
        m_manager->addPart( part, true );
        m_tabsPartsMap[m_widget->currentPage()] = part;
        m_tabsFilesMap[m_widget->currentPage()] = url.url();
        connect(this,SIGNAL(hide(KParts::Part*)),part,SLOT(slotHide(KParts::Part*)));
        connect(part,SIGNAL(close()),this,SLOT(close()));
    }
  }
  else
  {
    // if we couldn't find our Part, we exit since the Shell by
    // itself can't do anything useful
    KMessageBox::error(this, i18n("Could not find the KGraphViewer part."));
    kapp->quit();
    // we return here, cause kapp->quit() only means "exit the
    // next time we enter the event loop...
    return;
  }
}

void KGraphViewer::openUrlCommand(const KUrl& url, KParts::ReadOnlyPart* part)
{
  kDebug() << url;
  part->openUrl( url );
}

void KGraphViewer::openUrlLibrary(const KUrl& url, KParts::ReadOnlyPart* part)
{
  kDebug() << "in openUrlLibrary" << url;
  connect(this, SIGNAL(loadLibrary(graph_t*)),part,SLOT(slotLoadLibrary(graph_t*)));
  GVC_t *gvc;
  graph_t *g;
  FILE* fp;
  gvc = gvContext();
  fp = fopen(url.pathOrUrl().toUtf8().data(), "r");
  kDebug() << "fopen" << url.pathOrUrl() << (void*)fp;
  g = agread(fp);
  emit(loadLibrary(g));
}

void KGraphViewer::fileOpen()
{
  kDebug() ;
  // this slot is called whenever the File->Open menu is selected,
  // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
  // button is clicked
  QStringList file_names = KFileDialog::getOpenFileNames(KUrl(QString()), QString("*.dot"), 0, QString::null);
  
  if (!file_names.empty())
  {
    QStringList::const_iterator it, it_end;
    it = file_names.constBegin(); it_end = file_names.constEnd();
    for (; it != it_end; it++)
    {
      if (m_rfa != 0)
      {
        m_rfa->addUrl(*it);
      }
      openUrl(*it);
    }
  }
}

void KGraphViewer::setupActions()
{
  // create our actions
  KAction* newAction = actionCollection()->addAction( KStandardAction::New, "file_new", this, SLOT( fileNew() ) );
  newAction->setWhatsThis(i18n("Opens a new empty KGraphViewer window."));
  
  KAction* openAction = actionCollection()->addAction( KStandardAction::Open, "file_open", this, SLOT( fileOpen() ) );
  openAction->setWhatsThis(i18n("Shows the file open dialog to choose a GraphViz dot file to open."));
  
  m_rfa = KStandardAction::openRecent(this, SLOT( slotURLSelected(const KUrl&) ), this);
  actionCollection()->addAction(m_rfa->objectName(),m_rfa);
  m_rfa->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));

  KSharedConfig::Ptr config = KGlobal::config();
  m_rfa->loadEntries(KConfigGroup(config, "kgraphviewer recent files"));
  
  KAction* quitAction = actionCollection()->addAction( KStandardAction::Quit, "file_quit", KApplication::kApplication(), SLOT( quit() ) );
  quitAction->setWhatsThis(i18n("Quits KGraphViewer."));
  
  m_statusbarAction = KStandardAction::showStatusbar(this, SLOT(optionsShowStatusbar()), this);
  m_statusbarAction->setWhatsThis(i18n("Shows or hide the status bar."));
  
  KAction* kbAction = actionCollection()->addAction( KStandardAction::KeyBindings, "options_configure_keybinding", this, SLOT( optionsConfigureKeys() ) );
  kbAction->setWhatsThis(i18n("Configure the bindings between keys and actions."));
  
  KAction* ctAction = actionCollection()->addAction( KStandardAction::ConfigureToolbars, "options_configure_toolbars", this, SLOT( optionsConfigureToolbars() ) );
  ctAction->setWhatsThis(i18n("Tollbars configuration ."));
  
  KAction* configureAction = actionCollection()->addAction( KStandardAction::Preferences, "options_configure", this, SLOT( optionsConfigure() ) );
  configureAction->setWhatsThis(i18n("Main KGraphViewer configuration options."));
  

}

bool KGraphViewer::queryExit()
{
//   std::cerr << "queryExit" << std::endl;
  KGraphViewerSettings::setPreviouslyOpenedFiles(m_openedFiles);
  KGraphViewerSettings::self()->writeConfig();
  return true;
}

void KGraphViewer::fileNew()
{
  // this slot is called whenever the File->New menu is selected,
  // the New shortcut is pressed (usually CTRL+N) or the New toolbar
  // button is clicked

  (new KGraphViewer)->show();
}


void KGraphViewer::optionsShowToolbar()
{
  // this is all very cut and paste code for showing/hiding the
  // toolbar
  if (m_toolbarAction->isChecked())
      toolBar()->show();
  else
      toolBar()->hide();
}

void KGraphViewer::optionsShowStatusbar()
{
  // this is all very cut and paste code for showing/hiding the
  // statusbar
  if (m_statusbarAction->isChecked())
      statusBar()->show();
  else
      statusBar()->hide();
}

void KGraphViewer::optionsConfigureKeys()
{
  KShortcutsDialog::configure(actionCollection());
}

void KGraphViewer::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("kgraphviewer") );

  // use the standard toolbar editor
  KEditToolBar dlg(factory());
  connect(&dlg, SIGNAL(newToolbarConfig()),
          this, SLOT(applyNewToolbarConfig()));
  dlg.exec();
}

void KGraphViewer::optionsConfigure()
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

  Ui::KGraphViewerPreferencesParsingWidget*  parsingWidget = dialog->parsingWidget;
  kDebug() << KGraphViewerSettings::parsingMode();
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
  kDebug() << KGraphViewerSettings::reloadOnChangeMode();
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

void KGraphViewer::applyNewToolbarConfig()
{
  applyMainWindowSettings(KGlobal::config()->group("kgraphviewer"));
}

void KGraphViewer::slotReloadOnChangeModeYesToggled(bool value)
{
  kDebug();
  if (value)
  {
    KGraphViewerSettings::setReloadOnChangeMode("true");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotReloadOnChangeModeNoToggled(bool value)
{
  kDebug();
  if (value)
  {
    KGraphViewerSettings::setReloadOnChangeMode("false");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotReloadOnChangeModeAskToggled(bool value)
{
  kDebug();
  if (value)
  {
    KGraphViewerSettings::setReloadOnChangeMode("ask");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotOpenInExistingWindowModeYesToggled(bool value)
{
  kDebug() << value;
  if (value)
  {
    KGraphViewerSettings::setOpenInExistingWindowMode("true");
  }
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotOpenInExistingWindowModeNoToggled(bool value)
{
  kDebug() << value;
  if (value)
  {
    KGraphViewerSettings::setOpenInExistingWindowMode("false");
  }
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotOpenInExistingWindowModeAskToggled(bool value)
{
  kDebug() << value;
  if (value)
  {
    KGraphViewerSettings::setOpenInExistingWindowMode("ask");
  }
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotReopenPreviouslyOpenedFilesModeYesToggled(bool value)
{
  kDebug() << value;
  if (value)
  {
    KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("true");
  }
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotReopenPreviouslyOpenedFilesModeNoToggled(bool value)
{
  kDebug() << value;
  if (value)
  {
    KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("false");
  }
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotReopenPreviouslyOpenedFilesModeAskToggled(bool value)
{
  kDebug() << value;
  if (value)
  {
    KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("ask");
  }
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotParsingModeExternalToggled(bool value)
{
  kDebug();
  if (value)
  {
    KGraphViewerSettings::setParsingMode("external");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->writeConfig();
}

void KGraphViewer::slotParsingModeInternalToggled(bool value)
{
  kDebug();
  if (value)
  {
    KGraphViewerSettings::setParsingMode("internal");
  }
  //   kDebug() << "emiting";
  //   emit(settingsChanged());
  KGraphViewerSettings::self()->writeConfig();
}



void KGraphViewer::slotURLSelected(const KUrl& url)
{
  openUrl(url);
}

void KGraphViewer::close(QWidget* tab)
{
  kDebug() << tab;
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

void KGraphViewer::close()
{
  kDebug();
  QWidget* currentPage = m_widget->currentPage();
  if (currentPage != 0)
  {
    close(currentPage);
  }
}

void KGraphViewer::newTabSelectedSlot(QWidget* tab)
{
  kDebug() << tab;
  emit(hide((KParts::Part*)(m_manager->activePart())));
  if (!m_tabsPartsMap.isEmpty())
  {
    m_manager->setActivePart(m_tabsPartsMap[tab]);
  }
}

#include "kgraphviewer.moc"
