/* This file is part of KGraphViewer.
   Copyright (C) 2005-2010 Gael de Chalendar <kleag@free.fr>

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

// app
#include "kgraphviewer.h"
#include "kgraphvieweradaptor.h"
#include "config-kgraphviewer.h"
#include "kgraphviewer_debug.h"
// KF
#include <KAboutData>
#include <KLocalizedString>
// Qt
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("kgraphviewer");

    KAboutData about(QStringLiteral("kgraphviewer"),
                     i18n("KGraphViewer"),
                     QStringLiteral(KGRAPHVIEWER_VERSION_STRING),
                     i18n("A Graphviz DOT graph viewer"),
                     KAboutLicense::GPL,
                     i18n("(C) 2005-2010 Gaël de Chalendar"),
                     QString(),
                     QStringLiteral("https://apps.kde.org/kgraphviewer"));
    about.addAuthor(i18n("Gaël de Chalendar"), i18n("Original Author and current maintainer"), QStringLiteral("kleag@free.fr"));
    about.addAuthor(i18n("Reimar Döffinger"), i18n("Contributor"), QStringLiteral("kde@reimardoeffinger.de"));
    about.addAuthor(i18n("Matthias Peinhardt"), i18n("Contributor"), QStringLiteral("matthias.peinhardt@googlemail.com"));
    about.addAuthor(i18n("Sandro Andrade"), i18n("Contributor"), QStringLiteral("sandro.andrade@gmail.com"));
    about.addAuthor(i18n("Milian Wolff"), i18n("Contributor"), QStringLiteral("mail@milianw.de"));
    about.addAuthor(i18n("Martin Sandsmark"), i18n("Port to KF5"), QStringLiteral("martin.sandsmark@kde.org"));

    app.setOrganizationName(QStringLiteral("KDE"));
    KAboutData::setApplicationData(about);

    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kgraphviewer"), app.windowIcon()));

    QCommandLineParser options;
    options.addPositionalArgument(QStringLiteral("url"), i18n("Path or URL to scan"), i18n("[url]"));
    about.setupCommandLine(&options);
    options.process(app);
    about.processCommandLine(&options);

    // see if we are starting with session management
    if (app.isSessionRestored()) {
        kRestoreMainWindows<KGraphViewerWindow>();
    } else {
        // no session.. just start up normally
        QStringList args = options.positionalArguments();

        KGraphViewerWindow *widget = nullptr;
        if (args.count() == 0) {
            widget = new KGraphViewerWindow;
            new KgraphviewerAdaptor(widget);
            QDBusConnection::sessionBus().registerObject(QStringLiteral("/KGraphViewer"), widget);
            widget->show();
        } else {
            QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kgraphviewer"));

            bool instanceExists = reply.value();

            for (int i = 0; i < args.count(); i++) {
                if (instanceExists &&
                    (QMessageBox::question(nullptr, i18n("Opening in new window confirmation"), i18n("A KGraphViewer window is already open, where do you want to open this file in the existing window?")) == QMessageBox::Yes)) {
                    QByteArray tosenddata;
                    QDataStream arg(&tosenddata, QIODevice::WriteOnly);
                    const QUrl url = QUrl::fromUserInput(args[i], QDir::currentPath(), QUrl::AssumeLocalFile);
                    arg << url;
                    QDBusInterface iface(QStringLiteral("org.kde.kgraphviewer"), QStringLiteral("/KGraphViewer"), QString(), QDBusConnection::sessionBus());
                    if (iface.isValid()) {
                        QDBusReply<void> reply = iface.call(QStringLiteral("openUrl"), url.url(QUrl::PreferLocalFile));
                        if (reply.isValid()) {
                            qCWarning(KGRAPHVIEWER_LOG) << "Reply was valid";
                            return 0;
                        }

                        qCWarning(KGRAPHVIEWER_LOG) << "Call failed: " << reply.error().message();
                        return 1;
                    }
                    qCWarning(KGRAPHVIEWER_LOG) << "Invalid interface";
                    exit(0);
                } else {
                    widget = new KGraphViewerWindow;
                    new KgraphviewerAdaptor(widget);
                    QDBusConnection::sessionBus().registerObject(QStringLiteral("/KGraphViewer"), widget);
                    widget->show();
                    widget->openUrl(QUrl::fromUserInput(args[i], QDir::currentPath(), QUrl::AssumeLocalFile));
                }
            }
        }
        if (widget) {
            widget->reloadPreviousFiles();
        }
    }
    return app.exec();
}
