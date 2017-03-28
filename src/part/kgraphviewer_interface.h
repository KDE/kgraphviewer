// interface.h
// Copyright (C)  2002  Dominique Devriese <devriese@kde.org>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#ifndef KGRAPHVIEWER_INTERFACE_H
#define KGRAPHVIEWER_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtGui/QCursor>

#include <graphviz/gvc.h>

namespace KParts
{
  class Part;
}


namespace KGraphViewer
{
/**
 * KGraphViewerInterface is an interface implemented by KGraphViewer to
 * allow developers access to the KGraphViewerPart in ways that are not
 * possible through the normal KPart interface.
 *
 * Note that besides the functions below here, KGraphViewer also has
 * some signals you can connect to.  They aren't in this class cause
 * we can't have signals without having a QObject, which
 * KGraphViewerInterface is not. To see a list of signals, take a look at kgraphviewer_part.h
 * 
 * See the example code below for how to connect to these..
 *
 * Use it like this:
 * \code
 *  // fetch the Part..
 *  KPluginFactory *factory = KPluginLoader("kgraphviewerpart").factory();
 *  if (factory) {
 *      KParts::ReadOnlyPart* part = factory->create<KParts::ReadOnlyPart>("kgraphviewerpart", this);
 *
 *      // cast the part to the KGraphViewerInterface...
 *      KGraphViewerInterface* graph = qobject_cast<KGraphViewerInterface*>( part );
 *      if( ! graph )
 *      {
 *        // This should not happen
 *        return;
 *      }
 *      // now use the interface in all sorts of ways...
 *  }
 * \endcode
 *
 * @author Milian Wolff <mail@milianw.de>
 * 
 * 
 * WARNING: This is highly experimental and no kind of BC guarantees are given!
 * TODO: documentation
 */
class KGraphViewerInterface
{
public:
  enum LayoutMethod
  {
    ExternalProgram,
    InternalLibrary
  };

  virtual void setLayoutMethod(LayoutMethod method) = 0;
  virtual void zoomIn() = 0;
  virtual void zoomOut() = 0;
  virtual void zoomBy(double factor) = 0;
  virtual void setZoomFactor(double factor) = 0;

  enum PannerPosition { TopLeft, TopRight, BottomLeft, BottomRight, Auto };
  virtual void setPannerPosition(PannerPosition position) = 0;
  virtual void setPannerEnabled(bool enabled) = 0;

  virtual void setLayoutCommand(const QString& command) = 0;

  virtual void selectNode(const QString& nodeId) = 0;
  virtual void centerOnNode(const QString& nodeId) = 0;

  // Slots
  virtual void slotHide(KParts::Part* part) = 0;
  virtual void slotUpdate() = 0;
  virtual void prepareAddNewElement(QMap<QString,QString> attribs) = 0;
  virtual void prepareAddNewEdge(QMap<QString,QString> attribs) = 0;
  virtual void setReadOnly() = 0;
  virtual void setReadWrite() = 0;
  virtual void saveTo(const QString& fileName) = 0;
  virtual void slotRemoveNode(const QString&) = 0;
  virtual void slotRemoveNodeFromSubgraph(const QString& nodeName, const QString& subgraphName) = 0;
  virtual void slotRemoveSubgraph(const QString&) = 0;
  virtual void slotAddAttribute(const QString&) = 0;
  virtual void slotSetAttribute(const QString& elementId, const QString& attributeName, const QString& attributeValue) = 0;
  virtual void slotRemoveAttribute(const QString&,const QString&) = 0;
  virtual void slotSetGraphAttributes(QMap<QString,QString> attribs) = 0;
  virtual void slotAddNewNode(QMap<QString,QString> attribs) = 0;
  virtual void slotAddNewNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph) = 0;
  virtual void slotAddExistingNodeToSubgraph(QMap<QString,QString> attribs, QString subgraph) = 0;
  virtual void slotMoveExistingNodeToMainGraph(QMap<QString,QString>) = 0;
  virtual void slotAddNewSubgraph(QMap<QString,QString> attribs) = 0;
  virtual void slotAddNewEdge(QString src, QString tgt, QMap<QString,QString> attribs) = 0;
  virtual void slotRemoveEdge(const QString& id) = 0;
  virtual void slotRemoveElement(const QString& id) = 0;
  virtual void slotClose() = 0;
  virtual void slotSelectNode(const QString&) = 0;
  virtual void slotSetHighlighting(bool highlightingValue) = 0;
  virtual void slotPrepareToSelect() = 0;
  virtual void slotSetCursor(const QCursor& cursor) = 0;
  virtual void slotUnsetCursor() = 0;
  virtual void slotSetLayoutMethod(LayoutMethod method) = 0;
  virtual void slotRenameNode(const QString& oldName, const QString& newName) = 0;
  virtual bool slotLoadLibrary(graph_t* graph) = 0;
  virtual void setBackgroundColor(const QColor& color) = 0;
  

protected:
  KGraphViewerInterface() {}
  ~KGraphViewerInterface() {}
};

}

Q_DECLARE_INTERFACE(KGraphViewer::KGraphViewerInterface, "org.kde.KGraphViewerInterface")

#endif // KGRAPHVIEWER_INTERFACE_H
