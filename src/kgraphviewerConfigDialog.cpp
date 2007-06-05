/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gaël de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

v*/


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

#include <kstdaction.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfigdialog.h>

#include <kapp.h>
#include <dcopclient.h>

#include <qbuttongroup.h>
#include <iostream>

using namespace KGraphViewer;

KgvConfigurationDialog::KgvConfigurationDialog (QWidget *parent, const char *name, KConfigSkeleton *config, 
              DialogType dialogType, int dialogButtons, 
              ButtonCode defaultButton, bool modal) : 
  KConfigDialog (parent, name, config, dialogType, dialogButtons, defaultButton, modal) ,
  m_changed(false),
  m_reloadWidget(new KGraphViewerPreferencesReloadWidget( this, "KGraphViewerPreferencesReloadWidget" )),
  m_openingWidget(new KGraphViewerPreferencesOpenInExistingWindowWidget( this, "KGraphViewerPreferencesOpenInExistingWindowWidget" )),
  m_reopeningWidget(new KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget( 0, "KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget" ))

{
  addPage( m_reloadWidget, i18n("Reloading"), "kgraphreloadoptions", i18n("Reloading"), false); 
  addPage( m_openingWidget, i18n("Opening"), "kgraphopeningoptions", i18n("Opening"), false); 
  addPage( m_reopeningWidget, i18n("Session Management"), "kgraphreopeningoptions", i18n("Session Management"), false); 
  connect(m_reloadWidget->reloadOnChangeMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
  connect(m_openingWidget->openInExistingWindowMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
  connect(m_reopeningWidget->reopenPreviouslyOpenedFilesMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
}

KgvConfigurationDialog::~KgvConfigurationDialog () 
{
}

void KgvConfigurationDialog::settingChanged(int)
{
//   std::cerr << "KgvConfigurationDialog::settingChanged" << std::endl;
  m_changed = true;
  settingsChangedSlot();
  updateButtons ();
}

bool KgvConfigurationDialog::hasChanged()
{
//   std::cerr << "KgvConfigurationDialog::hasChanged" << std::endl;
  return m_changed;
}

void KgvConfigurationDialog::updateSettings()
{
//   std::cerr << "KgvConfigurationDialog::updateSettings" << std::endl;
  m_changed = false;
  switch (m_openingWidget->openInExistingWindowMode->selectedId())
  {
    case 0: // no
      KGraphViewerSettings::setOpenInExistingWindowMode("no");
    break;
    case 1: // yes
      KGraphViewerSettings::setOpenInExistingWindowMode("yes");
    break;
    case 2: // ask
      KGraphViewerSettings::setOpenInExistingWindowMode("ask");
    break;
    default: ;
  }
  switch (m_reloadWidget->reloadOnChangeMode->selectedId())
  {
    case 0: // no
      KGraphViewerSettings::setReloadOnChangeMode("no");
    break;
    case 1: // yes
      KGraphViewerSettings::setReloadOnChangeMode("yes");
    break;
    case 2: // ask
      KGraphViewerSettings::setReloadOnChangeMode("ask");
    break;
    default: ;
  }
  switch (m_reopeningWidget->reopenPreviouslyOpenedFilesMode->selectedId())
  {
    case 0: // no
      KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("no");
    break;
    case 1: // yes
      KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("yes");
    break;
    case 2: // ask
      KGraphViewerSettings::setReopenPreviouslyOpenedFilesMode("ask");
    break;
    default: ;
  }
   KGraphViewerSettings::writeConfig();
}

void KgvConfigurationDialog::updateWidgets()
{
//   std::cerr << "KgvConfigurationDialog::updateWidgets" << std::endl;

  m_changed = false;
//   std::cerr << "  openInExistingWindowMode: " << KGraphViewerSettings::openInExistingWindowMode() << std::endl;
  if (KGraphViewerSettings::openInExistingWindowMode() == "no")
    m_openingWidget->openInExistingWindowMode->setButton(0);
  else if (KGraphViewerSettings::openInExistingWindowMode() == "yes")
    m_openingWidget->openInExistingWindowMode->setButton(1);
  else if (KGraphViewerSettings::openInExistingWindowMode() == "ask")
    m_openingWidget->openInExistingWindowMode->setButton(2);


//   std::cerr << "  reloadOnChangeMode: " << KGraphViewerSettings::reloadOnChangeMode() << std::endl;
  if (KGraphViewerSettings::reloadOnChangeMode() == "no")
    m_reloadWidget->reloadOnChangeMode->setButton(0);
  else if (KGraphViewerSettings::reloadOnChangeMode() == "yes")
    m_reloadWidget->reloadOnChangeMode->setButton(1);
  else if (KGraphViewerSettings::reloadOnChangeMode() == "ask")
    m_reloadWidget->reloadOnChangeMode->setButton(2);

//   std::cerr << "  reopenPreviouslyOpenedFilesMode: " << KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() << std::endl;
  if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "no")
    m_reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(0);
  else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "yes")
    m_reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(1);
  else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "ask")
    m_reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(2);
}
