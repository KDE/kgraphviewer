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
#include "kgrapheditor_debug.h"

#include <QAction>
#include <QDebug>

#include <QContextMenuEvent>
#include <QMenu>
#include <klocalizedstring.h>

KGraphEditorNodesTreeWidget::KGraphEditorNodesTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_popup(nullptr)
    , m_item(nullptr)
{
}

KGraphEditorNodesTreeWidget::~KGraphEditorNodesTreeWidget()
{
}

void KGraphEditorNodesTreeWidget::setupPopup(const QPoint &point)
{
    qCDebug(KGRAPHEDITOR_LOG) << point;

    if (m_popup) {
        delete m_popup;
    }
    m_popup = new QMenu();

    m_item = itemAt(point);
    if (m_item == nullptr) {
        qCDebug(KGRAPHEDITOR_LOG) << "no item at" << point;
        return;
    }
    QAction *aaa = new QAction(i18n("Add a new attribute"), this);
    connect(aaa, &QAction::triggered, this, &KGraphEditorNodesTreeWidget::slotAddAttribute);
    m_popup->addAction(aaa);

    if (m_item->parent()) // attribute item
    {
        QAction *raa = new QAction(i18n("Remove this attribute"), this);
        connect(raa, &QAction::triggered, this, &KGraphEditorNodesTreeWidget::slotRemoveAttribute);
        m_popup->addAction(raa);
    }
    m_popup->addSeparator();
    QAction *rna = new QAction(i18n("Remove this node"), this);
    connect(rna, &QAction::triggered, this, &KGraphEditorNodesTreeWidget::slotRemoveNode);
    m_popup->addAction(rna);
}

void KGraphEditorNodesTreeWidget::slotRemoveNode()
{
    emit removeNode(m_item->text(0));
    delete takeTopLevelItem(indexOfTopLevelItem(m_item));
    m_item = nullptr;
}

void KGraphEditorNodesTreeWidget::slotRemoveElement(const QString &id)
{
    qCDebug(KGRAPHEDITOR_LOG) << id;
    QList<QTreeWidgetItem *> items = findItems(id, Qt::MatchExactly, 0);
    foreach (QTreeWidgetItem *item, items) {
        delete takeTopLevelItem(indexOfTopLevelItem(item));
    }
}

void KGraphEditorNodesTreeWidget::slotAddAttribute()
{
    qCDebug(KGRAPHEDITOR_LOG) << "Add Attribute";
    QString nodeName = "NewAttribute";
    emit addAttribute(m_item->text(0));
    if (m_item->parent() == nullptr) {
        nodeName += QString::number(m_item->childCount());
        QTreeWidgetItem *item = new QTreeWidgetItem(m_item, QStringList(nodeName));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    } else {
        nodeName += QString::number(m_item->parent()->childCount());
        QTreeWidgetItem *item = new QTreeWidgetItem(m_item->parent(), QStringList(nodeName));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

void KGraphEditorNodesTreeWidget::slotRemoveAttribute()
{
    qCDebug(KGRAPHEDITOR_LOG) << "Remove Attribute";
    emit removeAttribute(m_item->parent()->text(0), m_item->text(0));
    m_item->parent()->removeChild(m_item);
    m_item = nullptr;
}

void KGraphEditorNodesTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    setupPopup(e->pos());
    m_popup->exec(e->globalPos());
}
