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
  setupPopup();
}

KGraphEditorNodesTreeWidget::~KGraphEditorNodesTreeWidget()
{
}

void KGraphEditorNodesTreeWidget::setupPopup()
{
  kDebug();

  if (m_popup != 0)
  {
    return;
  }
  m_popup = new QMenu();

  KAction* bba = new KAction(i18n("Automatic"), this);
  connect(bba, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          this, SLOT(slotGloup()));
  m_popup->addAction(bba);
}

void KGraphEditorNodesTreeWidget::slotGloup()
{
  kDebug() << "AAAAAAAAAAAAAAaaaaaaahhhhhhhhhhh!";
}



void KGraphEditorNodesTreeWidget::contextMenuEvent ( QContextMenuEvent * e )
{
  m_popup->exec(e->globalPos());
}


#include "KGraphEditorNodesTreeWidget.moc"
