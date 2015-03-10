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


#include "KGraphEditorNodesTreeWidget.h"

#include <QDebug>
#include <QAction>

#include <QMenu>
#include <QContextMenuEvent>
#include <QLoggingCategory>
#include <klocalizedstring.h>

static QLoggingCategory debugCategory("org.kde.kgraphviewer");

KGraphEditorNodesTreeWidget::KGraphEditorNodesTreeWidget(QWidget* parent) :
    QTreeWidget(parent),
    m_popup(0),
    m_item(0)
{
}

KGraphEditorNodesTreeWidget::~KGraphEditorNodesTreeWidget()
{
}

void KGraphEditorNodesTreeWidget::setupPopup(const QPoint& point)
{
  qCDebug(debugCategory) << point;

  if (m_popup != 0)
  {
    delete m_popup;
  }
  m_popup = new QMenu();

  m_item = itemAt(point);
  if (m_item == 0)
  {
    qCDebug(debugCategory) << "no item at" << point;
    return;
  }
  QAction* aaa = new QAction(i18n("Add a new attribute"), this);
  connect(aaa, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
          this, SLOT(slotAddAttribute()));
  m_popup->addAction(aaa);

  if (m_item->parent() != 0) // attribute item
  {
    QAction* raa = new QAction(i18n("Remove this attribute"), this);
    connect(raa, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
            this, SLOT(slotRemoveAttribute()));
    m_popup->addAction(raa);
  }
  m_popup->addSeparator();
  QAction* rna = new QAction(i18n("Remove this node"), this);
  connect(rna, SIGNAL(triggered(Qt::MouseButtons,Qt::KeyboardModifiers)),
          this, SLOT(slotRemoveNode()));
  m_popup->addAction(rna);  
}

void KGraphEditorNodesTreeWidget::slotRemoveNode()
{
  emit removeNode(m_item->text(0));
  delete takeTopLevelItem (indexOfTopLevelItem(m_item));
  m_item = 0;
}

void KGraphEditorNodesTreeWidget::slotRemoveElement(const QString& id)
{
  qCDebug(debugCategory) << id;
  QList<QTreeWidgetItem*> items = findItems(id,Qt::MatchExactly,0);
  foreach (QTreeWidgetItem* item, items)
  {
    delete takeTopLevelItem (indexOfTopLevelItem(item));
  }
}

void KGraphEditorNodesTreeWidget::slotAddAttribute()
{
  qCDebug(debugCategory) << "Add Attribute";
  QString nodeName = "NewAttribute";
  emit addAttribute(m_item->text(0));
  if (m_item->parent() == 0)
  {
    nodeName += QString::number(m_item->childCount());
    QTreeWidgetItem* item = new QTreeWidgetItem(m_item, QStringList(nodeName));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }
  else
  {
    nodeName += QString::number(m_item->parent()->childCount());
    QTreeWidgetItem* item = new QTreeWidgetItem(m_item->parent(), QStringList(nodeName));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
  }
 }

void KGraphEditorNodesTreeWidget::slotRemoveAttribute()
{
  qCDebug(debugCategory) << "Remove Attribute";
  emit removeAttribute(m_item->parent()->text(0), m_item->text(0));
  m_item->parent()->removeChild(m_item);
  m_item = 0;
}



void KGraphEditorNodesTreeWidget::contextMenuEvent ( QContextMenuEvent * e )
{
  setupPopup(e->pos());
  m_popup->exec(e->globalPos());
}

#include "KGraphEditorNodesTreeWidget.moc"
