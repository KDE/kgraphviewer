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

#include <QUuid>

#include <iostream>

// #include "kgraphviewersettings.h"
#include "kgraphviewer_partsettings.h"

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
  connect( m_widget, SIGNAL( newEdgeAdded(const QString&, const QString&) ),
          this, SIGNAL( newEdgeAdded(const QString&, const QString&) ) );
  connect( m_widget, SIGNAL( newNodeAdded(const QString&) ),
          this, SIGNAL( newNodeAdded(const QString&) ) );
  connect( m_widget, SIGNAL( removeEdge(const QString&) ),
           this, SIGNAL( removeEdge(const QString&) ) );
  connect( m_widget, SIGNAL( removeElement(const QString&) ),
           this, SIGNAL( removeElement(const QString&) ) );
  connect( m_widget, SIGNAL( selectionIs(const QList<QString>, const QPoint&) ),
           this, SIGNAL( selectionIs(const QList<QString>, const QPoint&) ) );
  connect( m_widget,
           SIGNAL( contextMenuEvent(const QString&, const QPoint&) ),
           this,
           SIGNAL( contextMenuEvent(const QString&, const QPoint&) ) );
  connect( m_widget, SIGNAL( newEdgeFinished(
      const QString&, const QString&,
      const QMap<QString, QString>&) ),
          this, SIGNAL( newEdgeFinished(
      const QString&, const QString&,
      const QMap<QString, QString>&) ) );
                    
  // notify the part that this is our internal widget
  setWidget(m_widget);

  QAction* closeAct = actionCollection()->addAction( KStandardAction::Close, "file_close", this, SLOT( slotClose() ) );
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

/*  KAction *updateAct = new KAction("view_update", i18n("&Update view"), actionCollection(), "update");
  updateAct->setShortcut(Qt::CTRL+Qt::Key_U);
  updateAct->setWhatsThis(i18n("Update the view of the current graph by running dot"));
  connect(updateAct, SIGNAL(triggered(bool)), m_widget, SLOT(slotUpdate()));*/
  
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

void kgraphviewerPart::slotClose()
{
  kDebug();
  emit close();
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
//   m_widget->graph()->nodes().insert(newNode->id(), newNode);
  m_widget->graph()->subgraphs()[subgraph]->content().push_back(newNode);

  kDebug() << "node added as" << newNode->id() << "in" << subgraph;
}

void kgraphviewerPart::slotAddExistingNodeToSubgraph(
    QMap<QString,QString> attribs,
    QString subgraph)
{
  kDebug() << attribs << "to" << subgraph;
  GraphNode* node = dynamic_cast<GraphNode*>(m_widget->graph()->elementNamed(attribs["id"]));
  if (node == 0)
  {
    kError() << "No such node" << attribs["id"];
    return;
  }
  if (m_widget->graph()->nodes().contains(attribs["id"]))
  {
    m_widget->graph()->nodes().remove(attribs["id"]);
    node->attributes() = attribs;
    m_widget->graph()->subgraphs()[subgraph]->content().push_back(node);
    kDebug() << "node " << node->id() << " added in " << subgraph;
  }
  else
  {
    foreach(GraphSubgraph* gs, m_widget->graph()->subgraphs())
    {
      GraphElement* elt = 0;
      foreach(GraphElement* element, gs->content())
      {
        if (element == node)
        {
          elt = element;
          break;
        }
      }
      if (elt != 0)
      {
        kDebug() << "removing node " << elt->id() << " from " << gs->id();
        gs->removeElement(elt);
        m_widget->graph()->subgraphs()[subgraph]->content().push_back(elt);
        kDebug() << "node " << elt->id() << " added in " << subgraph;
        break;
      }
    }
  }
}

void kgraphviewerPart::slotMoveExistingNodeToMainGraph(QMap<QString,QString> attribs)
{
  kDebug() << attribs;
  GraphNode* node = dynamic_cast<GraphNode*>(m_widget->graph()->elementNamed(attribs["id"]));
  if (node == 0)
  {
    kError() << "No such node" << attribs["id"];
    return;
  }
  else if (m_widget->graph()->nodes().contains(attribs["id"]))
  {
    kError() << "Node" << attribs["id"] << "already in main graph";
    return;
  }
  else
  {
    foreach(GraphSubgraph* gs, m_widget->graph()->subgraphs())
    {
      bool found = false;
      foreach(GraphElement* element, gs->content())
      {
        if (element == node)
        {
          found = true;
          break;
        }
      }
      if (found)
      {
        kDebug() << "removing node " << node->id() << " from " << gs->id();
        gs->removeElement(node);
        m_widget->graph()->nodes()[node->id()] = node;
        kDebug() << "node " << node->id() << " moved to main graph";
        break;
      }
    }
  }
}

void kgraphviewerPart::slotAddNewEdge(QString src, QString tgt,
            QMap<QString,QString> attribs)
{
  kDebug() << src << tgt << attribs;
  GraphEdge* newEdge = new GraphEdge();
  newEdge->attributes() = attribs;
  GraphElement* srcElement = graph()->elementNamed(src);
  if (srcElement == 0)
  {
    srcElement = graph()->elementNamed(QString("cluster_")+src);
  }
  GraphElement* tgtElement = graph()->elementNamed(tgt);
  if (tgtElement == 0)
  {
    tgtElement = graph()->elementNamed(QString("cluster_")+tgt);
  }
  
  if (srcElement == 0 || tgtElement == 0)
  {
    kError() << src << "or" << tgt << "missing";
    return;
  }
  if (attribs.keys().contains("id"))
  {
    newEdge->setId(attribs["id"]);
  }
  else
  {
    newEdge->setId(src+tgt+QUuid::createUuid().toString().remove("{").remove("}").remove("-"));
  }
  newEdge->setFromNode(srcElement);
  newEdge->setToNode(tgtElement);
  m_widget->graph()->edges().insert(newEdge->id(), newEdge);
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
  kDebug() << nodeName;
  m_widget->graph()->removeNodeNamed(nodeName);
}

void kgraphviewerPart::slotRemoveNodeFromSubgraph(const QString& nodeName, const QString& subgraphName)
{
  kDebug() << nodeName << subgraphName;
  m_widget->graph()->removeNodeFromSubgraph(nodeName, subgraphName);
}

void kgraphviewerPart::slotRemoveSubgraph(const QString& subgraphName)
{
  kDebug() << subgraphName;
  m_widget->graph()->removeSubgraphNamed(subgraphName);
}

void kgraphviewerPart::slotSelectNode(const QString& nodeName)
{
  kDebug() << nodeName;
  GraphNode* node = dynamic_cast<GraphNode*>(m_widget->graph()->elementNamed(nodeName));
  if (node == 0) return;
  node->setSelected(true);
  if (node->canvasNode()!=0)
  {
    node->canvasNode()->modelChanged();
    m_widget->slotElementSelected(node->canvasNode(),Qt::NoModifier);
  }
}

void kgraphviewerPart::slotAddAttribute(const QString&)
{
  kDebug();
}

void kgraphviewerPart::slotSetAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue)
{
  kDebug();
  m_widget->graph()->setAttribute(elementId,attributeName,attributeValue);
}

void kgraphviewerPart::slotRemoveAttribute(const QString& nodeName, const QString& attribName)
{
  kDebug();
  GraphElement* element = m_widget->graph()->elementNamed(nodeName);
  if (element != 0)
  {
    element->removeAttribute(attribName);
  }
}

void kgraphviewerPart::slotRemoveEdge(const QString& id)
{
  kDebug();
  m_widget->graph()->removeEdge(id);
}

void kgraphviewerPart::slotRemoveElement(const QString& id)
{
  kDebug();
  m_widget->graph()->removeElement(id);
}

void kgraphviewerPart::slotSetHighlighting(bool highlightingValue)
{
  kDebug();
  m_widget->setHighlighting(highlightingValue);
}


void kgraphviewerPart::slotPrepareToSelect()
{
  kDebug();
  m_widget->prepareSelectElements();
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
