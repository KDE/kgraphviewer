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
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "kgraphviewer.h"
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <iostream>
#include <qurl.h>
#include <qdir.h>

static const char description[] =
I18N_NOOP("A Graphviz dot graph viewer for KDE");

static const char version[] = "1.0.3";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Dot graph to open" ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
  KAboutData about("kgraphviewer", I18N_NOOP("KGraphViewer"), version, description,
                    KAboutData::License_GPL, "(C) 2005-2006 Gaël de Chalendar", 0, 0, "kleag@free.fr");
  about.addAuthor( "Gaël de Chalendar", 0, "kleag@free.fr" );
  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions( options );
  KApplication app;
  
// see if we are starting with session management
  if (app.isRestored())
  {
      RESTORE(kgraphviewer);
  }
  else
  {
      // no session.. just start up normally
      KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

      kgraphviewer *widget = 0;
      if ( args->count() == 0 )
      {
      widget = new kgraphviewer;
      widget->show();
      }
      else
      {
        QCString remApp, remObj, remFun, foundApp, foundObj;
        QByteArray data;
        DCOPClient* client = new DCOPClient();;
        client->attach();
  
        remApp = "kgraphviewer*";
        remObj = "DCOPKGraphViewerIface";
        bool instanceExists =  (client->findObject(remApp, remObj, remFun, data, foundApp, foundObj));
  
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
            QByteArray data;
            QDataStream arg(data, IO_WriteOnly);
            QString strarg = args->arg(i);
            KURL url;
            if (strarg.left(1) == "/")
              url = KURL(QUrl(strarg));
            else url = KURL(QDir::currentDirPath() +'/' + strarg);
            arg << url;
//             kdError() << "Sending openURL " << url << " to " << foundApp << " / " << foundObj << endl;
            client->send(foundApp,foundObj,"openURL(KURL)",data);
            exit(0);
          }
          else
          {
            widget = new kgraphviewer;
            widget->show();
            widget->openURL( args->url( i ) );
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
