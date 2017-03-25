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


#ifndef KGRAPHEDITORCONFIGDIALOG_H
#define KGRAPHEDITORCONFIGDIALOG_H

#include <kconfigdialog.h>
#include <KConfigSkeleton>

namespace Ui {
class KGraphViewerPreferencesParsingWidget;
class KGraphViewerPreferencesReloadWidget;
class KGraphViewerPreferencesOpenInExistingWindowWidget;
class KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget;
class KGraphViewerPreferencesAppearanceWidget;
}

/**
 * This is the KGraphEditor configuration dialog
 *
 * @short Configuration dialog
 * @author Gael de Chalendar <kleag@free.fr>
 */
class KgeConfigurationDialog : public KConfigDialog
{
  Q_OBJECT
public:
   KgeConfigurationDialog (QWidget *parent, const QString& name, KConfigSkeleton *config);

  virtual ~KgeConfigurationDialog ();

protected slots:
  virtual void updateSettings();
  virtual void updateWidgets();
  void settingChanged(int);

protected:
  virtual bool hasChanged();

  bool m_changed;

public:
  Ui::KGraphViewerPreferencesParsingWidget*  m_parsingWidget;
  Ui::KGraphViewerPreferencesReloadWidget*  m_reloadWidget;
  Ui::KGraphViewerPreferencesOpenInExistingWindowWidget* m_openingWidget;
  Ui::KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget* m_reopeningWidget;
  Ui::KGraphViewerPreferencesAppearanceWidget* m_appearanceWidget;
};

#endif
