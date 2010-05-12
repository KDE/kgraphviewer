/* This file is part of KGraphViewer.
   Copyright (C) 2007 Gael de Chalendar <kleag@free.fr>

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


#ifndef _KGRAPHEDITORELEMENTTREEWIDGET_H_
#define _KGRAPHEDITORELEMENTTREEWIDGET_H_

#include <QTreeWidget>

/**
 * This is the widget used to list the attributes of an element and edit them
 *
 * @short Element attributes editing widget
 * @author Gael de Chalendar <kleag@free.fr>
 */
class KGraphEditorElementTreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  /**
    * Default Constructor
    */
  explicit KGraphEditorElementTreeWidget(QWidget* parent = 0);

  /**
    * Default Destructor
    */
  virtual ~KGraphEditorElementTreeWidget();

protected:

  virtual void contextMenuEvent ( QContextMenuEvent * e );

protected Q_SLOTS:
  void slotAddAttribute();
  void slotRemoveAttribute();

Q_SIGNALS:
  void addAttribute(const QString&);
  void removeAttribute(const QString&);

private:
  void setupPopup(const QPoint& point);

  QMenu* m_popup;
  QTreeWidgetItem* m_item;
};


#endif // _KGRAPHEDITORELEMENTTREEWIDGET_H_
