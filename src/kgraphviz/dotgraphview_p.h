/*
    Copyright (C) 2010 Kevin Funk <krf@electrostorm.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef DOTGRAPHVIEW_P_H
#define DOTGRAPHVIEW_P_H

#include "dotgraphview.h"

#include "graphexporter.h"
#include "support/loadagraphthread.h"
#include "support/layoutagraphthread.h"

#include <QSet>

class QMenu;

class KToggleAction;

namespace KGraphViz
{

class PannerView;
class KGVSimplePrintingCommand;

class DotGraphViewPrivate
{

public:
  DotGraphViewPrivate(KActionCollection* actions, DotGraphView* parent);
  virtual ~DotGraphViewPrivate();

  void updateSizes(QSizeF s = QSizeF(0,0));
  void updateBirdEyeView();
  void setupCanvas();
  void setupPopup();
  void exportToImage();
  KActionCollection* actionCollection() {return m_actions;}
  int displaySubgraph(GraphSubgraph* gsubgraph, int zValue, CanvasElement* parent = 0);

  QSet<QGraphicsSimpleTextItem*> m_labelViews;
  QGraphicsScene* m_canvas;
  QMenu* m_popup;
  KSelectAction* m_bevPopup;
  KSelectAction* m_layoutAlgoSelectAction;
  int m_xMargin, m_yMargin;
  PannerView *m_birdEyeView;
  double m_cvZoom;
  double m_zoom;
  bool m_isMoving;
  QPoint m_lastPos;

  GraphExporter m_exporter;

  // widget options
  KGraphViewer::KGraphViewerInterface::PannerPosition m_zoomPosition, m_lastAutoPosition;

  DotGraph* m_graph;

  KGVSimplePrintingCommand* m_printCommand;

  KToggleAction* m_bevEnabledAction;
  KActionCollection* m_actions;

  int m_detailLevel;

  GraphElement* m_defaultNewElement;

  /** image used for a new node just added in an edited graph because this new node has
    *  still no attribute and thus no render operation */
  QPixmap m_defaultNewElementPixmap;
  DotGraphView::EditingMode m_editingMode;

  CanvasElement* m_newEdgeSource;
  QGraphicsLineItem* m_newEdgeDraft;

  bool m_readWrite;

  QMap<QString, QString> m_newElementAttributes;

  /// identifier of the timer started when the mouse leaves the view during
  /// edge drawing
  int m_leavedTimer;

  DotGraphView::ScrollDirection m_scrollDirection;

  QPoint m_pressPos;
  QPoint m_pressScrollBarsPos;

  /// true if elements should be highlighted on hover; false otherwise
  bool m_highlighting;

  /// A thread to load graphviz agraph files
  LoadAGraphThread m_loadThread;

  /// A thread to layout graphviz agraph files
  LayoutAGraphThread m_layoutThread;

  /// The graph background color
  QColor m_backgroundColor;

private:
  DotGraphView * const q_ptr;
  Q_DECLARE_PUBLIC(DotGraphView);
};

}

#endif
