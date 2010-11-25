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


#ifndef KGRAPHVIEWERCONFIGDIALOG_H
#define KGRAPHVIEWERCONFIGDIALOG_H

#include <kconfigdialog.h>

namespace Ui {
  class KGraphViewerPreferencesParsingWidget;
  class KGraphViewerPreferencesReloadWidget;
  class KGraphViewerPreferencesOpenInExistingWindowWidget;
  class KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget;
  class KGraphViewerPreferencesAppearanceWidget;
}

/**
 * This is the KGraphViewer configration dialog
 *
 * @short Configuration dialog
 * @author Gaël de Chalendar <kleag@free.fr>
 */
class KgvConfigurationDialog : public KConfigDialog
{
  Q_OBJECT
public:
   KgvConfigurationDialog (QWidget *parent, const QString& name, KConfigSkeleton *config, 
                     KPageDialog::FaceType dialogType,//=IconList, 
                  ButtonCodes dialogButtons=Default|Ok|Apply|Cancel|Help, 
                  ButtonCode defaultButton=Ok, bool modal=false);

  virtual ~KgvConfigurationDialog ();

Q_SIGNALS:
  void backgroundColorChanged(const QColor& color);
  
protected Q_SLOTS:
  virtual void updateSettings();
  virtual void updateWidgets();
  void settingChanged(int);
  void slotBackgroundColorChanged(const QColor& color);

protected:
  virtual bool hasChanged();

  bool m_changed;

public:
  Ui::KGraphViewerPreferencesParsingWidget*  parsingWidget;
  Ui::KGraphViewerPreferencesReloadWidget*  reloadWidget;
  Ui::KGraphViewerPreferencesOpenInExistingWindowWidget* openingWidget;
  Ui::KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget* reopeningWidget;
  Ui::KGraphViewerPreferencesAppearanceWidget* appearanceWidget;
};

#endif
