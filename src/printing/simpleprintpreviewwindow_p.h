/* This file is part of KGraphViewer.
   Copyright (C) 2005-2006 Gaël de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

class KGVSimplePrintPreviewView : public QWidget
{
	public:
		KGVSimplePrintPreviewView(QWidget *parent, KGVSimplePrintPreviewWindow *window);

		virtual void paintEvent( QPaintEvent *pe );

		bool enablePainting;
	protected:
		KGVSimplePrintPreviewWindow *m_window;
};

class KGVSimplePrintPreviewScrollView : public QScrollView
{
	Q_OBJECT

	public:
		KGVSimplePrintPreviewScrollView(KGVSimplePrintPreviewWindow *window);

		KGVSimplePrintPreviewView *widget;

	public slots:
		void setFullWidth();
		void setContentsPos(int x, int y);

	protected:
		virtual void resizeEvent( QResizeEvent *re );
		KGVSimplePrintPreviewWindow *m_window;
};

#endif
