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


#include "KGraphEditorElementTreeWidget.h"

#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include <klocalizedstring.h>
#include <QLoggingCategory>

static QLoggingCategory debugCategory("org.kde.kgraphviewer");

KGraphEditorElementTreeWidget::KGraphEditorElementTreeWidget(QWidget* parent) :
    QTreeWidget(parent),
    m_popup(0),
    m_item(0)
{
}

KGraphEditorElementTreeWidget::~KGraphEditorElementTreeWidget()
{
}

void KGraphEditorElementTreeWidget::setupPopup(const QPoint& point)
{
  qCDebug(debugCategory) << point;

  if (m_popup != 0)
  {
    delete m_popup;
  }
  m_popup = new QMenu();

  m_item = itemAt(point);
  QAction* aaa = new QAction(i18n("Add a new attribute"), this);
  connect(aaa, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
          this, SLOT(slotAddAttribute()));
  m_popup->addAction(aaa);

  if (m_item != 0) // attribute item
  {
    QAction* raa = new QAction(i18n("Remove this attribute"), this);
    connect(raa, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotRemoveAttribute()));
    m_popup->addAction(raa);
  }
}

void KGraphEditorElementTreeWidget::slotAddAttribute()
{
  QString nodeName = "NewAttribute";
  nodeName += QString::number(topLevelItemCount());
  emit addAttribute(nodeName);
  QTreeWidgetItem* item = new QTreeWidgetItem(this, QStringList(nodeName));
  item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void KGraphEditorElementTreeWidget::slotRemoveAttribute()
{
  qCDebug(debugCategory) << "Remove Attribute";
  if (m_item == 0) // should not happen
  {
    qWarning() << "null item ; should not happen" << endl;
    return;
  }
  emit removeAttribute(m_item->text(0));
  delete takeTopLevelItem (indexOfTopLevelItem(m_item));
  m_item = 0;
}



void KGraphEditorElementTreeWidget::contextMenuEvent ( QContextMenuEvent * e )
{
  setupPopup(e->pos());
  m_popup->exec(e->globalPos());
}

#include "KGraphEditorElementTreeWidget.moc"
