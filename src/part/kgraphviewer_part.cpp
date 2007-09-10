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


#include "kgraphviewer_part.h"
#include "dotgraph.h"

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
  kDebug() ;
  // we need an instance
  setComponentData( kgraphviewerPartFactory::componentData() );

  // this should be your custom internal widget
  m_widget = new DotGraphView( actionCollection(), parentWidget);
  m_widget->initEmpty();
  m_widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  connect( m_widget, SIGNAL( graphLoaded() ),
           this, SIGNAL( graphLoaded() ) );

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

  QAction* updateAct = actionCollection()->addAction("view_update", m_widget, SLOT(slotUpdate()));
  updateAct->setWhatsThis(i18n("Update the view of the current graph by running dot"));
  updateAct->setShortcut(Qt::CTRL+Qt::Key_U);

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
  kDebug() << " " << localFilePath();
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

void kgraphviewerPart::slotUpdate()
{
  kDebug();
  m_widget->slotUpdate();
}

void kgraphviewerPart::prepareAddNewElement(QMap<QString,QString> attribs)
{
  m_widget->prepareAddNewElement(attribs);
}

void kgraphviewerPart::slotSetGraphAttributes(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  m_widget->graph()->attributes() = attribs;
}

void kgraphviewerPart::slotAddNewNode(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  GraphNode* newNode = new GraphNode();
  newNode->attributes() = attribs;
  m_widget->graph()->nodes().insert(newNode->id(), newNode);
  kDebug() << "node added as" << newNode->id();
}

void kgraphviewerPart::slotAddNewSubgraph(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  GraphSubgraph* newSG = new GraphSubgraph();
  newSG->attributes() = attribs;
  m_widget->graph()->subgraphs()[newSG->id()] = newSG;
  kDebug() << "subgraph added as" << newSG->id();
}

void kgraphviewerPart::slotAddNewNodeToSubgraph(QMap<QString,QString> attribs,
    QString subgraph)
{
  kDebug() << attribs << "to" << subgraph;
  GraphNode* newNode = new GraphNode();
  newNode->attributes() = attribs;
  m_widget->graph()->nodes().insert(newNode->id(), newNode);
  m_widget->graph()->subgraphs()[subgraph]->content().push_back(newNode);

  kDebug() << "node added as" << newNode->id();
}

void kgraphviewerPart::slotAddNewEdge(QString src, QString tgt,
            QMap<QString,QString> attribs)
{
  kDebug() << src << tgt << attribs;
  GraphEdge* newEdge = new GraphEdge();
  newEdge->setFromNode(m_widget->graph()->nodes()[src]);
  newEdge->setToNode(m_widget->graph()->nodes()[tgt]);
  newEdge->attributes() = attribs;
  m_widget->graph()->edges().insert(QPair<GraphNode*,GraphNode*>(newEdge->fromNode(),newEdge->toNode()), newEdge);
}

void kgraphviewerPart::prepareAddNewEdge(QMap<QString,QString> attribs)
{
  m_widget->prepareAddNewEdge(attribs);
}

void kgraphviewerPart::setReadOnly()
{
  kDebug() ;
  m_widget->setReadOnly();
}

void kgraphviewerPart::setReadWrite()
{
  kDebug() ;
  m_widget->setReadWrite();
}

void kgraphviewerPart::saveTo(const QString& fileName)
{
  kDebug() << fileName;
  m_widget->graph()->saveTo(fileName);
}

void kgraphviewerPart::slotRemoveNode(const QString& nodeName)
{
  kDebug();
  m_widget->graph()->removeNodeNamed(nodeName);
}

void kgraphviewerPart::slotAddAttribute(const QString&)
{
  kDebug();
}

void kgraphviewerPart::slotRemoveAttribute(const QString& nodeName, const QString& attribName)
{
  kDebug();
  m_widget->graph()->nodes()[nodeName]->removeAttribute(attribName);
}

/*It's usually safe to leave the factory code alone.. with the
notable exception of the KAboutData data*/
#include <kaboutdata.h>

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
    "kgraphviewerpart", 0, ki18n("kgraphviewerPart"),
    "1.0", ki18n( "GraphViz dot files viewer" ),
    KAboutData::License_GPL,
    ki18n("(c) 2005-2006, Gael de Chalendar &lt;kleag@free.fr&gt;"));

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
        s_about = new KAboutData( "kgraphviewerpart", 0, ki18n("kgraphviewerPart"),
                    "1.0", ki18n( "GraphViz dot files viewer" ),
                    KAboutData::License_GPL,
                    ki18n("(c) 2005-2006, Gaël de Chalendar <kleag@free.fr>"));
        s_instance(s_about);
    }*/
    return s_instance;
}

#include "kgraphviewer_part.moc"
