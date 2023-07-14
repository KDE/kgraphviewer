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

#include "simpleprintingengine.h"
#include "kgraphviewerlib_debug.h"
#include "simpleprintingsettings.h"

#include <QApplication>
#include <QDebug>
#include <QFontDialog>
#include <QIcon>
#include <kconfig.h>
#include <kurllabel.h>

#include <qcheckbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>

#include <QDateTime>
#include <QGraphicsScene>
#include <QPaintDevice>
#include <QPixmap>
#include <klocalizedstring.h>
#include <math.h>

namespace KGraphViewer
{
KGVSimplePrintingEngine::KGVSimplePrintingEngine(KGVSimplePrintingSettings *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_data(nullptr)
{
    setObjectName("KGVSimplePrintingEngine");
    clear();
}

KGVSimplePrintingEngine::~KGVSimplePrintingEngine()
{
    done();
}

bool KGVSimplePrintingEngine::init(DotGraphView &data, const QString &titleText, QString &errorMessage)
{
    errorMessage.clear();
    done();
    m_headerText = titleText;

    m_data = &data;
    m_eof = false;

    m_painting = QPixmap(m_data->scene()->sceneRect().size().toSize());
    QPainter p(&m_painting);
    m_data->scene()->render(&p);

    return true;
}

bool KGVSimplePrintingEngine::done()
{
    bool result = true;
    m_data = nullptr;
    m_pagesCount = 0;
    m_paintInitialized = false;
    return result;
}

void KGVSimplePrintingEngine::clear()
{
    m_eof = false;
    m_pagesCount = 0;
    m_paintInitialized = false;
}

void KGVSimplePrintingEngine::paintPage(int pageNumber, QPainter &painter, bool paint)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << pageNumber << "/" << m_pagesCount << paint;

    uint y = 0;

    if (pageNumber <= m_pagesCount) {
        m_eof = false;
    }
    int w = 0, h = 0;
    QPaintDevice *pdm = painter.device();
    const bool printer = pdm->devType() == QInternal::Printer;
    qCDebug(KGRAPHVIEWERLIB_LOG) << "printer:" << printer;

    if (!printer) {
        w = pdm->width();
        h = pdm->height();
    } else { // QPrinter...
        w = pdm->widthMM();
        h = pdm->heightMM();
    }

    if (!m_paintInitialized) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "initializing";
        // HACK: some functions here do not work properly if were
        // are not in a paint event, so repeat this until we actually paint.
        m_paintInitialized = paint;

        double widthMM = KgvPageFormat::width(m_settings->pageLayout.format, m_settings->pageLayout.orientation);
        double heightMM = KgvPageFormat::height(m_settings->pageLayout.format, m_settings->pageLayout.orientation);

        m_dpiY = pdm->logicalDpiY();
        m_dpiX = pdm->logicalDpiX();
#ifdef Q_WS_WIN // fix for 120dpi
        if (!printer) {
            //			m_dpiY = 96;
            //			m_dpiX = 96;
            m_dpiY = 86;
            m_dpiX = 86;
        }
#endif
        int pdWidthMM = pdm->widthMM();
        int pdHeightMM = pdm->heightMM();

        double screenF;
        //		if (printer)
        screenF = 1.0;
        //		else
        //			screenF = (double)96.0/120.0;

        leftMargin = POINT_TO_INCH(m_settings->pageLayout.ptLeft) * m_dpiX * screenF;
        rightMargin = POINT_TO_INCH(m_settings->pageLayout.ptRight) * m_dpiX * screenF;
        topMargin = POINT_TO_INCH(m_settings->pageLayout.ptTop) * m_dpiY * screenF;
        bottomMargin = POINT_TO_INCH(m_settings->pageLayout.ptBottom) * m_dpiY * screenF;

        m_fx = widthMM / (pdWidthMM * screenF);
        m_fy = heightMM / (pdHeightMM * screenF);

        // screen only
        //	painter.fillRect(QRect(0,0,w,h), QBrush(white));
        m_pageWidth = int(m_fx * (double)pdm->width() - leftMargin - rightMargin);
        m_pageHeight = int(m_fy * (double)pdm->height() - topMargin - bottomMargin);

        //! @todo add setting
        m_mainFont = m_settings->pageTitleFont;
        if (!printer) {
            int pixelSize = int(POINT_TO_INCH(m_mainFont.pointSizeF()) * m_dpiX);
            m_mainFont.setPixelSize(pixelSize);
        }
        painter.setFont(m_mainFont);

        m_dateTimeText = QLocale().toString(QDateTime::currentDateTime());
        m_dateTimeWidth = 0;
        if (m_settings->addDateAndTime) {
            m_dateTimeWidth = painter.boundingRect(leftMargin, topMargin, m_pageWidth, m_pageHeight, Qt::AlignRight, m_dateTimeText + "   ").width();
        }
        m_mainLineSpacing = painter.fontMetrics().lineSpacing();
        m_headerTextRect = painter.boundingRect((int)leftMargin, (int)topMargin, m_pageWidth - m_dateTimeWidth, m_pageHeight, Qt::AlignLeft | Qt::TextWordWrap, m_headerText);
        m_headerTextRect.setRight(m_headerTextRect.right() + 10);
        m_headerTextRect.setWidth(qMin(int(m_pageWidth - m_dateTimeWidth), m_headerTextRect.width()));
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << "after initialization";

    // screen only
    if (!printer) {
        painter.setWindow(0, 0, int((double)w * m_fx), int((double)h * m_fy));
    }

    painter.setFont(m_mainFont);
    if (paint) {
        // paint header
        painter.drawText(m_headerTextRect, Qt::AlignLeft | Qt::TextWordWrap, m_headerText);
        if (m_settings->addDateAndTime) {
            painter.drawText((int)leftMargin + m_pageWidth - m_dateTimeWidth, (int)topMargin, m_dateTimeWidth, m_headerTextRect.height(), Qt::AlignRight, m_dateTimeText);
        }

        // footer
        if (m_settings->addPageNumbers) {
            QString pageNumString;
            if (m_pagesCount > 0) {
                pageNumString = i18nc("Page (number) of (total)", "Page %1 of %2", pageNumber + 1, m_pagesCount);
            } else {
                pageNumString = i18n("Page %1", pageNumber + 1);
            }
            painter.drawText((int)leftMargin, (int)topMargin + m_pageHeight - m_mainLineSpacing, m_pageWidth, m_mainLineSpacing, Qt::AlignRight | Qt::AlignBottom, pageNumString);
        }
    }

    w = m_pageWidth;
    h = m_pageHeight;
    y = (int)topMargin;
    h -= (m_headerTextRect.height() + 1);
    y += (m_headerTextRect.height());
    if (m_settings->addTableBorders) {
        w -= 2;
        h -= 2;
    }
    if (m_settings->addPageNumbers) {
        h -= (m_mainLineSpacing * 3 / 2 + 1);
    }

    qCDebug(KGRAPHVIEWERLIB_LOG) << "(w, h) = (" << w << ", " << h << ")";
    if (((m_settings->fitToOnePage) || (m_painting.width() <= w && m_painting.height() <= h)) && !m_eof) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "single-page printing";
        if (paint) {
            QPixmap pix = m_painting.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            qCDebug(KGRAPHVIEWERLIB_LOG) << "drawPixmap";
            painter.drawPixmap((int)leftMargin, y, pix);
        }
        m_eof = true;
    } else if (m_settings->horizFitting != 0 || m_settings->vertFitting != 0) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "fitted multi-pages printing page " << pageNumber;
        int nbTilesByRow = (int)(ceil((double)m_painting.width()) / w) + 1;
        qCDebug(KGRAPHVIEWERLIB_LOG) << "  nb tiles by row = " << nbTilesByRow;

        int tileWidth = w;
        int tileHeight = h;

        if (m_settings->horizFitting != 0) {
            tileWidth = int(ceil(((double)m_painting.width()) / m_settings->horizFitting));
            nbTilesByRow = m_settings->horizFitting;
        }
        if (m_settings->vertFitting != 0) {
            tileHeight = int(ceil(((double)m_painting.height()) / m_settings->vertFitting));
        }
        qCDebug(KGRAPHVIEWERLIB_LOG) << "  tile size = " << tileWidth << "/" << tileHeight;
        int rowNum = pageNumber / nbTilesByRow;
        int columnNum = pageNumber % nbTilesByRow;
        int x1, y1, x2, y2;
        x1 = int(tileWidth * columnNum);
        x2 = int(tileWidth * (columnNum + 1));
        y1 = int(tileHeight * rowNum);
        y2 = int(tileHeight * (rowNum + 1));
        qCDebug(KGRAPHVIEWERLIB_LOG) << "(x1, y1, x2, 2) = (" << x1 << "," << y1 << "," << x2 << "," << y2 << ")";
        qCDebug(KGRAPHVIEWERLIB_LOG) << "painting size = (" << m_painting.width() << "/" << m_painting.height() << ")";
        if (paint) {
            Qt::AspectRatioMode scaleMode = Qt::IgnoreAspectRatio;
            if (m_settings->chainedFittings) {
                scaleMode = Qt::KeepAspectRatio;
            }
            QPixmap pix = m_painting.copy(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
            if (m_settings->horizFitting == 0) {
                pix = pix.scaled(pix.width(), h, scaleMode, Qt::SmoothTransformation);
            } else if (m_settings->vertFitting == 0) {
                pix = pix.scaled(w, pix.height(), scaleMode, Qt::SmoothTransformation);
            } else {
                pix = pix.scaled(w, h, scaleMode, Qt::SmoothTransformation);
            }
            painter.drawPixmap((int)leftMargin, y, pix);
        }
        if (x2 >= m_painting.width() && y2 >= m_painting.height()) {
            m_eof = true;
        }
    } else {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "multi-pages printing page " << pageNumber;
        int nbTilesByRow = (int)(((double)m_painting.width()) / w) + 1;
        int rowNum = pageNumber / nbTilesByRow;
        int columnNum = pageNumber % nbTilesByRow;
        int x1, y1, x2, y2;
        x1 = int(w * columnNum);
        x2 = int(w * (columnNum + 1));
        y1 = int(h * rowNum);
        y2 = int(h * (rowNum + 1));
        qCDebug(KGRAPHVIEWERLIB_LOG) << "(x1, y1, x2, 2) = (" << x1 << "," << y1 << "," << x2 << "," << y2 << ")";
        if (paint) {
            painter.drawPixmap((int)leftMargin, y, m_painting.copy(x1, y1, x2 - x1 + 1, y2 - y1 + 1));
        }
        if (x2 >= m_painting.width() && y2 >= m_painting.height()) {
            m_eof = true;
        }
    }

    if (m_settings->addTableBorders) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "Adding table borders";
        int y1 = (int)topMargin;
        y1 += (m_headerTextRect.height());
        int y2 = (int)topMargin + m_pageHeight;
        if (m_settings->addPageNumbers) {
            y2 -= (m_headerTextRect.height() /*+ m_mainLineSpacing*3/2*/ + 1);
        }
        if (paint) {
            painter.drawLine((int)leftMargin, y1, (int)leftMargin + m_pageWidth - 1, y1);
            painter.drawLine((int)leftMargin + m_pageWidth - 1, y1, (int)leftMargin + m_pageWidth - 1, y2);
            painter.drawLine((int)leftMargin + m_pageWidth - 1, y2, (int)leftMargin, y2);
            painter.drawLine((int)leftMargin, y2, (int)leftMargin, y1);
        }
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << "paintPage done";
}

void KGVSimplePrintingEngine::calculatePagesCount(QPainter &painter)
{
    if (m_eof || !m_data) {
        m_pagesCount = 0;
        return;
    }

    uint pageNumber = 0;
    if (m_settings->fitToOnePage) {
        m_pagesCount = 1;
    } else {
        for (; !m_eof; ++pageNumber) {
            paintPage(pageNumber, painter, false /* !paint */);
        }
        m_pagesCount = pageNumber;
    }
}

void KGVSimplePrintingEngine::setTitleText(const QString &titleText)
{
    m_headerText = titleText;
}

uint KGVSimplePrintingEngine::maxHorizFit() const
{
    uint w = m_pageWidth;
    if (m_settings->addTableBorders) {
        w -= 2;
    }
    //   () << "maxHorizFit: " << m_painting.width() << " / " << w
    //     << " = " << m_painting.width()/w;
    return (uint)ceil(((double)m_painting.width()) / w) + 1;
}

uint KGVSimplePrintingEngine::maxVertFit() const
{
    uint h = m_pageHeight;
    h -= (m_headerTextRect.height() + 1);
    if (m_settings->addTableBorders) {
        h -= 2;
    }
    if (m_settings->addPageNumbers) {
        h -= (m_mainLineSpacing * 3 / 2 + 1);
    }
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "maxVertFit: " << m_painting.height() << " / " << h
    //     << " = " << m_painting.height()/h;
    return (uint)ceil(((double)m_painting.height())) / h + 1;
}

}

#include "moc_simpleprintingengine.cpp"
