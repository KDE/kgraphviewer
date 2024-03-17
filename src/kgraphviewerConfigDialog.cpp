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

#include "kgraphviewerConfigDialog.h"
#include "kgraphviewer_debug.h"
#include "kgraphviewersettings.h"
#include "ui_preferencesAppearance.h"
#include "ui_preferencesOpenInExistingWindow.h"
#include "ui_preferencesParsing.h"
#include "ui_preferencesReload.h"
#include "ui_preferencesReopenPreviouslyOpenedFiles.h"

#include <QDebug>
#include <QFileDialog>
#include <QTabWidget>
#include <QUrl>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kparts/partmanager.h>

#include <KPluginFactory>
#include <QMessageBox>
#include <QStatusBar>
#include <kconfigdialog.h>
#include <klocalizedstring.h>

//#include <kapp.h>
//#include <dcopclient.h>

#include <QDialogButtonBox>
#include <iostream>

KgvConfigurationDialog::KgvConfigurationDialog(QWidget *parent, const QString &name, KConfigSkeleton *config)
    : KConfigDialog(parent, name, config)
    , m_changed(false)
    , parsingWidget(new Ui::KGraphViewerPreferencesParsingWidget())
    , reloadWidget(new Ui::KGraphViewerPreferencesReloadWidget())
    , openingWidget(new Ui::KGraphViewerPreferencesOpenInExistingWindowWidget())
    , reopeningWidget(new Ui::KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget())
    , appearanceWidget(new Ui::KGraphViewerPreferencesAppearanceWidget())
{
    QWidget *page4 = new QWidget();
    appearanceWidget->setupUi(page4);
    QWidget *page0 = new QWidget();
    parsingWidget->setupUi(page0);
    QWidget *page1 = new QWidget();
    reloadWidget->setupUi(page1);
    QWidget *page2 = new QWidget();
    openingWidget->setupUi(page2);
    QWidget *page3 = new QWidget();
    reopeningWidget->setupUi(page3);

    appearanceWidget->kcolorbutton->setColor(KGraphViewerSettings::backgroundColor());
    appearanceWidget->kcolorbutton->setDefaultColor(KGraphViewerSettings::backgroundColor());
    addPage(page4, i18n("Appearance"), "preferences-other", i18n("Appearance"), false);
    addPage(page0, i18n("Parsing"), "preferences-other", i18n("Parsing"), false);
    addPage(page1, i18n("Reloading"), "view-refresh", i18n("Reloading"), false);
    addPage(page2, i18n("Opening"), "document-open", i18n("Opening"), false);
    addPage(page3, i18n("Session Management"), "preferences-other", i18n("Session Management"), false);
    connect(parsingWidget->parsingMode, &QGroupBox::clicked, this, &KgvConfigurationDialog::settingChanged);
    connect(reloadWidget->reloadOnChangeMode, &QGroupBox::clicked, this, &KgvConfigurationDialog::settingChanged);
    connect(openingWidget->openInExistingWindowMode, &QGroupBox::clicked, this, &KgvConfigurationDialog::settingChanged);
    connect(reopeningWidget->reopenPreviouslyOpenedFilesMode, &QGroupBox::clicked, this, &KgvConfigurationDialog::settingChanged);
    connect(appearanceWidget->kcolorbutton, &KColorButton::changed, this, &KgvConfigurationDialog::slotBackgroundColorChanged);
}

KgvConfigurationDialog::~KgvConfigurationDialog()
{
}

void KgvConfigurationDialog::settingChanged(int)
{
    //   std::cerr << "KgvConfigurationDialog::settingChanged" << std::endl;
    m_changed = true;
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
    /*  switch (openingWidget->openInExistingWindowMode->selectedId())
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
      switch (reloadWidget->reloadOnChangeMode->selectedId())
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
      switch (reopeningWidget->reopenPreviouslyOpenedFilesMode->selectedId())
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
    // KGraphViewerSettings::writeConfig();
}

void KgvConfigurationDialog::updateWidgets()
{
    m_changed = false;
    qCDebug(KGRAPHVIEWER_LOG) << "  openInExistingWindowMode: " << KGraphViewerSettings::openInExistingWindowMode();
    //   if (KGraphViewerSettings::openInExistingWindowMode() == "no")
    //     openingWidget->openInExistingWindowMode->setButton(0);
    //   else if (KGraphViewerSettings::openInExistingWindowMode() == "yes")
    //     openingWidget->openInExistingWindowMode->setButton(1);
    //   else if (KGraphViewerSettings::openInExistingWindowMode() == "ask")
    //     openingWidget->openInExistingWindowMode->setButton(2);

    //   std::cerr << "  reloadOnChangeMode: " << KGraphViewerSettings::reloadOnChangeMode() << std::endl;
    //   if (KGraphViewerSettings::reloadOnChangeMode() == "no")
    //     reloadWidget->reloadOnChangeMode->setButton(0);
    //   else if (KGraphViewerSettings::reloadOnChangeMode() == "yes")
    //     reloadWidget->reloadOnChangeMode->setButton(1);
    //   else if (KGraphViewerSettings::reloadOnChangeMode() == "ask")
    //     reloadWidget->reloadOnChangeMode->setButton(2);

    //   std::cerr << "  reopenPreviouslyOpenedFilesMode: " << KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() << std::endl;
    /*  if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "no")
        reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(0);
      else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "yes")
        reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(1);
      else if (KGraphViewerSettings::reopenPreviouslyOpenedFilesMode() == "ask")
        reopeningWidget->reopenPreviouslyOpenedFilesMode->setButton(2);*/
}

void KgvConfigurationDialog::slotBackgroundColorChanged(const QColor &color)
{
    KGraphViewerSettings::setBackgroundColor(color);
    emit backgroundColorChanged(color);
}

#include "moc_kgraphviewerConfigDialog.cpp"
