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
#include "dotgraphview.h"
#include "dotgraph.h"

#include <KDirWatch>
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

#include <iostream>

#include <graphviz/gvc.h>


// #include "kgraphviewersettings.h"
#include "kgraphviewer_partsettings.h"

namespace KGraphViewer
{

class KGraphViewerPartPrivate
{
public:
  KGraphViewerPartPrivate() : m_watch(new KDirWatch()), m_layoutMethod(KGraphViewerInterface::InternalLibrary)
  {
    
  }
  
  ~KGraphViewerPartPrivate()
  {
    delete m_watch;
  }
  
  DotGraphView *m_widget;
  KDirWatch* m_watch;
  KGraphViewerPart::LayoutMethod m_layoutMethod;
  
};

KGraphViewerPart::KGraphViewerPart( QWidget *parentWidget, QObject *parent)
: KParts::ReadOnlyPart(parent), d(new KGraphViewerPartPrivate())
{
  kDebug() ;
  // we need an instance
  setComponentData( KGraphViewerPartFactory::componentData() );

  // this should be your custom internal widget
  d->m_widget = new DotGraphView( actionCollection(), parentWidget);
  d->m_widget->initEmpty();
  d->m_widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  connect( d->m_widget, SIGNAL( graphLoaded() ),
           this, SIGNAL( graphLoaded() ) );
  connect( d->m_widget, SIGNAL( newEdgeAdded(const QString&, const QString&) ),
          this, SIGNAL( newEdgeAdded(const QString&, const QString&) ) );
  connect( d->m_widget, SIGNAL( newNodeAdded(const QString&) ),
          this, SIGNAL( newNodeAdded(const QString&) ) );
  connect( d->m_widget, SIGNAL( removeEdge(const QString&) ),
           this, SIGNAL( removeEdge(const QString&) ) );
  connect( d->m_widget, SIGNAL( removeElement(const QString&) ),
           this, SIGNAL( removeElement(const QString&) ) );
  connect( d->m_widget, SIGNAL( selectionIs(const QList<QString>, const QPoint&) ),
           this, SIGNAL( selectionIs(const QList<QString>, const QPoint&) ) );
  connect( d->m_widget,
           SIGNAL( contextMenuEvent(const QString&, const QPoint&) ),
           this,
           SIGNAL( contextMenuEvent(const QString&, const QPoint&) ) );
  connect( d->m_widget,
           SIGNAL( newEdgeFinished(const QString&, const QString&, const QMap<QString, QString>&) ),
          this,
           SIGNAL( newEdgeFinished(const QString&, const QString&, const QMap<QString, QString>&) ) );
  connect( d->m_widget, SIGNAL( hoverEnter(const QString&)) ,
          this, SIGNAL( hoverEnter(const QString&) ) );
  connect( d->m_widget, SIGNAL( hoverLeave(const QString&)) ,
          this, SIGNAL( hoverLeave(const QString&)) );
                   

          
           
  // notify the part that this is our internal widget
  setWidget(d->m_widget);

  QAction* closeAct = actionCollection()->addAction( KStandardAction::Close, "file_close", this, SLOT( slotClose() ) );
  closeAct->setWhatsThis(i18n("Closes the current tab"));

  QAction* printAct = actionCollection()->addAction(KStandardAction::Print, "file_print", d->m_widget, SLOT(print()));
  printAct->setWhatsThis(i18n("Print the graph using current page setup settings"));
  printAct->setShortcut(Qt::ControlModifier + Qt::Key_P);
  
  QAction* printPreviewAct = actionCollection()->addAction(KStandardAction::PrintPreview, "file_print_preview", d->m_widget, SLOT(printPreview()));
  printPreviewAct->setWhatsThis(i18n("Open the print preview window"));
  printPreviewAct->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_P);
  
//   KAction* pagesetupAct = new KAction(i18n("&Page setup"), this); //actionCollection(), "file_page_setup");
  QAction* pagesetupAct = actionCollection()->addAction("file_page_setup", d->m_widget, SLOT(pageSetup()));
  pagesetupAct->setText(i18n("Page setup"));
  pagesetupAct->setWhatsThis(i18n("Opens the Page Setup dialog to allow graph printing to be setup"));

  QAction* redisplayAct = actionCollection()->addAction(KStandardAction::Redisplay, "view_redisplay", d->m_widget, SLOT(reload()));
  redisplayAct->setWhatsThis(i18n("Reload the current graph from file"));
  redisplayAct->setShortcut(Qt::Key_F5);

  QAction* zoomInAct = actionCollection()->addAction(KStandardAction::ZoomIn, "view_zoom_in", d->m_widget, SLOT(zoomIn()));
  // xgettext: no-c-format
  zoomInAct->setWhatsThis(i18n("Zoom in by 10% on the currently viewed graph"));
  zoomInAct->setShortcut(Qt::Key_F7);
  
  QAction* zoomOutAct = actionCollection()->addAction(KStandardAction::ZoomOut, "view_zoom_out", d->m_widget, SLOT(zoomOut()));
  // xgettext: no-c-format
  zoomOutAct->setWhatsThis(i18n("Zoom out by 10% from the currently viewed graph"));
  zoomOutAct->setShortcut(Qt::Key_F8);

// set our XML-UI resource file
  setXMLFile("kgraphviewer_part.rc");
}

/*DotGraph* KGraphViewerPart::graph()
{
  return d->m_widget->graph();
}

const DotGraph* KGraphViewerPart::graph() const
{
  return d->m_widget->graph();
}
*/

QList<QString> KGraphViewerPart::nodesIdsPrivate()
{
  QList<QString> result;
  
  return result;
}

QMap<QString,QString> KGraphViewerPart::nodeAtributesPrivate(const QString& nodeId)
{
  QMap<QString,QString> result;
  return result;
}

void KGraphViewerPart::slotClose()
{
  kDebug();
  emit close();
}

bool KGraphViewerPart::closeUrl()
{
  return d->m_widget->initEmpty();
}

bool KGraphViewerPart::slotLoadLibrary(graph_t* graph) 	 
{ 	 
  kDebug(); 	 
  bool res = d->m_widget->slotLoadLibrary(graph); 	 
  if (res) 	 
    d->m_widget->show(); 	 
  return res; 	 
}

KGraphViewerPart::~KGraphViewerPart()
{
  delete d;
}

bool KGraphViewerPart::openFile()
{
  kDebug() << " " << localFilePath();
  //    d->m_widget->loadedDot( localFilePath() );
  switch (d->m_layoutMethod)
  {
    case ExternalProgram:
      if (!d->m_widget->loadDot(localFilePath()))
        return false;
      break;
    case InternalLibrary:
      if (!d->m_widget->loadLibrary(localFilePath()))
        return false;
      break;
    default:
      kError() << "Unsupported layout method " << d->m_layoutMethod;
  }
  
  // deletes the existing file watcher because we have no way know here the name of the
  // previously watched file and thus we cannot use removeFile... :-(
  delete d->m_watch;
  d->m_watch = new KDirWatch();
  
  //   kDebug() << "Watching file " << localFilePath();
  d->m_watch->addFile(localFilePath());
  connect(d->m_watch, SIGNAL(dirty(const QString &)), d->m_widget, SLOT(dirty(const QString&)));
  QString label = localFilePath().section('/',-1,-1);
  
  d->m_widget->show();
  return true;
}

void KGraphViewerPart::slotHide(KParts::Part* part)
{
  if (part == this)
  {
    d->m_widget->hideToolsWindows();
  }
}

void KGraphViewerPart::slotUpdate()
{
  d->m_widget->slotUpdate();
}

void KGraphViewerPart::prepareAddNewElement(QMap<QString,QString> attribs)
{
  d->m_widget->prepareAddNewElement(attribs);
}

void KGraphViewerPart::slotSetGraphAttributes(QMap<QString,QString> attribs)
{
  d->m_widget->graph()->setGraphAttributes(attribs);
}

void KGraphViewerPart::slotAddNewNode(QMap<QString,QString> attribs)
{
  d->m_widget->graph()->addNewNode(attribs);
}

void KGraphViewerPart::slotAddNewSubgraph(QMap<QString,QString> attribs)
{
  d->m_widget->graph()->addNewSubgraph(attribs);
}

void KGraphViewerPart::slotAddNewNodeToSubgraph(QMap<QString,QString> attribs,
    QString subgraph)
{
  d->m_widget->graph()->addNewNodeToSubgraph(attribs, subgraph);
}

void KGraphViewerPart::slotAddExistingNodeToSubgraph(QMap<QString,QString> attribs,QString subgraph)
{
  d->m_widget->graph()->addExistingNodeToSubgraph(attribs, subgraph);
}

void KGraphViewerPart::slotMoveExistingNodeToMainGraph(QMap<QString,QString> attribs)
{
  d->m_widget->graph()->moveExistingNodeToMainGraph(attribs);
}

void KGraphViewerPart::slotAddNewEdge(QString src, QString tgt, QMap<QString,QString> attribs)
{
  d->m_widget->graph()->addNewEdge(src,tgt,attribs);
}

void KGraphViewerPart::prepareAddNewEdge(QMap<QString,QString> attribs)
{
  d->m_widget->prepareAddNewEdge(attribs);
}

void KGraphViewerPart::setReadOnly()
{
  d->m_widget->setReadOnly();
}

void KGraphViewerPart::setReadWrite()
{
  d->m_widget->setReadWrite();
}

void KGraphViewerPart::saveTo(const QString& fileName)
{
  d->m_widget->graph()->saveTo(fileName);
}

void KGraphViewerPart::slotRemoveNode(const QString& nodeName)
{
  d->m_widget->graph()->removeNodeNamed(nodeName);
}

void KGraphViewerPart::slotRemoveNodeFromSubgraph(const QString& nodeName, const QString& subgraphName)
{
  d->m_widget->graph()->removeNodeFromSubgraph(nodeName, subgraphName);
}

void KGraphViewerPart::slotRemoveSubgraph(const QString& subgraphName)
{
  d->m_widget->graph()->removeSubgraphNamed(subgraphName);
}

void KGraphViewerPart::slotSelectNode(const QString& nodeName)
{
  d->m_widget->slotSelectNode(nodeName);
}

void KGraphViewerPart::slotAddAttribute(const QString&)
{
  kDebug() << "NOT IMPLEMENTED";
}

void KGraphViewerPart::slotSetAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue)
{
  d->m_widget->graph()->setAttribute(elementId,attributeName,attributeValue);
}

void KGraphViewerPart::slotRemoveAttribute(const QString& nodeName, const QString& attribName)
{
  d->m_widget->graph()->removeAttribute(nodeName, attribName);
}

void KGraphViewerPart::slotRemoveEdge(const QString& id)
{
  d->m_widget->graph()->removeEdge(id);
}

void KGraphViewerPart::slotRemoveElement(const QString& id)
{
  d->m_widget->graph()->removeElement(id);
}

void KGraphViewerPart::slotSetHighlighting(bool highlightingValue)
{
  d->m_widget->setHighlighting(highlightingValue);
}


void KGraphViewerPart::slotPrepareToSelect()
{
  d->m_widget->prepareSelectElements();
}

void KGraphViewerPart::slotSetCursor(const QCursor& cursor)
{
  d->m_widget->setCursor(cursor);
}

void KGraphViewerPart::slotUnsetCursor()
{
  d->m_widget->unsetCursor();
}

void KGraphViewerPart::slotSetLayoutMethod(LayoutMethod method)
{
  setLayoutMethod(method);
}

void KGraphViewerPart::setLayoutMethod(LayoutMethod method)
{
  d->m_layoutMethod = method;
}

void KGraphViewerPart::centerOnNode(const QString& nodeId)
{
  d->m_widget->centerOnNode(nodeId);
}

void KGraphViewerPart::selectNode(const QString& nodeId)
{
  slotSelectNode(nodeId);
}

void KGraphViewerPart::setLayoutCommand(const QString& command)
{
  d->m_widget->setLayoutCommand(command);
}

void KGraphViewerPart::setPannerPosition(KGraphViewerInterface::PannerPosition position)
{
  d->m_widget->viewBevActivated(position);
}

void KGraphViewerPart::setPannerEnabled(bool enabled)
{
  d->m_widget->setPannerEnabled(enabled);
}

void KGraphViewerPart::setZoomFactor(double factor)
{
  d->m_widget->setZoomFactor(factor);
}

void KGraphViewerPart::zoomBy(double factor)
{
  d->m_widget->applyZoom(factor);
}

void KGraphViewerPart::zoomIn()
{
  d->m_widget->zoomIn();
}

void KGraphViewerPart::zoomOut()
{
  d->m_widget->zoomOut();
}

void KGraphViewerPart::slotRenameNode(const QString& oldNodeName, const QString& newNodeName)
{
  d->m_widget->graph()->renameNode(oldNodeName,newNodeName);
}

extern "C"
{
  /**
  * This function is the 'main' function of this part.  It takes
  * the form 'void *init_lib<library name>()  It always returns a
  * new factory object
  */
  KDE_EXPORT void *init_kgraphviewerpart()
  {
    return new KGraphViewerPartFactory();
  }
}

// KComponentData KGraphViewerPartFactory::s_instance = 0L;
KAboutData* KGraphViewerPartFactory::s_about = new KAboutData(
"kgraphviewerpart", 0, ki18n("KGraphViewerPart"),
"1.0", ki18n( "GraphViz dot files viewer" ),
KAboutData::License_GPL,
ki18n("(c) 2005-2006, Gaël de Chalendar &lt;kleag@free.fr&gt;"));

KComponentData KGraphViewerPartFactory::s_instance(s_about);

KGraphViewerPartFactory::KGraphViewerPartFactory(QObject* parent)
: KParts::Factory(parent)
{
}

KGraphViewerPartFactory::~KGraphViewerPartFactory()
{
gvFreeContext(gvContext());
delete s_about;
}

KParts::Part* KGraphViewerPartFactory::createPartObject( QWidget *parentWidget,
QObject *parent,
const char *classname,
const QStringList &args )
{
  Q_UNUSED(classname);
  Q_UNUSED(args);
  //     Create an instance of our Part
  KGraphViewerPart* obj = new KGraphViewerPart( parentWidget, parent);

  //     See if we are to be read-write or not
  //     if (QCString(classname) == "KParts::ReadOnlyPart")
  //         obj->setReadWrite(false);

  return obj;
}

KComponentData KGraphViewerPartFactory::componentData()
{
  /*    if( !s_instance )
  {
    s_about = new KAboutData( "kgraphviewerpart", 0, ki18n("KGraphViewerPart"),
                              "1.0", ki18n( "GraphViz dot files viewer" ),
                              KAboutData::License_GPL,
                              ki18n("(c) 2005-2006, Gaël de Chalendar <kleag@free.fr>"));
                              s_instance(s_about);
  }*/
  return s_instance;
}
                                                                                                                       
}

#include "kgraphviewer_part.moc"
