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


#include "kgrapheditor.h"
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

#include "kgrapheditoradaptor.h"

using namespace KGraphViewer;

static const char description[] =
I18N_NOOP("A Graphviz dot graph viewer for KDE");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
  KAboutData about("kgrapheditor", 0, ki18n("KGraphEditor"), version, ki18n(description),
                    KAboutData::License_GPL, ki18n("(C) 2005-2010 Gaël de Chalendar"), KLocalizedString(), 0, "kleag@free.fr");
  about.addAuthor( ki18n("Gaël de Chalendar"), KLocalizedString(), "kleag@free.fr" );
  KCmdLineArgs::init(argc, argv, &about);

  KCmdLineOptions options;
  options.add("+[URL]", ki18n( "Dot graph to open" ));
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;
  
// see if we are starting with session management
  if (app.isSessionRestored())
  {
      RESTORE(KGraphEditor);
  }
  else
  {
      // no session.. just start up normally
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

      KGraphEditor *widget = 0;
      if ( args->count() == 0 )
      {
        widget = new KGraphEditor;
        new KgrapheditorAdaptor(widget);
        QDBusConnection::sessionBus().registerObject("/KGraphEditor", widget);
        widget->show();
      }
      else
      {
        QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.kgrapheditor" );
      
        bool instanceExists = reply.value();
  
        for (int i = 0; i < args->count(); i++ )
        {
          if (instanceExists 
              && (KMessageBox::questionYesNo(0, 
                                         i18n("A KGraphEditor window is already open, where do you want to open this file?"),
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
            QDBusInterface iface("org.kde.kgrapheditor", "/KGraphEditor", "", QDBusConnection::sessionBus());
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
            widget = new KGraphEditor;
            new KgrapheditorAdaptor(widget);
            QDBusConnection::sessionBus().registerObject("/KGraphEditor", widget);
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
