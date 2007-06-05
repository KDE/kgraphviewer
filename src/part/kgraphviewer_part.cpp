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


#include "kgraphviewer_part.h"

#include <kcomponentdata.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kselectaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kparts/factory.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qicon.h>
#include <qpixmap.h>
#include <qlayout.h>

#include <iostream>

// #include "kgraphviewersettings.h"
#include "kgraphviewer_partsettings.h"

using namespace KGraphViewer;

kgraphviewerPart::kgraphviewerPart( QWidget *parentWidget, QObject *parent)
: KParts::ReadOnlyPart(parent), m_watch(new KDirWatch())
{
//   std::cerr << "kgraphviewerPart::kgraphviewerPart" << std::endl;
  // we need an instance
  setComponentData( kgraphviewerPartFactory::componentData() );

  // this should be your custom internal widget
  m_widget = new DotGraphView( actionCollection(), parentWidget);
  m_widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  
  // notify the part that this is our internal widget
  setWidget(m_widget);

  QAction* closeAct = actionCollection()->addAction( KStandardAction::Close, "file_close", parentWidget, SLOT( close() ) );
  closeAct->setWhatsThis(i18n("Closes the current tab"));

  QAction* printAct = actionCollection()->addAction(KStandardAction::Print, "file_print", m_widget, SLOT(print()));
  printAct->setWhatsThis(i18n("Print the graph using current page setup settings"));
  printAct->setShortcut(Qt::ControlModifier + Qt::Key_P);
  
  QAction* printPreviewAct = actionCollection()->addAction(KStandardAction::PrintPreview, "file_print_preview", m_widget, SLOT(printPreview()));
  printPreviewAct->setWhatsThis(i18n("Open the print preview window"));
  printPreviewAct->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_P);
  
//   KAction* pagesetupAct = new KAction(i18n("&Page setup"), this); //actionCollection(), "file_page_setup");
  QAction* pagesetupAct = actionCollection()->addAction("file_page_setup", m_widget, SLOT(pageSetup()));
  pagesetupAct->setText(i18n("Page setup"));
  pagesetupAct->setWhatsThis(i18n("Opens the Page Setup dialog allowing to setup how the graph will be printed"));

  QAction* redisplayAct = actionCollection()->addAction(KStandardAction::Redisplay, "view_redisplay", m_widget, SLOT(reload()));
  redisplayAct->setWhatsThis(i18n("Reload the current graph from file"));
  redisplayAct->setShortcut(Qt::Key_F5);

  QAction* zoomInAct = actionCollection()->addAction(KStandardAction::ZoomIn, "view_zoom_in", m_widget, SLOT(zoomIn()));
  zoomInAct->setWhatsThis(i18n("Zoom in by 10% the currently viewed graph"));
  zoomInAct->setShortcut(Qt::Key_F7);
  
  QAction* zoomOutAct = actionCollection()->addAction(KStandardAction::ZoomOut, "view_zoom_out", m_widget, SLOT(zoomOut()));
  zoomOutAct->setWhatsThis(i18n("Zoom out by 10% the currently viewed graph"));
  zoomOutAct->setShortcut(Qt::Key_F8);

// set our XML-UI resource file
  setXMLFile("kgraphviewer_part.rc");
}

kgraphviewerPart::~kgraphviewerPart()
{
  delete m_watch; 
}

bool kgraphviewerPart::openFile()
{
  kDebug() << "kgraphviewerPart::openFile" << localFilePath() << endl;
//    m_widget->loadedDot( localFilePath() );
  if (!m_widget->loadDot(localFilePath()))
    return false;
//   std::cerr << "Watching file " << localFilePath() << std::endl;
  m_watch->addFile(localFilePath());
  connect(m_watch, SIGNAL(dirty(const QString &)), m_widget, SLOT(dirty(const QString&)));
  QString label = localFilePath().section('/',-1,-1);
  
  m_widget->show();
  return true;
}

void kgraphviewerPart::slotHide(KParts::Part* part)
{
  if (part == this)
  {
    m_widget->hideToolsWindows();
  }
}

/*It's usually safe to leave the factory code alone.. with the
notable exception of the KAboutData data*/
#include <kaboutdata.h>
#include <klocale.h>

extern "C"
{
  /**
   * This function is the 'main' function of this part.  It takes
   * the form 'void *init_lib<library name>()  It always returns a
   * new factory object
   */
  KDE_EXPORT void *init_kgraphviewerpart()
  {
    return new kgraphviewerPartFactory;
  }
}

// KComponentData kgraphviewerPartFactory::s_instance = 0L;
KAboutData* kgraphviewerPartFactory::s_about = new KAboutData(
    "kgraphviewerpart", I18N_NOOP("kgraphviewerPart"),
    "1.0", I18N_NOOP( "GraphViz dot files viewer" ),
    KAboutData::License_GPL,
    "(c) 2005-2006, Gaël de Chalendar <kleag@free.fr>");

KComponentData kgraphviewerPartFactory::s_instance(s_about);

kgraphviewerPartFactory::kgraphviewerPartFactory()
    : KParts::Factory()
{
}

kgraphviewerPartFactory::~kgraphviewerPartFactory()
{
    delete s_about;
}

KParts::Part* kgraphviewerPartFactory::createPartObject( QWidget *parentWidget, 
                                                        QObject *parent, 
                                                        const char *classname, 
                                                        const QStringList &args )
{
  Q_UNUSED(classname);
  Q_UNUSED(args);
//     Create an instance of our Part
    kgraphviewerPart* obj = new kgraphviewerPart( parentWidget, parent);

//     See if we are to be read-write or not
//     if (QCString(classname) == "KParts::ReadOnlyPart")
//         obj->setReadWrite(false);

    return obj;
}

KComponentData kgraphviewerPartFactory::componentData()
{
/*    if( !s_instance )
    {
        s_about = new KAboutData( "kgraphviewerpart", I18N_NOOP("kgraphviewerPart"),
                    "1.0", I18N_NOOP( "GraphViz dot files viewer" ),
                    KAboutData::License_GPL,
                    "(c) 2005-2006, Gaël de Chalendar <kleag@free.fr>");
        s_instance(s_about);
    }*/
    return s_instance;
}

#include "kgraphviewer_part.moc"
