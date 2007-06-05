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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "kgraphviewer.h"
#include "kgraphviewerConfigDialog.h"
#include "kgraphviewersettings.h"
#include "preferencesReload.h"
#include "preferencesOpenInExistingWindow.h"
#include "preferencesReopenPreviouslyOpenedFiles.h"

#include <kkeydialog.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <ktabwidget.h>
#include <kparts/partmanager.h>
#include <kedittoolbar.h>
#include <kaccel.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfigdialog.h>
#include <kiconloader.h>
#include <kapp.h>
#include <dcopclient.h>

#include <qbuttongroup.h>
#include <iostream>

using namespace KGraphViewer;

kgraphviewer::kgraphviewer()
    : KParts::MainWindow( 0L, "kgraphviewer" ),
    DCOPObject("DCOPKGraphViewerIface")
{
//   std::cerr << "Creating tab widget" << std::endl;
  m_widget = new KTabWidget(this,"CentralTabWidget");
  m_widget->setHoverCloseButton(true);
  connect(m_widget, SIGNAL(closeRequest(QWidget*)), this, SLOT(close(QWidget*)));
  connect(m_widget, SIGNAL(currentChanged(QWidget*)), this, SLOT(newTabSelectedSlot(QWidget*)));
  
  setCentralWidget(m_widget);
  
  // Register with DCOP
  if ( !kapp->dcopClient()->isRegistered() ) 
  {
    kapp->dcopClient()->registerAs( "dcopkgraphviewer" );
    kapp->dcopClient()->setDefaultObject( objId() );
  }
  
  
  // set the shell's ui resource file
  setXMLFile("kgraphviewer_shell.rc");

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
  
  // apply the saved mainwindow settings, if any, and ask the mainwindow
  // to automatically save settings if changed: window size, toolbar
  // position, icon size, etc.
  setAutoSaveSettings();
  
}

kgraphviewer::~kgraphviewer()
{
  m_rfa->saveEntries(KGlobal::config());
}

void kgraphviewer::reloadPreviousFiles()
{
  QStringList previouslyOpenedFiles = KGraphViewerSettings::previouslyOpenedFiles();
  if ( (previouslyOpenedFiles.empty() == false) 
       && (KMessageBox::questionYesNo(this, 
              i18n("Do you want to reload files from previous session ?"),
              i18n("Reload Confirmation"),
              KStdGuiItem::yes(),
              KStdGuiItem::no(),
              "reopenPreviouslyOpenedFilesMode"   ) == KMessageBox::Yes) )
  {
    QStringList::const_iterator it, it_end;
    it = previouslyOpenedFiles.begin(); it_end = previouslyOpenedFiles.end();
    for (; it != it_end; it++)
    {
      openURL(*it);
    }
  }
  
}

void kgraphviewer::openURL(const KURL& url)
{
//   std::cerr << "kgraphviewer::openURL" << std::endl;
  KLibFactory *factory = KLibLoader::self()->factory("libkgraphviewerpart");
  if (factory)
  {
    KParts::ReadOnlyPart* part = static_cast<KParts::ReadOnlyPart*>(factory->create(this, "kgraphviewer_part", 
      "KParts::ReadOnlyPart" ));
    
    if (part)
    {
      QString label = url.url().section('/',-1,-1);
      bool loaded = part->openURL( url );
      if (loaded)
      {
        createGUI(part);
        m_openedFiles.push_back(url.url());
        m_manager->addPart( part, true );
        m_widget-> insertTab(part->widget(), QIconSet( KIconLoader().loadIcon("kgraphviewer", KIcon::NoGroup) ), label);
        m_widget->setCurrentPage(m_widget->indexOf(part->widget()));
        m_tabsPartsMap[m_widget->currentPage()] = part;
        m_tabsFilesMap[m_widget->currentPage()] = url.url();
        connect(this,SIGNAL(hide(KParts::Part*)),part,SLOT(slotHide(KParts::Part*)));
      }
      else 
      {
        delete part;
      }
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

void kgraphviewer::fileOpen()
{
//   std::cerr << "kgraphviewer::fileOpen" << std::endl;
  // this slot is called whenever the File->Open menu is selected,
  // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
  // button is clicked
  QStringList file_names = KFileDialog::getOpenFileNames(QString::null, "*.dot");
  
  if (!file_names.empty())
  {
    QStringList::const_iterator it, it_end;
    it = file_names.begin(); it_end = file_names.end();
    for (; it != it_end; it++)
    {
      if (m_rfa != 0)
      {
        m_rfa->addURL(*it);
      }
      openURL(*it);
    }
  }
}


void kgraphviewer::setupActions()
{
  // create our actions
  KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
  
  KStdAction::open(this, SLOT(fileOpen()), actionCollection());
  m_rfa = KStdAction::openRecent(this, SLOT(slotURLSelected(const KURL&)), actionCollection());
  m_rfa->loadEntries(KGlobal::config());
  
  KStdAction::close(this, SLOT(close()), actionCollection());
  KAccel *close_accel = new KAccel( this, "CloseAccel" );
  close_accel->insert( "Close", i18n("Close"), "", CTRL+Key_W, 
                       this, SLOT( close() ) );
  close_accel->readSettings();
  
  KStdAction::quit(kapp, SLOT(quit()), actionCollection());

  m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

  KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
  KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
  KStdAction::preferences(this, SLOT(optionsConfigure()), actionCollection());
}

bool kgraphviewer::queryExit()
{
//   std::cerr << "queryExit" << std::endl;
  KGraphViewerSettings::setPreviouslyOpenedFiles(m_openedFiles);
  KGraphViewerSettings::writeConfig();
}

void kgraphviewer::fileNew()
{
  // this slot is called whenever the File->New menu is selected,
  // the New shortcut is pressed (usually CTRL+N) or the New toolbar
  // button is clicked

  (new kgraphviewer)->show();
}


void kgraphviewer::optionsShowToolbar()
{
  // this is all very cut and paste code for showing/hiding the
  // toolbar
  if (m_toolbarAction->isChecked())
      toolBar()->show();
  else
      toolBar()->hide();
}

void kgraphviewer::optionsShowStatusbar()
{
  // this is all very cut and paste code for showing/hiding the
  // statusbar
  if (m_statusbarAction->isChecked())
      statusBar()->show();
  else
      statusBar()->hide();
}

void kgraphviewer::optionsConfigureKeys()
{
  KKeyDialog::configure(actionCollection());
}

void kgraphviewer::optionsConfigureToolbars()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    saveMainWindowSettings(KGlobal::config() );
# endif
#else
    saveMainWindowSettings(KGlobal::config() );
#endif

  // use the standard toolbar editor
  KEditToolbar dlg(factory());
  connect(&dlg, SIGNAL(newToolbarConfig()),
          this, SLOT(applyNewToolbarConfig()));
  dlg.exec();
}

void kgraphviewer::optionsConfigure()
{
  //An instance of your dialog could be already created and could be cached, 
  //in which case you want to display the cached dialog instead of creating 
  //another one 
  if ( KgvConfigurationDialog::showDialog( "settings" ) ) 
    return; 
 
  //KConfigDialog didn't find an instance of this dialog, so lets create it : 
  KgvConfigurationDialog* dialog = new KgvConfigurationDialog( this, "settings", 
                                             KGraphViewerSettings::self() ); 
  
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

void kgraphviewer::applyNewToolbarConfig()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    applyMainWindowSettings(KGlobal::config());
# endif
#else
    applyMainWindowSettings(KGlobal::config());
#endif
}

// void kgraphviewer::reloadOnChangeMode_pressed(int value)
// {
//   std::cerr << "reloadOnChangeMode_pressed " << value << std::endl;
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
//   kdError() << "Invalid reload on change mode value: " << value << endl;
//     return;
//   }
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphViewerSettings::writeConfig();
// }
// 
// void kgraphviewer::openInExistingWindowMode_pressed(int value)
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
//   kdError() << "Invalid OpenInExistingWindow value: " << value << endl;
//     return;
//   }
// 
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphViewerSettings::writeConfig();
// }
// 
// void kgraphviewer::reopenPreviouslyOpenedFilesMode_pressed(int value)
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
//   kdError() << "Invalid ReopenPreviouslyOpenedFilesMode value: " << value << endl;
//     return;
//   }
// 
//   std::cerr << "emiting" << std::endl;
//   emit(settingsChanged());
//   KGraphViewerSettings::writeConfig();
// }


void kgraphviewer::slotURLSelected(const KURL& url)
{
  openURL(url);
}

void kgraphviewer::close(QWidget* tab)
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

void kgraphviewer::close()
{
  QWidget* currentPage = m_widget->currentPage();
  if (currentPage != 0)
  {
    close(currentPage);
  }
}

void kgraphviewer::newTabSelectedSlot(QWidget* tab)
{
//   std::cerr << "newTabSelectedSlot " << tab << std::endl;
  emit(hide((KParts::Part*)(m_manager->activePart())));
  if (!m_tabsPartsMap.isEmpty())
  {
    m_manager->setActivePart(m_tabsPartsMap[tab]);
  }
}

#include "kgraphviewer.moc"
