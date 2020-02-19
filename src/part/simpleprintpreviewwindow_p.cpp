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

#include "simpleprintpreviewwindow_p.h"
#include "kgraphviewerlib_debug.h"
#include "simpleprintingengine.h"
#include "simpleprintingsettings.h"

#include <QApplication>
#include <QDebug>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <QStyle>

namespace KGraphViewer
{
KGVSimplePrintPreviewView::KGVSimplePrintPreviewView(KGVSimplePrintPreviewWindow *window)
    : QWidget()
    , m_window(window)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "KGVSimplePrintPreviewView";
    /*  resize(300,400);
      resizeContents(200, 400);*/
    // setAttribute(Qt::WA_PaintOutsidePaintEvent,true);
}

void KGVSimplePrintPreviewView::paintEvent(QPaintEvent *pe)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << pe;
    Q_UNUSED(pe);

    QPainter p(this);
    //   QPainter p(m_window);
    //   p.begin(&pm);
    //   p.initFrom(this);
    //! @todo only for screen!
    qCDebug(KGRAPHVIEWERLIB_LOG) << "filling rect";
    p.fillRect(QRect(QPoint(0, 0), m_window->size()), QBrush(Qt::white)); // pe->rect(), QBrush(white));
    if (m_window->currentPage() >= 0) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "painting page";
        m_window->m_engine.paintPage(m_window->currentPage(), p);
    }
    //    emit m_window->paintingPageRequested(m_window->currentPage(), p);
    p.end();
}

// TODO: redo usages instead with QStyle::PM_Layout{Top,Left,Right,Bottom}Margin
#define KGVSimplePrintPreviewScrollView_MARGIN QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing)

KGVSimplePrintPreviewScrollView::KGVSimplePrintPreviewScrollView(KGVSimplePrintPreviewWindow *window)
    : QScrollArea(window)
    , m_window(window)
{
    //      this->settings = settings;
    m_view = new KGVSimplePrintPreviewView(m_window);

    /*      int widthMM = KgvPageFormat::width(
        settings.pageLayout.format, settings.pageLayout.orientation);
      int heightMM = KgvPageFormat::height(
        settings.pageLayout.format, settings.pageLayout.orientation);
    //      int constantHeight = 400;
    //      m_view->resize(constantHeight * widthMM / heightMM, constantHeight ); //keep aspect
    */
    setWidget(m_view);
}

void KGVSimplePrintPreviewScrollView::paintEvent(QPaintEvent *pe)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << widget();
    QScrollArea::paintEvent(pe);
    ((KGVSimplePrintPreviewView *)widget())->paintEvent(pe);
}

// void KGVSimplePrintPreviewScrollView::resizeEvent( QResizeEvent *re )
// {
//   QScrollArea::resizeEvent(re);
// //  qCDebug(KGRAPHVIEWERLIB_LOG) << re->size().width() << " " << re->size().height();
// //  qCDebug(KGRAPHVIEWERLIB_LOG) << contentsWidth() << " " << contentsHeight();
// //  qCDebug(KGRAPHVIEWERLIB_LOG) << m_view->width() << " " << m_view->height();
//   setUpdatesEnabled(false);
//   if (re->size().width() > (m_view->width()+2*KGVSimplePrintPreviewScrollView_MARGIN)
//     || re->size().height() > (m_view->height()+2*KGVSimplePrintPreviewScrollView_MARGIN))
//   {
//     resizeContents(
//         qMax(re->size().width(), m_view->width()+2*KGVSimplePrintPreviewScrollView_MARGIN),
//         qMax(re->size().height(), m_view->height()+2*KGVSimplePrintPreviewScrollView_MARGIN));
//
//     int vscrbarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
//     moveChild(m_view, (contentsWidth() - vscrbarWidth - m_view->width())/2,
//       (contentsHeight() - m_view->height())/2);
//   }
//   setUpdatesEnabled(true);
// }

void KGVSimplePrintPreviewScrollView::setFullWidth()
{
    viewport()->setUpdatesEnabled(false);
    double widthMM = KgvPageFormat::width(m_window->settings()->pageLayout.format, m_window->settings()->pageLayout.orientation);
    double heightMM = KgvPageFormat::height(m_window->settings()->pageLayout.format, m_window->settings()->pageLayout.orientation);
    //  int constantWidth = m_window->width()- KGVSimplePrintPreviewScrollView_MARGIN*6;
    double constantWidth = width() - KGVSimplePrintPreviewScrollView_MARGIN * 6;
    double heightForWidth = constantWidth * heightMM / widthMM;
    //  heightForWidth = qMin(kapp->desktop()->height()*4/5, heightForWidth);
    constantWidth = heightForWidth * widthMM / heightMM;
    m_view->resize((int)constantWidth, (int)heightForWidth); // keep aspect
    /*  resizeContents(int(m_view->width() + 2*KGVSimplePrintPreviewScrollView_MARGIN),
        int(m_view->height() + 2*KGVSimplePrintPreviewScrollView_MARGIN));*/
    /*  moveChild(m_view, (contentsWidth()-m_view->width())/2,
        (contentsHeight()-m_view->height())/2);*/
    viewport()->setUpdatesEnabled(true);
    resize(size() + QSize(1, 1)); // to update pos.
    //   m_view->enablePainting = true;
    m_view->repaint();
}

// void KGVSimplePrintPreviewScrollView::setContentsPos(int x, int y)
// {
// //  qCDebug(KGRAPHVIEWERLIB_LOG) << "############" << x << " " << y << " " << contentsX()<< " " <<contentsY();
//   if (x<0 || y<0) //to avoid endless loop on Linux
//     return;
//   QScrollArea::setContentsPos(x,y);
// }

}
