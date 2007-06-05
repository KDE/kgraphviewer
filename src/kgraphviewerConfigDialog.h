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


#ifndef _KGRAPHVIEWERCONFIGDIALOG_H_
#define _KGRAPHVIEWERCONFIGDIALOG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfigdialog.h>

class KGraphViewerPreferencesReloadWidget;
class KGraphViewerPreferencesOpenInExistingWindowWidget;
class KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget;

/**
 * This is the KGraphViewer configuration dialog
 *
 * @short Configuration dialog
 * @author Gaël de Chalendar <kleag@free.fr>
 */
class KgvConfigurationDialog : public KConfigDialog
{
  Q_OBJECT
public:
   KgvConfigurationDialog (QWidget *parent, const char *name, KConfigSkeleton *config, 
                  DialogType dialogType=IconList, 
                  int dialogButtons=Default|Ok|Apply|Cancel|Help, 
                  ButtonCode defaultButton=Ok, bool modal=false);

  virtual ~KgvConfigurationDialog ();

protected slots:
  virtual void updateSettings();
  virtual void updateWidgets();
  void settingChanged(int);

protected:
  virtual bool hasChanged();

  bool m_changed;
  KGraphViewerPreferencesReloadWidget*  m_reloadWidget;
  KGraphViewerPreferencesOpenInExistingWindowWidget* m_openingWidget;
  KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget* m_reopeningWidget;
};

#endif
