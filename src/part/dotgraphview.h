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

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#ifndef DOTGRAPHVIEW_H
#define DOTGRAPHVIEW_H

#include <kconfig.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>

#include <QGraphicsView>
#include <QSet>

#include <graphviz/types.h>

#include "kgraphviewer_export.h"
#include "kgraphviewer_interface.h"

class KSelectAction;
class KToggleAction;

class QMenu;
class QKeyEvent;
class QMouseEvent;
class QFocusEvent;
class QResizeEvent;
class QWheelEvent;
class QContextMenuEvent;
class QWidget;

namespace KGraphViewer
{

class GraphElement;
class GraphSubgraph;
class CanvasElement;
class CanvasEdge;
class PannerView;
class DotGraph;
class KGVSimplePrintingCommand;

#define DEFAULT_DETAILLEVEL 1

class DotGraphViewPrivate;
/**
 * A CanvasView showing a part of the call graph
 * and another zoomed out CanvasView in a border acting as
 * a panner to select to visible part (only if needed)
 */
class KGRAPHVIEWER_EXPORT DotGraphView: public QGraphicsView
{
 Q_OBJECT

public:
  enum EditingMode { None, AddNewElement, AddNewEdge, DrawNewEdge, SelectingElements };
  enum ScrollDirection { Here, Left, Right, Top, Bottom };
  
  explicit KGRAPHVIEWER_EXPORT DotGraphView(KActionCollection* actions, QWidget* parent=0);
  virtual ~DotGraphView();

  bool KGRAPHVIEWER_EXPORT loadDot(const QString& dotFileName);
  bool KGRAPHVIEWER_EXPORT loadLibrary(const QString& dotFileName);
  bool loadLibrary(graph_t* graph, const QString& layoutCommand = "dot");

  void readViewConfig();
  void saveViewConfig();

  QWidget* widget() { return this; }

  //TODO: rename zoomPos -> bev / panner, but _please_ make it consistent...
  KGraphViewerInterface::PannerPosition zoomPos() const;
  static KGraphViewerInterface::PannerPosition zoomPos(const QString&);
  static QString zoomPosString(KGraphViewerInterface::PannerPosition);
  void setPannerEnabled(bool enabled);
  
  static KConfigGroup* configGroup(KConfig*, const QString& prefix, const QString& postfix);
  static void writeConfigEntry(KConfigGroup*, const char* pKey, const QString& value,
                               const char* def);
  static void writeConfigEntry(KConfigGroup*, const char* pKey,
                               int value, int def);
  static void writeConfigEntry(KConfigGroup*, const char* pKey,
                               bool value, bool def);
  static void writeConfigEntry(KConfigGroup*, const char* pKey,
                               double value, double def);

  virtual void wheelEvent(QWheelEvent* e);

  /// multiplies current zoom factor with @p factor
  void applyZoom(double factor);
  /// sets zoom factor to @p factor
  void setZoomFactor(double factor);
  
  void setLayoutCommand(const QString& command);
    
  const QString& dotFileName();

  void hideToolsWindows();
  double zoom() const;
  KSelectAction* bevPopup();

  DotGraph* graph();
  const DotGraph* graph() const;

  const GraphElement* defaultNewElement() const;
  QPixmap defaultNewElementPixmap() const;

  void setDefaultNewElement(GraphElement* elem);
  void setDefaultNewElementPixmap(const QPixmap& pm);

  void prepareAddNewElement(QMap<QString,QString> attribs);
  void prepareAddNewEdge(QMap<QString,QString> attribs);
  void prepareSelectElements();
  
  void createNewEdgeDraftFrom(CanvasElement* node);
  void finishNewEdgeTo(CanvasElement* node);

  EditingMode editingMode() const;

  void KGRAPHVIEWER_EXPORT setReadOnly();
  void KGRAPHVIEWER_EXPORT setReadWrite();
  bool isReadWrite() const;
  bool isReadOnly() const;
  
  void removeSelectedNodes();
  void removeSelectedEdges();
  void removeSelectedSubgraphs();
  void removeSelectedElements();
  
  bool highlighting();
  void setHighlighting(bool highlightingValue);

  // public so that the panner view can bubble through
  void contextMenuEvent(QContextMenuEvent*);

  void setBackgroundColor(const QColor& color);
  
Q_SIGNALS:
  void zoomed(double factor);
  void sigViewBevEnabledToggled(bool value);
  void sigViewBevActivated(int newPos);
  void graphLoaded();
  void newNodeAdded(const QString&);
  void newEdgeAdded(const QString&, const QString&);
  /** signals that the user has activated a remove edge command */
  void removeEdge(const QString&);
  /** signals that the user has activated a remove edge command */
  void removeNodeNamed(const QString&);
  /** signals that the user has activated a remove element command */
  void removeElement(const QString&);
  /** signals the content of the new selection */
  void selectionIs(const QList<QString>, const QPoint&);
  /** let the application tweak the created edge if necessary */
  void newEdgeFinished(
      const QString&, const QString&,
      const QMap<QString, QString>&);
  void contextMenuEvent(const QString&, const QPoint&);
  void hoverEnter(const QString&);
  void hoverLeave(const QString&);
  
public Q_SLOTS:
  void zoomIn();
  void zoomOut();  
  void zoomRectMovedTo(QPointF newZoomPos);
  void zoomRectMoveFinished();
  bool initEmpty();
  bool slotLoadLibrary(graph_t* graph);
  bool reload();
  void dirty(const QString& dotFileName);
  void pageSetup();
  void print();
  void printPreview();
  void viewBevActivated(int newPos);
  void slotExportImage();
  void slotSelectLayoutAlgo(const QString& text);
  void slotLayoutSpecify();
  void slotLayoutReset();
  void slotSelectLayoutDot();
  void slotSelectLayoutNeato();
  void slotSelectLayoutTwopi();
  void slotSelectLayoutFdp();
  void slotSelectLayoutCirco();
  void slotBevToggled();
  void slotBevTopLeft();
  void slotBevTopRight();
  void slotBevBottomLeft();
  void slotBevBottomRight();
  void slotBevAutomatic();
  void slotUpdate();
  bool displayGraph();
  void slotEdgeSelected(CanvasEdge*, Qt::KeyboardModifiers);
  void slotElementSelected(CanvasElement*, Qt::KeyboardModifiers);
  void slotSelectionChanged();
  void slotContextMenuEvent(const QString&, const QPoint&);
  void slotElementHoverEnter(CanvasElement*);
  void slotElementHoverLeave(CanvasElement*);
  void slotElementHoverEnter(CanvasEdge*);
  void slotElementHoverLeave(CanvasEdge*);
  void slotSelectNode(const QString& nodeName);
  void centerOnNode(const QString& nodeId);

protected:
  void scrollContentsBy(int dx, int dy);
  void resizeEvent(QResizeEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent*);
  void focusInEvent(QFocusEvent*);
  void focusOutEvent(QFocusEvent*);
  
  void timerEvent ( QTimerEvent * event );
  void leaveEvent ( QEvent * event );
  void enterEvent ( QEvent * event );
  
private Q_SLOTS:
  void slotAGraphReadFinished();
  void slotAGraphLayoutFinished();
  
protected:
  DotGraphViewPrivate * const d_ptr;

private:
  Q_DECLARE_PRIVATE(DotGraphView);
  
};

}

#endif // DOTGRAPHVIEW_H
