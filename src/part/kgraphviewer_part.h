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

#ifndef _KGRAPHVIEWERPART_H_
#define _KGRAPHVIEWERPART_H_

#include <kparts/part.h>
#include <kparts/genericfactory.h>

#include <graphviz/gvc.h>
#include "kgraphviewer_interface.h"

class KComponentData;
class KAboutData;
class KDirWatch;

class QWidget;

namespace KGraphViewer
{

class DotGraph;
class DotGraphView;

class KGraphViewerPartPrivate;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Gael de Chalendar <kleag@free.fr>
 */
class KGraphViewerPart : public KParts::ReadOnlyPart, public KGraphViewerInterface
{
    Q_OBJECT
    Q_INTERFACES(KGraphViewer::KGraphViewerInterface)

//BEGIN: KGraphViewerInterface
public:
    virtual void setLayoutMethod(LayoutMethod method);
    virtual void centerOnNode(const QString& nodeId);
    virtual void selectNode(const QString& nodeId);
    virtual void setLayoutCommand(const QString& command);
    virtual void setPannerPosition(PannerPosition position);
    virtual void setPannerEnabled(bool enabled);
    virtual void zoomBy(double factor);
    virtual void setZoomFactor(double factor);
    virtual void zoomIn();
    virtual void zoomOut();

public:
    /**
     * Default constructor
     */
    KGraphViewerPart(QWidget *parentWidget, QObject *parent);

    
    /**
     * Destructor
     */
    virtual ~KGraphViewerPart();

  // Return information about the part
  static KAboutData* createAboutData();


Q_SIGNALS:
  void graphLoaded();
  void newNodeAdded(const QString&);
  void newEdgeAdded(const QString&, const QString&);
  /** signals that the user has activated a remove edge command */
  void removeEdge(const QString&);
  /** signals that the user has activated a remove element command */
  void removeElement(const QString&);
  void close();
  void selectionIs(const QList<QString>, const QPoint&);
  void contextMenuEvent(const QString&, const QPoint&);
  /** let the application tweak the created edge if necessary */
  void newEdgeFinished(
      const QString&, const QString&,
      const QMap<QString, QString>&);
  /// emited when the mouse enters a node, a subgraph or an edge. The parameter is the hovered element id
  void hoverEnter(const QString&);
  /// emited when the mouse leaves a node, a subgraph or an edge. The parameter is the hovered element id
  void hoverLeave(const QString&);

public slots:
  void slotHide(KParts::Part* part);
  void slotUpdate();
  void prepareAddNewElement(QMap<QString,QString> attribs);
  void prepareAddNewEdge(QMap<QString,QString> attribs);
  void setReadOnly();
  void setReadWrite();
  void saveTo(const QString& fileName);
  void slotRemoveNode(const QString&);
  void slotRemoveNodeFromSubgraph(
      const QString& nodeName,
      const QString& subgraphName);
  void slotRemoveSubgraph(const QString&);
  void slotAddAttribute(const QString&);
  void slotSetAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue);
  void slotRemoveAttribute(const QString&,const QString&);
  void slotSetGraphAttributes(QMap<QString,QString> attribs);
  void slotAddNewNode(QMap<QString,QString> attribs);
  void slotAddNewNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph);
  void slotAddExistingNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph);
  void slotMoveExistingNodeToMainGraph(QMap<QString,QString>);
  void slotAddNewSubgraph(QMap<QString,QString> attribs);
  void slotAddNewEdge(QString src, QString tgt, QMap<QString,QString> attribs);
  void slotRemoveEdge(const QString& id);
  void slotRemoveElement(const QString& id);
  void slotClose();
  void slotSelectNode(const QString&);
  void slotSetHighlighting(bool highlightingValue);
  void slotPrepareToSelect();
  void slotSetCursor(const QCursor& cursor);
  void slotUnsetCursor();
  virtual bool closeUrl();
  bool slotLoadLibrary(graph_t* graph);
  void slotSetLayoutMethod(LayoutMethod method);
  void slotRenameNode(const QString& oldNodeName, const QString& newNodeName);
  
  QList<QString> nodesIds()
  {
//     return nodesIdsPrivate();
  }
  
  QMap<QString,QString> nodeAtributes(const QString& nodeId)
  {
//     return nodeAtributesPrivate(nodeId);
  }
  
/*  inline DotGraph* graph() {return m_widget->graph();}
  inline const DotGraph* graph() const {return m_widget->graph();}*/
  
  
  protected:
    /**
     * This must be implemented by each part. Use openUrl to open a file
     */
    virtual bool openFile();
    
private:
  KGraphViewerPartPrivate * const d;

  QList<QString> nodesIdsPrivate();
  QMap<QString,QString> nodeAtributesPrivate(const QString& nodeId);
  
};

class KGraphViewerPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    KGraphViewerPartFactory(QObject* parent = 0);
    ~KGraphViewerPartFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget,
                                            QObject *parent,
                                            const char *classname, const QStringList &args );
    static KComponentData componentData();

private:
    static KComponentData s_instance;
    static KAboutData* s_about;
};

}
#endif // _KGRAPHVIEWERPART_H_
