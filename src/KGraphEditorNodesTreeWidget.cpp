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

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>

#include <QMenu>
#include <QContextMenuEvent>

KGraphEditorNodesTreeWidget::KGraphEditorNodesTreeWidget(QWidget* parent) :
    QTreeWidget(parent),
    m_popup(0)
{
}

KGraphEditorNodesTreeWidget::~KGraphEditorNodesTreeWidget()
{
}

void KGraphEditorNodesTreeWidget::setupPopup(const QPoint& point)
{
  kDebug() << point;

  if (m_popup != 0)
  {
    delete m_popup;
  }
  m_popup = new QMenu();

  QTreeWidgetItem* item = itemAt(point);
  if (item == 0)
  {
    kDebug() << "no item at" << point;
    return;
  }
  KAction* aaa = new KAction(i18n("Add a new attribute"), this);
  connect(aaa, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          this, SLOT(slotAddAttribute()));
  m_popup->addAction(aaa);

  if (item->parent() != 0) // attribute item
  {
    KAction* raa = new KAction(i18n("Remove this attribute"), this);
    connect(raa, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
            this, SLOT(slotRemoveAttribute()));
    m_popup->addAction(raa);
  }
  m_popup->addSeparator();
  KAction* rna = new KAction(i18n("Remove this node"), this);
  connect(rna, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          this, SLOT(slotRemoveNode()));
  m_popup->addAction(rna);  
}

void KGraphEditorNodesTreeWidget::slotRemoveNode()
{
  kDebug() << "Remove Node";
}

void KGraphEditorNodesTreeWidget::slotAddAttribute()
{
  kDebug() << "Add Attribute";
}

void KGraphEditorNodesTreeWidget::slotRemoveAttribute()
{
  kDebug() << "Remove Attribute";
}



void KGraphEditorNodesTreeWidget::contextMenuEvent ( QContextMenuEvent * e )
{
  setupPopup(e->pos());
  m_popup->exec(e->globalPos());
}


#include "KGraphEditorNodesTreeWidget.moc"
