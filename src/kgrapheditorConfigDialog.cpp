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


#include "kgrapheditorConfigDialog.h"
#include "kgrapheditorsettings.h"
#include "ui_preferencesReload.h"
#include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesReopenPreviouslyOpenedFiles.h"

#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <ktabwidget.h>
#include <kparts/partmanager.h>
#include <kedittoolbar.h>
#include <kdebug.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfigdialog.h>

//#include <kapp.h>
//#include <dcopclient.h>

#include <iostream>

KgeConfigurationDialog::KgeConfigurationDialog (QWidget *parent, const QString& name, KConfigSkeleton *config,
              KPageDialog::FaceType dialogType, 
              ButtonCodes dialogButtons, 
              ButtonCode defaultButton, bool modal) : 
  KConfigDialog (parent, name, config),//, dialogType, dialogButtons, defaultButton, modal) ,
  m_changed(false),
  m_reloadWidget(new Ui::KGraphViewerPreferencesReloadWidget()),
  m_openingWidget(new Ui::KGraphViewerPreferencesOpenInExistingWindowWidget()),
  m_reopeningWidget(new Ui::KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget())
{
  QWidget* page1 = new QWidget();
  m_reloadWidget->setupUi(page1);
  QWidget* page2 = new QWidget();
  m_openingWidget->setupUi(page2);
  QWidget* page3 = new QWidget();
  m_reopeningWidget->setupUi(page3);
  addPage( page1, i18n("Reloading"), "kgraphreloadoptions", i18n("Reloading"), false); 
  addPage( page2, i18n("Opening"), "kgraphopeningoptions", i18n("Opening"), false); 
  addPage( page3, i18n("Session Management"), "kgraphreopeningoptions", i18n("Session Management"), false); 
  connect(m_reloadWidget->reloadOnChangeMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
  connect(m_openingWidget->openInExistingWindowMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
  connect(m_reopeningWidget->reopenPreviouslyOpenedFilesMode, SIGNAL(clicked(int)), this, SLOT(settingChanged(int)));
}

KgeConfigurationDialog::~KgeConfigurationDialog ()
{
}

void KgeConfigurationDialog::settingChanged(int)
{
//   std::cerr << "KgeConfigurationDialog::settingChanged" << std::endl;
  m_changed = true;
}

bool KgeConfigurationDialog::hasChanged()
{
//   std::cerr << "KgeConfigurationDialog::hasChanged" << std::endl;
  return m_changed;
}

void KgeConfigurationDialog::updateSettings()
{
//   std::cerr << "KgeConfigurationDialog::updateSettings" << std::endl;
  m_changed = false;
/*  switch (m_openingWidget->openInExistingWindowMode->selectedId())
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
  }*/
   //@TODO to port
   //KGraphViewerSettings::writeConfig();
}

void KgeConfigurationDialog::updateWidgets()
{
//   std::cerr << "KgeConfigurationDialog::updateWidgets" << std::endl;

  m_changed = false;
//   std::cerr << "  openInExistingWindowMode: " << KGraphViewerSettings::openInExistingWindowMode() << std::endl;
//   if (KGraphViewerSettings::openInExistingWindowMode() == "no")
//     m_openingWidget->openInExistingWindowMode->setButton(0);
//   else if (KGraphViewerSettings::openInExistingWindowMode() == "yes")
//     m_openingWidget->openInExistingWindowMode->setButton(1);
//   else if (KGraphViewerSettings::openInExistingWindowMode() == "ask")
//     m_openingWidget->openInExistingWindowMode->setButton(2);


//   std::cerr << "  reloadOnChangeMode: " << KGraphViewerSettings::reloadOnChangeMode() << std::endl;
//   if (KGraphViewerSettings::reloadOnChangeMode() == "no")
//     m_reloadWidget->reloadOnChangeMode->setButton(0);
//   else if (KGraphViewerSettings::reloadOnChangeMode() == "yes")
//     m_reloadWidget->reloadOnChangeMode->setButton(1);
//   else if (KGraphViewerSettings::reloadOnChangeMode() == "ask")
//     m_reloadWidget->reloadOnChangeMode->setButton(2);

//   std::cerr << "  reopenPreviouslyOpenedFilesMode: " << KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() << std::endl;
/*  if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "no")
    m_reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(0);
  else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "yes")
    m_reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(1);
  else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "ask")
    m_reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(2);*/
}

#include "kgrapheditorConfigDialog.moc"
