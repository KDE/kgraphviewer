/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gaël de Chalendar <kleag@free.fr>

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

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <qtooltip.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qwhatsthis.h>
#include <qcanvas.h>
#include <qwmatrix.h>
#include <qpair.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstyle.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktempfile.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>

#include "pannerview.h"

//
// PannerView
//
PannerView::PannerView(QWidget * parent, const char * name)
  : QCanvasView(parent, name), m_drawContents(true)
{
  _movingZoomRect = false;

  // why doesn't this avoid flicker ?
  // viewport()->setBackgroundMode(Qt::NoBackground);
  setBackgroundMode(Qt::NoBackground);

  QToolTip::add(this, i18n("View of The complete graph. Click and drag to move this visible part."));
  QWhatsThis::add(this, i18n("<h1>View of The complete graph</h1>"
    "<p>Single click out of the red square to move the center of his one at the "
    "mouse position.</p><p>Click in the red square and drag the mouse and the "
    "red square will follow the movement.</p>"));
}

void PannerView::setZoomRect(QRect r)
{
  QRect oldRect = _zoomRect;
  _zoomRect = r;
  if (!_zoomRect.isValid() || _zoomRect.width() < 15 || _zoomRect.height() < 15) 
  {
    double factor = ((double)_zoomRect.width())/_zoomRect.height();
    uint newWidth, newHeight;
    if (factor > 1.0)
    {
      newWidth = 15;
      newHeight = (uint)ceil(newWidth/factor);
      if (newHeight < 10) newHeight = 10;
    }
    else
    {
      newHeight = 15;
      newWidth = (uint)ceil(newHeight*factor);
      if (newWidth < 10) newWidth = 10;
    }
    QRect newRect = _zoomRect;
    int newXPos = newRect.x() + (newRect.width()/2) - newWidth/2;
    newXPos = (newXPos<0)?0:newXPos;
    newRect.setX(newXPos);
    int newYPos = newRect.y() + (newRect.height()/2) -newHeight/2;
    newYPos = (newYPos<0)?0:newYPos;
    newRect.setY(newYPos);
    newRect.setWidth(newWidth);
    newRect.setHeight(newHeight);
    _zoomRect = newRect;
  }
  updateContents(oldRect);
  updateContents(_zoomRect);
}

void PannerView::drawContents(QPainter * p, int clipx, int clipy, int clipw, int cliph)
{
  // save/restore around QCanvasView::drawContents seems to be needed
  // for QT 3.0 to get the red rectangle drawn correctly
  if (m_drawContents)
  {
    p->save();
    QCanvasView::drawContents(p,clipx,clipy,clipw,cliph);
    p->restore();
  }
  if (_zoomRect.isValid()) 
  {
    p->save();
    p->setPen(red);
    if (_zoomRect.width() > 10 && _zoomRect.height() > 10)
    {
      p->drawRect(_zoomRect);
    }
    else
    {
      QBrush brush(red);
      p->fillRect(_zoomRect.x(), _zoomRect.y(), _zoomRect.width(), _zoomRect.height(), brush);
    }
    p->restore();
  }
}

void PannerView::contentsMousePressEvent(QMouseEvent* e)
{
  if (_zoomRect.isValid()) {
    if (!_zoomRect.contains(e->pos()))
      emit zoomRectMoved(e->pos().x() - _zoomRect.center().x(),
                         e->pos().y() - _zoomRect.center().y());

    _movingZoomRect = true;
    _lastPos = e->pos();
  }
}

void PannerView::contentsMouseMoveEvent(QMouseEvent* e)
{
  if (_movingZoomRect) {
    emit zoomRectMoved(e->pos().x() - _lastPos.x(), e->pos().y() - _lastPos.y());
    _lastPos = e->pos();
  }
}

void PannerView::contentsMouseReleaseEvent(QMouseEvent*)
{
    _movingZoomRect = false;
    emit zoomRectMoveFinished();
}

