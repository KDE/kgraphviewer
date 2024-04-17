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

#include "simpleprintingcommand.h"

// lib
#include "dotgraphview.h"
#include "simpleprintingengine.h"
#include "simpleprintingsettings.h"
#include "simpleprintingpagesetup.h"
#include "simpleprintpreviewwindow.h"
#include "kgraphviewerlib_debug.h"
// KF
#include <KLocalizedString>
// Qt
#include <QMessageBox>
#include <QPrintDialog>
#include <QVBoxLayout>
#include <QPrinter>

namespace KGraphViewer
{
KGVSimplePrintingCommand::KGVSimplePrintingCommand(DotGraphView *mainWin, int objectId, QObject *parent)
    : QObject(parent)
    , m_previewEngine(nullptr)
    , m_graphView(mainWin)
    , m_objectId(objectId)
    , m_settings(new KGVSimplePrintingSettings(KGVSimplePrintingSettings::load()))
    , m_previewWindow(nullptr)
    , m_printPreviewNeedsReloading(true)
    , m_pageSetupDialog(nullptr)
{
    setObjectName("KGVSimplePrintCommand");
    connect(this, &KGVSimplePrintingCommand::showPageSetupRequested, this, &KGVSimplePrintingCommand::slotShowPageSetupRequested);
}

KGVSimplePrintingCommand::~KGVSimplePrintingCommand()
{
    delete m_previewWindow;
    delete m_previewEngine;
    delete m_settings;
}

bool KGVSimplePrintingCommand::init(const QString &aTitleText)
{
    if (!m_previewEngine)
        m_previewEngine = new KGVSimplePrintingEngine(m_settings, this);

    QString titleText(aTitleText.trimmed());
    if (!m_previewWindow) {
        QString errorMessage;
        if (!m_previewEngine->init(*m_graphView, titleText, errorMessage)) {
            if (!errorMessage.isEmpty())
                QMessageBox::warning(m_graphView, i18n("Print Preview"), errorMessage);
            return false;
        }
        m_previewWindow = new KGVSimplePrintPreviewWindow(*m_previewEngine, QString(), nullptr);
        connect(m_previewWindow, &KGVSimplePrintPreviewWindow::printRequested, this, [&]() { print(); });
        connect(m_previewWindow, &KGVSimplePrintPreviewWindow::pageSetupRequested, this, &KGVSimplePrintingCommand::slotShowPageSetupRequested);
        //     KDialog::centerOnScreen(m_previewWindow);
        m_printPreviewNeedsReloading = false;
    }
    return true;
}

bool KGVSimplePrintingCommand::print(const QString &aTitleText)
{
    init(aTitleText);
    m_previewEngine->clear();

    // setup printing
    QPrinter printer;
    printer.setPageOrientation(m_settings->pageLayout.orientation == PG_PORTRAIT ? QPageLayout::Portrait : QPageLayout::Landscape);
    printer.setPageSize(QPageSize((QPageSize::PageSizeId)KgvPageFormat::printerPageSize(m_settings->pageLayout.format)));
    // #endif

    printer.setFullPage(true);
    QString docName(aTitleText);
    printer.setDocName(docName);
    printer.setCreator(QStringLiteral("kgraphviewer"));
    QPointer<QPrintDialog> dlg = new QPrintDialog(&printer, m_graphView);
    if (dlg->exec() != QDialog::Accepted) {
        return true;
    }

    // now we have final settings

    //! @todo get printer.pageOrder() (for reversed order requires improved engine)
    QPainter painter;

    if (!painter.begin(&printer)) {
        //! @todo msg
        return false;
    }
    m_previewEngine->calculatePagesCount(painter);

    uint loops, loopsPerPage;
    QList<int> pagesToPrint;
    int fromPage = 0;
    // on !win32 print QPrinter::copyCount() times (the OS does not perform buffering)
    //   pagesToPrint = printer.pageList();
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << pagesToPrint;
    if (pagesToPrint.isEmpty()) {
        fromPage = 0;
        for (int i = 0; i < (int)m_previewEngine->pagesCount(); i++) {
            //       std::cerr << "Page " << i << " has to be printed" << std::endl;
            pagesToPrint.append(i);
        }
    } else
        fromPage = pagesToPrint.first();
    if (printer.collateCopies()) {
        // collation: p1, p2,..pn; p1, p2,..pn; ......; p1, p2,..pn
        loops = printer.copyCount();
        loopsPerPage = 1;
    } else {
        // no collation: p1, p1, ..., p1; p2, p2, ..., p2; ......; pn, pn,..pn
        loops = 1;
        loopsPerPage = printer.copyCount();
    }
    //! @todo also look at printer.pageSet() option : all/odd/even pages
    // #endif
    // now, total number of printed pages is printer.copyCount()*printer.pageList().count()

    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "printing...";
    bool firstPage = true;
    for (uint copy = 0; copy < loops; copy++) {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "copy " << (copy+1) << " of " << loops;
        //     std::cerr << "fromPage = " << fromPage << " ; eof = " << m_previewEngine->eof() << std::endl;
        uint pageNumber = fromPage;
        QList<int>::ConstIterator pagesIt = pagesToPrint.constBegin();
        for (; (int)pageNumber == fromPage || !m_previewEngine->eof(); ++pageNumber) {
            //       std::cerr << "printing..." << std::endl;
            if (pagesIt == pagesToPrint.constEnd()) // no more pages to print
                break;
            if ((int)pageNumber < *pagesIt) { // skip pages without printing (needed for computation)
                m_previewEngine->paintPage(pageNumber, painter, false);
                continue;
            }
            if (*pagesIt < (int)pageNumber) { // sanity
                ++pagesIt;
                continue;
            }
            for (uint onePageCounter = 0; onePageCounter < loopsPerPage; onePageCounter++) {
                if (!firstPage)
                    printer.newPage();
                else
                    firstPage = false;
                //         std::cerr << "page #" << pageNumber << std::endl;
                m_previewEngine->paintPage(pageNumber, painter);
            }
            ++pagesIt;
        }
    }
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "end of printing.";

    // stop painting, this will automatically send the print data to the printer
    if (!painter.end())
        return false;

    //   if (!m_previewEngine->done())
    //     return false;

    return true;
}

bool KGVSimplePrintingCommand::showPrintPreview(const QString &aTitleText, bool reload)
{
    init(aTitleText);

    if (reload)
        m_printPreviewNeedsReloading = true;

    if (m_printPreviewNeedsReloading) { // dirty
        m_previewEngine->clear();
        //! @todo progress bar...
        m_previewEngine->setTitleText(aTitleText);
        m_previewWindow->setFullWidth();
        m_previewWindow->updatePagesCount();
        m_printPreviewNeedsReloading = false;
        m_previewWindow->goToPage(0);
    }
    m_previewWindow->show();
    m_previewWindow->raise();
    return true;
}

void KGVSimplePrintingCommand::hidePageSetup()
{
    if (m_pageSetupDialog) {
        m_pageSetupDialog->hide();
    }
}

void KGVSimplePrintingCommand::hidePrintPreview()
{
    if (m_previewWindow) {
        m_previewWindow->hide();
    }
}

void KGVSimplePrintingCommand::slotShowPageSetupRequested()
{
    if (m_pageSetupDialog == nullptr) {
        m_pageSetupDialog = new QDialog(nullptr);
        QMap<QString, QString> map{
            {QStringLiteral("action"), QStringLiteral("pageSetup")},
            {QStringLiteral("title"), m_graphView->dotFileName()},
        };
        QVBoxLayout *lyr = new QVBoxLayout(m_pageSetupDialog);
        KGVSimplePrintingPageSetup *sppsb = new KGVSimplePrintingPageSetup(this, m_graphView, m_pageSetupDialog, &map);
        if (m_previewWindow) {
            connect(sppsb, &KGVSimplePrintingPageSetup::needsRedraw, m_previewWindow, &KGVSimplePrintPreviewWindow::slotRedraw);
        }
        lyr->addWidget(sppsb);
    }
    m_pageSetupDialog->show();
    m_pageSetupDialog->raise();
}

void KGVSimplePrintingCommand::showPageSetup(const QString &aTitleText)
{
    init(aTitleText);
    Q_EMIT showPageSetupRequested();
}

}

#include "moc_simpleprintingcommand.cpp"
