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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/* This file was part of the KDE project
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 */

#ifndef KGVSIMPLEPRINTPREVIEWWINDOW_P_H
#define KGVSIMPLEPRINTPREVIEWWINDOW_P_H

#include "simpleprintpreviewwindow.h"
//Added by qt3to4:
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollArea>

class KGVSimplePrintPreviewView : public QWidget
{
	public:
		KGVSimplePrintPreviewView(KGVSimplePrintPreviewWindow *window);

		virtual void paintEvent( QPaintEvent *pe );
	protected:
		KGVSimplePrintPreviewWindow *m_window;
};

class KGVSimplePrintPreviewScrollView : public QScrollArea
{
	Q_OBJECT

	public:
		KGVSimplePrintPreviewScrollView(KGVSimplePrintPreviewWindow *window);

		KGVSimplePrintPreviewView *m_view;

	public slots:
		void setFullWidth();
// 		void setContentsPos(int x, int y);

	protected:
        void paintEvent( QPaintEvent *pe );
   
// 		virtual void resizeEvent( QResizeEvent *re );
		KGVSimplePrintPreviewWindow *m_window;
};

#endif
