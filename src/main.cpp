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


#include "kgraphviewer.h"
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <iostream>
#include <qdir.h>
#include <QByteArray>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

#include "kgraphvieweradaptor.h"

static const char description[] =
I18N_NOOP("A Graphviz dot graph viewer for KDE");

static const char version[] = "2.0.2";

int main(int argc, char **argv)
{
  KAboutData about("kgraphviewer", 0, ki18n("KGraphViewer"), version, ki18n(description),
                    KAboutData::License_GPL, ki18n("(C) 2005-2006 Gaël de Chalendar"), KLocalizedString(), 0, "kleag@free.fr");
  about.addAuthor( ki18n("Gaël de Chalendar"), ki18n("Original Author and current maintainer"), "kleag@free.fr" );
  about.addAuthor( ki18n("Reimar Döffinger"), ki18n("Contributor"), "Reimar.Doeffinger@stud.uni-karlsruhe.de" );
  about.addAuthor( ki18n("Matthias Peinhardt"), ki18n("Contributor"), "matthias.peinhardt@googlemail.com" );
  about.addAuthor( ki18n("Sandro Andrade"), ki18n("Contributor"), "sandro.andrade@gmail.com" );
  
  KCmdLineArgs::init(argc, argv, &about);

  KCmdLineOptions options;
  options.add("+[URL]", ki18n( "Dot graph to open" ));
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;
  
// see if we are starting with session management
  if (app.isSessionRestored())
  {
    RESTORE(KGraphViewerWindow);
  }
  else
  {
      // no session.. just start up normally
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

      KGraphViewerWindow *widget = 0;
      if ( args->count() == 0 )
      {
        widget = new KGraphViewerWindow;
        new KgraphviewerAdaptor(widget);
        QDBusConnection::sessionBus().registerObject("/KGraphViewer", widget);
        widget->show();
      }
      else
      {
        QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.kgraphviewer" );
      
        bool instanceExists = reply.value();
  
        for (int i = 0; i < args->count(); i++ )
        {
          if (instanceExists 
              && (KMessageBox::questionYesNo(0, 
                                         i18n("A KGraphViewer window is already open, where do you want to open this file?"),
                                         i18n("Opening in new window confirmation"),
                                             KGuiItem(i18nc("Where to open a file", "Existing window")),
                                             KGuiItem(i18nc("Where to open a file", "New window")),
                                             "openInNewWindowMode"   ) == KMessageBox::Yes) )
          {
            QByteArray tosenddata;
            QDataStream arg(&tosenddata, QIODevice::WriteOnly);
            QString strarg = args->arg(i);
            KUrl url;
            if (strarg.left(1) == "/")
              url = KUrl(strarg);
            else url = KUrl(QDir::currentPath() + '/' + strarg);
            arg << url;
            QDBusInterface iface("org.kde.kgraphviewer", "/KGraphViewer", "", QDBusConnection::sessionBus());
            if (iface.isValid()) 
            {
              QDBusReply<void> reply = iface.call("openUrl", url.pathOrUrl());
              if (reply.isValid()) 
              {
                kDebug() << "Reply was valid";
                return 0;
              }

              kError() << "Call failed: " << reply.error().message() << endl;
              return 1;
            }
            kError() << "Invalid interface" << endl;
            exit(0);
          }
          else
          {
            widget = new KGraphViewerWindow;
            new KgraphviewerAdaptor(widget);
            QDBusConnection::sessionBus().registerObject("/KGraphViewer", widget);
            widget->show();
            widget->openUrl( args->url( i ) );
          }
        }
      }
      args->clear();
    if (widget != 0)
    {
      widget->  reloadPreviousFiles();
    }
    
  }
  return app.exec();
}
