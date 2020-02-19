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
#include "kgrapheditor_debug.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDebug>
#include <QMenu>
#include <klocalizedstring.h>

KGraphEditorElementTreeWidget::KGraphEditorElementTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_popup(nullptr)
    , m_item(nullptr)
{
}

KGraphEditorElementTreeWidget::~KGraphEditorElementTreeWidget()
{
}

void KGraphEditorElementTreeWidget::setupPopup(const QPoint &point)
{
    qCDebug(KGRAPHEDITOR_LOG) << point;

    if (m_popup) {
        delete m_popup;
    }
    m_popup = new QMenu();

    m_item = itemAt(point);
    QAction *aaa = new QAction(i18n("Add a new attribute"), this);
    connect(aaa, &QAction::triggered, this, &KGraphEditorElementTreeWidget::slotAddAttribute);
    m_popup->addAction(aaa);

    if (m_item) // attribute item
    {
        QAction *raa = new QAction(i18n("Remove this attribute"), this);
        connect(raa, &QAction::triggered, this, &KGraphEditorElementTreeWidget::slotRemoveAttribute);
        m_popup->addAction(raa);
    }
}

void KGraphEditorElementTreeWidget::slotAddAttribute()
{
    QString nodeName = "NewAttribute";
    nodeName += QString::number(topLevelItemCount());
    emit addAttribute(nodeName);
    QTreeWidgetItem *item = new QTreeWidgetItem(this, QStringList(nodeName));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void KGraphEditorElementTreeWidget::slotRemoveAttribute()
{
    qCDebug(KGRAPHEDITOR_LOG) << "Remove Attribute";
    if (m_item == nullptr) // should not happen
    {
        qCWarning(KGRAPHEDITOR_LOG) << "null item ; should not happen" << endl;
        return;
    }
    emit removeAttribute(m_item->text(0));
    delete takeTopLevelItem(indexOfTopLevelItem(m_item));
    m_item = nullptr;
}

void KGraphEditorElementTreeWidget::contextMenuEvent(QContextMenuEvent *e)
{
    setupPopup(e->pos());
    m_popup->exec(e->globalPos());
}
