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

/* This file was part of the KDE project
   Copyright (C) 2005 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#ifndef KGVSIMPLEPRINTPREVIEWWINDOW_P_H
#define KGVSIMPLEPRINTPREVIEWWINDOW_P_H

#include "simpleprintpreviewwindow.h"
// Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollArea>

namespace KGraphViewer
{
class KGVSimplePrintPreviewView : public QWidget
{
    Q_OBJECT
public:
    explicit KGVSimplePrintPreviewView(KGVSimplePrintPreviewWindow *window);

    void paintEvent(QPaintEvent *pe) override;

protected:
    KGVSimplePrintPreviewWindow *m_window;
};

class KGVSimplePrintPreviewScrollView : public QScrollArea
{
    Q_OBJECT

public:
    explicit KGVSimplePrintPreviewScrollView(KGVSimplePrintPreviewWindow *window);

    KGVSimplePrintPreviewView *m_view;

public Q_SLOTS:
    void setFullWidth();
    // 		void setContentsPos(int x, int y);

protected:
    void paintEvent(QPaintEvent *pe) override;

    // 		virtual void resizeEvent( QResizeEvent *re );
    KGVSimplePrintPreviewWindow *m_window;
};

}

#endif
