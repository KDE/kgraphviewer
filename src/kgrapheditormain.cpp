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
#include "kgrapheditor.h"
#include "kgrapheditoradaptor.h"
#include "config-kgraphviewer.h"
#include "kgrapheditor_debug.h"
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

    KAboutData about(
        QStringLiteral("kgrapheditor"),
        i18n("KGraphEditor"),
        QStringLiteral(KGRAPHVIEWER_VERSION_STRING),
        i18n("A Graphviz DOT graph editor by KDE"),
        KAboutLicense::GPL,
        i18n("© 2005–2010 Gaël de Chalendar")
    );

    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setOrganizationName(QStringLiteral("KDE"));

    KAboutData::setApplicationData(about);

    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kgraphviewer"), app.windowIcon()));

    QCommandLineParser options;
    options.addHelpOption();
    options.addVersionOption();
    options.addPositionalArgument(QStringLiteral("url"), i18n("Path or URL to scan"), i18n("[url]"));
    about.setupCommandLine(&options);
    options.process(app);
    about.processCommandLine(&options);

    // see if we are starting with session management
    if (app.isSessionRestored()) {
        kRestoreMainWindows<KGraphEditor>();
    } else {
        // no session.. just start up normally
        QStringList args = options.positionalArguments();

        KGraphEditor *widget = nullptr;
        if (args.count() == 0) {
            widget = new KGraphEditor;
            new KgrapheditorAdaptor(widget);
            QDBusConnection::sessionBus().registerObject(QStringLiteral("/KGraphEditor"), widget);
            widget->show();
        } else {
            QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kgrapheditor"));

            bool instanceExists = reply.value();

            for (int i = 0; i < args.count(); i++) {
                if (instanceExists &&
                    (QMessageBox::question(nullptr, i18n("Opening in new window confirmation"), i18n("A KGraphEditor window is already open, do you want to open the file in it?")) == QMessageBox::Yes)) {
                    QByteArray tosenddata;
                    QDataStream arg(&tosenddata, QIODevice::WriteOnly);
                    QString strarg = args[i];
                    QUrl url = QUrl::fromUserInput(strarg, QDir::currentPath(), QUrl::AssumeLocalFile);
                    arg << url;
                    QDBusInterface iface(QStringLiteral("org.kde.kgrapheditor"), QStringLiteral("/KGraphEditor"), QString(), QDBusConnection::sessionBus());
                    if (iface.isValid()) {
                        QDBusReply<void> reply = iface.call(QStringLiteral("openUrl"), url.url(QUrl::PreferLocalFile));
                        if (reply.isValid()) {
                            qCDebug(KGRAPHEDITOR_LOG) << "Reply was valid" << Qt::endl;
                            return 0;
                        }

                        qCWarning(KGRAPHEDITOR_LOG) << "Call failed: " << reply.error().message() << Qt::endl;
                        return 1;
                    }
                    qCWarning(KGRAPHEDITOR_LOG) << "Invalid interface" << Qt::endl;
                    exit(0);
                } else {
                    widget = new KGraphEditor;
                    new KgrapheditorAdaptor(widget);
                    QDBusConnection::sessionBus().registerObject(QStringLiteral("/KGraphEditor"), widget);
                    widget->show();
                    widget->openUrl(QUrl::fromUserInput(args[0], QDir::currentPath(), QUrl::AssumeLocalFile));
                }
            }
        }
        if (widget) {
            widget->reloadPreviousFiles();
        }
    }
    return app.exec();
}
