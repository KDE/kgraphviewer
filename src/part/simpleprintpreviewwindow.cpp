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

#include "simpleprintpreviewwindow.h"
#include "simpleprintpreviewwindow_p.h"

// lib
#include "simpleprintingengine.h"
#include "kgraphviewerlib_debug.h"
// KF
#include <KToolBar>
#include <KLocalizedString>
// Qt
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QKeyEvent>
#include <QTimer>

namespace KGraphViewer
{
KGVSimplePrintPreviewWindow::KGVSimplePrintPreviewWindow(KGVSimplePrintingEngine &engine, const QString &previewName, QWidget *parent)
    : QWidget(parent)
    , m_engine(engine)
    , m_settings(m_engine.settings())
    , m_pageNumber(-1)
    , m_actions(this)
{
    setObjectName("KGVSimplePrintPreviewWindow");
    //	m_pagesCount = INT_MAX;

    setWindowTitle(i18n("%1 - Print Preview - %2", previewName, QString()));
    setWindowIcon(QIcon::fromTheme(QLatin1String("document-print-preview")));
    QVBoxLayout *lyr = new QVBoxLayout();

    m_toolbar = new KToolBar(this);
    m_toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lyr->addWidget(m_toolbar);

    QAction *printAction = KStandardAction::print(this, &KGVSimplePrintPreviewWindow::slotPrintClicked, &m_actions);
    m_toolbar->addAction((QAction *)printAction);
    /// @todo handle the accelerator
    // 	static_cast<KPushButton*>(m_toolbar->getWidget(id))->setAccel(Qt::CTRL|Qt::Key_P);
    /// @todo add the separator
    // 	m_toolbar->addSeparator();

    QAction *pageSetupAction = new QAction(i18n("&Page setup"), this);
    //                                &m_actions, "file_page_setup");
    connect(pageSetupAction, &QAction::triggered, this, &KGVSimplePrintPreviewWindow::slotPageSetup);
    m_toolbar->addAction((QAction *)pageSetupAction);

    // 	id = m_toolbar->insertWidget(-1, 0, new KPushButton(i18n("Page Set&up..."), m_toolbar));
    /// @todo handle the accelerator
    // 	m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotPageSetup()));
    /// @todo add the separator
    // 	m_toolbar->addSeparator();

#ifndef KGV_NO_UNFINISHED
//! @todo unfinished
/*	id = m_toolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("viewmag+"), i18n("Zoom In"), m_toolbar));
    m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotZoomInClicked()));
    m_toolbar->addSeparator();

    id = m_toolbar->insertWidget( -1, 0, new KPushButton(BarIconSet("viewmag-"), i18n("Zoom Out"), m_toolbar));
    m_toolbar->addConnection(id, SIGNAL(clicked()), this, SLOT(slotZoomOutClicked()));
    m_toolbar->addSeparator();*/
#endif

    QAction *closeAction = KStandardAction::close(this, &KGVSimplePrintPreviewWindow::close, &m_actions);
    m_toolbar->addAction((QAction *)closeAction);

    m_scrollView = new KGVSimplePrintPreviewScrollView(this);
    m_scrollView->setUpdatesEnabled(true);
    m_view = (KGVSimplePrintPreviewView *)m_scrollView->widget();
    m_scrollView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lyr->addWidget(m_scrollView);

    m_navToolbar = new KToolBar(this);
    m_navToolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lyr->addWidget(m_navToolbar);

    QAction *firstPageAction = KStandardAction::firstPage(this, &KGVSimplePrintPreviewWindow::slotFirstClicked, &m_actions);
    m_navToolbar->addAction((QAction *)firstPageAction);
    m_navToolbar->addSeparator();

    QAction *previousAction = new QAction(i18n("Previous Page"), this);
    //                                &m_actions,  "prevpage");
    connect(previousAction, &QAction::triggered, this, &KGVSimplePrintPreviewWindow::slotPreviousClicked);
    m_navToolbar->addAction((QAction *)previousAction);
    m_navToolbar->addSeparator();

    m_pageNumberLabel = new QLabel(m_navToolbar);
    m_navToolbar->addWidget(m_pageNumberLabel);
    m_navToolbar->addSeparator();

    QAction *nextAction = new QAction(i18n("Next Page"), this); //&m_actions, "nextpage");
    connect(nextAction, &QAction::triggered, this, &KGVSimplePrintPreviewWindow::slotNextClicked);
    m_navToolbar->addAction((QAction *)nextAction);
    m_navToolbar->addSeparator();

    QAction *lastPageAction = KStandardAction::lastPage(this, &KGVSimplePrintPreviewWindow::slotLastClicked, &m_actions);
    //                                       &m_actions);
    m_navToolbar->addAction((QAction *)lastPageAction);
    m_navToolbar->addSeparator();

    // TODO: how to know which screen this will be shown on?
    resize(width(), qApp->primaryScreen()->size().height() * 4 / 5);

    this->setLayout(lyr);
    //! @todo progress bar...

    QTimer::singleShot(50, this, &KGVSimplePrintPreviewWindow::initLater);
}

void KGVSimplePrintPreviewWindow::initLater()
{
    qCDebug(KGRAPHVIEWERLIB_LOG);
    setFullWidth();
    updatePagesCount();
    goToPage(0);
}

KGVSimplePrintPreviewWindow::~KGVSimplePrintPreviewWindow()
{
}

void KGVSimplePrintPreviewWindow::slotPrintClicked()
{
    hide();
    Q_EMIT printRequested();
    show();
    raise();
}

void KGVSimplePrintPreviewWindow::slotPageSetup()
{
    lower();
    Q_EMIT pageSetupRequested();
}

void KGVSimplePrintPreviewWindow::slotZoomInClicked()
{
    //! @todo
}

void KGVSimplePrintPreviewWindow::slotZoomOutClicked()
{
    //! @todo
}

void KGVSimplePrintPreviewWindow::slotFirstClicked()
{
    goToPage(0);
}

void KGVSimplePrintPreviewWindow::slotPreviousClicked()
{
    goToPage(m_pageNumber - 1);
}

void KGVSimplePrintPreviewWindow::slotNextClicked()
{
    goToPage(m_pageNumber + 1);
}

void KGVSimplePrintPreviewWindow::slotLastClicked()
{
    goToPage(m_engine.pagesCount() - 1);
}

void KGVSimplePrintPreviewWindow::slotRedraw()
{
    m_engine.clear();
    setFullWidth();
    updatePagesCount();
    m_pageNumber = 0;
    m_view->repaint(); // this will automatically paint a new page

    //   m_navToolbar->setItemEnabled(m_idNext, m_pageNumber < ((int)m_engine.pagesCount()-1));
    //   m_navToolbar->setItemEnabled(m_idLast, m_pageNumber < ((int)m_engine.pagesCount()-1));
    //   m_navToolbar->setItemEnabled(m_idPrevious, m_pageNumber > 0);
    //   m_navToolbar->setItemEnabled(m_idFirst, m_pageNumber > 0);
    m_pageNumberLabel->setText(i18nc("Page (number) of (total)", "Page %1 of %2", m_pageNumber + 1, m_engine.pagesCount()));
}

void KGVSimplePrintPreviewWindow::goToPage(int pageNumber)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << pageNumber;
    if (pageNumber == m_pageNumber || pageNumber < 0 || pageNumber > ((int)m_engine.pagesCount() - 1))
        return;
    m_pageNumber = pageNumber;

    m_view->repaint(); // this will automatically paint a new page
    //	if (m_engine.eof())
    //		m_pagesCount = pageNumber+1;

    // 	m_navToolbar->setItemEnabled(m_idNext, pageNumber < ((int)m_engine.pagesCount()-1));
    // 	m_navToolbar->setItemEnabled(m_idLast, pageNumber < ((int)m_engine.pagesCount()-1));
    // 	m_navToolbar->setItemEnabled(m_idPrevious, pageNumber > 0);
    // 	m_navToolbar->setItemEnabled(m_idFirst, pageNumber > 0);
    m_pageNumberLabel->setText(i18nc("Page (number) of (total)", "Page %1 of %2", m_pageNumber + 1, m_engine.pagesCount()));
}

void KGVSimplePrintPreviewWindow::setFullWidth()
{
    m_scrollView->setFullWidth();
}

void KGVSimplePrintPreviewWindow::updatePagesCount()
{
    QPainter p(this);
    //   QPainter p(m_view);
    //   p.begin(this);
    m_engine.calculatePagesCount(p);
    //   p.end();
}

bool KGVSimplePrintPreviewWindow::event(QEvent *e)
{
    QEvent::Type t = e->type();
    if (t == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        const int k = ke->key();
        bool ok = true;
        if (k == Qt::Key_Equal || k == Qt::Key_Plus)
            slotZoomInClicked();
        else if (k == Qt::Key_Minus)
            slotZoomOutClicked();
        else if (k == Qt::Key_Home)
            slotFirstClicked();
        else if (k == Qt::Key_End)
            slotLastClicked();
        else
            ok = false;

        if (ok) {
            ke->accept();
            return true;
        }
    } else if (t == QEvent::ShortcutOverride) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        const int k = ke->key();
        bool ok = true;
        if (k == Qt::Key_PageUp)
            slotPreviousClicked();
        else if (k == Qt::Key_PageDown)
            slotNextClicked();
        else
            ok = false;

        if (ok) {
            ke->accept();
            return true;
        }
    }
    return QWidget::event(e);
}

}

#include "moc_simpleprintpreviewwindow.cpp"
