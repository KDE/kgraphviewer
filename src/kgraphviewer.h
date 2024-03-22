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

#ifndef _KGRAPHVIEWER_H_
#define _KGRAPHVIEWER_H_

// KF
#include <KParts/MainWindow>
// Qt
#include <QMap>
#include <QUrl>
#include <QDir>

class KRecentFilesAction;
class KToggleAction;

class QTabWidget;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Gael de Chalendar <kleag@free.fr>
 */
class KGraphViewerWindow : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    KGraphViewerWindow();

    /**
     * Default Destructor
     */
    ~KGraphViewerWindow() override;

    /**
     * Use this method to load whatever file/URL you have
     */
    void openUrl(const QUrl &url);

    void reloadPreviousFiles();

protected:
    void closeEvent(QCloseEvent *event) override;

Q_SIGNALS:
    void hide(KParts::Part *part);

public Q_SLOTS:
    /**
     * Use this method to load whatever file/URL you have
     */
    void openUrl(const QString &url)
    {
        openUrl(QUrl::fromUserInput(url, QDir::currentPath(), QUrl::AssumeLocalFile));
    }

    void close();

    void slotReloadOnChangeModeYesToggled(bool value);
    void slotReloadOnChangeModeNoToggled(bool value);
    void slotReloadOnChangeModeAskToggled(bool value);
    void slotOpenInExistingWindowModeYesToggled(bool value);
    void slotOpenInExistingWindowModeNoToggled(bool value);
    void slotOpenInExistingWindowModeAskToggled(bool value);
    void slotReopenPreviouslyOpenedFilesModeYesToggled(bool value);
    void slotReopenPreviouslyOpenedFilesModeNoToggled(bool value);
    void slotReopenPreviouslyOpenedFilesModeAskToggled(bool value);
    void slotParsingModeExternalToggled(bool value);
    void slotParsingModeInternalToggled(bool value);

private Q_SLOTS:
    void fileNew();
    void fileOpen();
    void close(int index);
    void slotURLSelected(const QUrl &);
    void optionsShowToolbar();
    void optionsShowStatusbar();
    void optionsConfigureToolbars();
    void optionsConfigure();
    void newTabSelectedSlot(int index);

    void applyNewToolbarConfig();

    void slotHoverEnter(const QString &);
    void slotHoverLeave(const QString &);
    void slotBackgroundColorChanged(const QColor &);

private:
    void setupAccel();
    void setupActions();

private:
    QTabWidget *m_widget;
    KRecentFilesAction *m_rfa;
    KParts::PartManager *m_manager;

    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
    QAction *m_closeAction;

    QStringList m_openedFiles;

    QMap<QWidget *, KParts::Part *> m_tabsPartsMap;
    QMap<QWidget *, QString> m_tabsFilesMap;
};

#endif // _KGRAPHVIEWER_H_
