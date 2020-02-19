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

#ifndef _KGRAPHEDITORNODESTREEWIDGET_H_
#define _KGRAPHEDITORNODESTREEWIDGET_H_

#include <QTreeWidget>

/**
 * This is the widget used to list the nodes and and edit their attributes
 *
 * @short Nodes attributes editing widget
 * @author Gael de Chalendar <kleag@free.fr>
 */
class KGraphEditorNodesTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    explicit KGraphEditorNodesTreeWidget(QWidget *parent = nullptr);

    /**
     * Default Destructor
     */
    ~KGraphEditorNodesTreeWidget() override;

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;

protected Q_SLOTS:
    void slotRemoveNode();
    void slotAddAttribute();
    void slotRemoveAttribute();

public Q_SLOTS:
    void slotRemoveElement(const QString &id);

Q_SIGNALS:
    void removeNode(const QString &);
    void addAttribute(const QString &);
    void removeAttribute(const QString &, const QString &);

private:
    void setupPopup(const QPoint &point);

    QMenu *m_popup;
    QTreeWidgetItem *m_item;
};

#endif // _KGRAPHEDITORNODESTREEWIDGET_H_
