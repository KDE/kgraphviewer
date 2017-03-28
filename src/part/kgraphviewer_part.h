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
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>

#include "kgraphviewer_interface.h"

class KComponentData;
class KAboutData;

class QWidget;

namespace KGraphViewer
{

class DotGraph;

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
    void setLayoutMethod(LayoutMethod method) override;
    void centerOnNode(const QString& nodeId) override;
    void selectNode(const QString& nodeId) override;
    void setLayoutCommand(const QString& command) override;
    void setPannerPosition(PannerPosition position) override;
    void setPannerEnabled(bool enabled) override;
    void zoomBy(double factor) override;
    void setZoomFactor(double factor) override;
    void zoomIn() override;
    void zoomOut() override;
    void setBackgroundColor(const QColor& color) override;

public:
    /**
     * Default constructor
     */
    KGraphViewerPart(QWidget *parentWidget, QObject *parent, const QVariantList &);

    
    /**
     * Destructor
     */
    ~KGraphViewerPart() override;

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
  /// emitted when the mouse enters a node, a subgraph or an edge. The parameter is the hovered element id
  void hoverEnter(const QString&);
  /// emitted when the mouse leaves a node, a subgraph or an edge. The parameter is the hovered element id
  void hoverLeave(const QString&);

public Q_SLOTS:
  void slotHide(KParts::Part* part) override;
  void slotUpdate() override;
  void prepareAddNewElement(QMap<QString,QString> attribs) override;
  void prepareAddNewEdge(QMap<QString,QString> attribs) override;
  void setReadOnly() override;
  void setReadWrite() override;
  void saveTo(const QString& fileName) override;
  void slotRemoveNode(const QString&) override;
  void slotRemoveNodeFromSubgraph(
      const QString& nodeName,
      const QString& subgraphName) override;
  void slotRemoveSubgraph(const QString&) override;
  void slotAddAttribute(const QString&) override;
  void slotSetAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue) override;
  void slotRemoveAttribute(const QString&,const QString&) override;
  void slotSetGraphAttributes(QMap<QString,QString> attribs) override;
  void slotAddNewNode(QMap<QString,QString> attribs) override;
  void slotAddNewNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph) override;
  void slotAddExistingNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph) override;
  void slotMoveExistingNodeToMainGraph(QMap<QString,QString>) override;
  void slotAddNewSubgraph(QMap<QString,QString> attribs) override;
  void slotAddNewEdge(QString src, QString tgt, QMap<QString,QString> attribs) override;
  void slotRemoveEdge(const QString& id) override;
  void slotRemoveElement(const QString& id) override;
  void slotClose() override;
  void slotSelectNode(const QString&) override;
  void slotSetHighlighting(bool highlightingValue) override;
  void slotPrepareToSelect() override;
  void slotSetCursor(const QCursor& cursor) override;
  void slotUnsetCursor() override;
  bool closeUrl() override;
  bool slotLoadLibrary(graph_t* graph) override;
  void slotSetLayoutMethod(LayoutMethod method) override;
  void slotRenameNode(const QString& oldNodeName, const QString& newNodeName) override;
  
/*  inline DotGraph* graph() {return m_widget->graph();}
  inline const DotGraph* graph() const {return m_widget->graph();}*/
  
  
  protected:
    /**
     * This must be implemented by each part. Use openUrl to open a file
     */
    bool openFile() override;

private:
  KGraphViewerPartPrivate * const d;
};

}
#endif // _KGRAPHVIEWERPART_H_
