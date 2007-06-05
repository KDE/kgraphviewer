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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef _KGRAPHVIEWERCONFIGDIALOG_H_
#define _KGRAPHVIEWERCONFIGDIALOG_H_

#include <kconfigdialog.h>

namespace Ui {
class KGraphViewerPreferencesReloadWidget;
class KGraphViewerPreferencesOpenInExistingWindowWidget;
class KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget;
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

protected slots:
  virtual void updateSettings();
  virtual void updateWidgets();
  void settingChanged(int);

protected:
  virtual bool hasChanged();

  bool m_changed;
  Ui::KGraphViewerPreferencesReloadWidget*  m_reloadWidget;
  Ui::KGraphViewerPreferencesOpenInExistingWindowWidget* m_openingWidget;
  Ui::KGraphViewerPreferencesReopenPreviouslyOpenedFilesWidget* m_reopeningWidget;
};

#endif
