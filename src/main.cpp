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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
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

static const char version[] = "1.0";

int main(int argc, char **argv)
{
  KAboutData about("kgraphviewer", 0, ki18n("KGraphViewer"), version, ki18n(description),
                    KAboutData::License_GPL, ki18n("(C) 2005-2006 Gaël de Chalendar"), KLocalizedString(), 0, "kleag@free.fr");
  about.addAuthor( ki18n("Gaël de Chalendar"), KLocalizedString(), "kleag@free.fr" );
  KCmdLineArgs::init(argc, argv, &about);

  KCmdLineOptions options;
  options.add("+[URL]", ki18n( "Dot graph to open" ));
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;
  
// see if we are starting with session management
  if (app.isSessionRestored())
  {
      RESTORE(KGraphViewer);
  }
  else
  {
      // no session.. just start up normally
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

      KGraphViewer *widget = 0;
      if ( args->count() == 0 )
      {
        widget = new KGraphViewer;
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
                                         i18n("There is already a KGraphViewer window opened. What's your choice ?"),
                                         i18n("Opening in new window confirmation"),
                                             KGuiItem("Open in the existing one"),
                                             KGuiItem("Open in new window"),
                                             "openInNewWindowMode"   ) == KMessageBox::Yes) )
          {
            QByteArray tosenddata;
            QDataStream arg(&tosenddata, QIODevice::WriteOnly);
            QString strarg = args->arg(i);
            KUrl url;
            if (strarg.left(1) == "/")
              url = KUrl(strarg);
            else url = KUrl(QDir::currentPath() +"/" + strarg);
            arg << url;
            QDBusInterface iface("org.kde.kgraphviewer", "/KGraphViewer", "", QDBusConnection::sessionBus());
            if (iface.isValid()) 
            {
              QDBusReply<void> reply = iface.call("openUrl", url.pathOrUrl());
              if (reply.isValid()) 
              {
                kDebug() << "Reply was valid" << endl;
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
            widget = new KGraphViewer;
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
