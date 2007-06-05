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

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#ifndef CALLGRAPHVIEW_H
#define CALLGRAPHVIEW_H

#include <kconfig.h>

#include <qcanvas.h>
#include <qwidget.h>
#include <qmap.h>

#include <set>

#include "graphexporter.h"


class CanvasNode;
class GraphEdge;
class GraphNode;
class PannerView;
class DotGraph;
class KGVSimplePrintingCommand;
class CallGraphTip;


/**
 * A CanvasView showing a part of the call graph
 * and another zoomed out CanvasView in a border acting as
 * a panner to select to visible part (only if needed)
 */
class DotGraphView: public QCanvasView, public GraphOptions
{
 Q_OBJECT

public:
 enum ZoomPosition { TopLeft, TopRight, BottomLeft, BottomRight, Auto };

  /**
   * Export menu items
   */
  enum { EXPORT_TO_IMAGE };

  /**
   * Popup menu items
   */
  enum { 
      POPUP_BEV   = 100,
      POPUP_BEV_ENABLED,
      POPUP_BEV_TOPLEFT,
      POPUP_BEV_TOPRIGTH,
      POPUP_BEV_BOTTOMLEFT,
      POPUP_BEV_BOTTOMRIGHT,
      POPUP_BEV_AUTOMATIC,
      POPUP_EXPORT = 200,
      POPUP_EXPORT_IMAGE,
      POPUP_LAYOUT = 300,
      POPUP_LAYOUT_SPECIFY,
      POPUP_LAYOUT_RESET,
      POPUP_LAYOUT_DOT,
      POPUP_LAYOUT_NEATO,
      POPUP_LAYOUT_TWOPI,
      POPUP_LAYOUT_FDP,
      POPUP_LAYOUT_CIRCO
  };
  
  DotGraphView(QWidget* parent=0, const char* name=0);
  virtual ~DotGraphView();

  void readViewConfig();
  void saveViewConfig();

  QWidget* widget() { return this; }
  QString whatsThis() const;

  ZoomPosition zoomPos() const { return m_zoomPosition; }
  static ZoomPosition zoomPos(const QString&);
  static QString zoomPosString(ZoomPosition);
  
  static KConfigGroup* configGroup(KConfig*, const QString& prefix, const QString& postfix);
  static void writeConfigEntry(KConfigBase*, const char* pKey, const QString& value,
                               const char* def, bool bNLS = false);
  static void writeConfigEntry(KConfigBase*, const char* pKey,
                               int value, int def);
  static void writeConfigEntry(KConfigBase*, const char* pKey,
                               bool value, bool def);
  static void writeConfigEntry(KConfigBase*, const char* pKey,
                               double value, double def);
  
  virtual void wheelEvent(QWheelEvent* e);
    
  void zoomIn();
  void zoomOut();  
  void applyZoom(double factor);
  
  void setLayoutCommand(const QString& command);
    
  const QString& dotFileName();
    
  void hideToolsWindows();
  inline double zoom() const {return m_zoom;}
  
signals:
  void zoomed(double factor);
  void sigViewBevEnabledToggled(bool value);
  void sigViewBevActivated(int newPos);

public slots:
  void contentsMovingSlot(int,int);
  void zoomRectMoved(int,int);
  void zoomRectMoveFinished();
  bool loadDot(const QString& dotFileName);
  bool reload();
  void dirty(const QString& dotFileName);
	void pageSetup();
  void print();
  void printPreview();
  void viewBevEnabledToggled(bool value);
  void viewBevActivated(int newPos);
  void fileExportActivated(int value);

protected:
  void resizeEvent(QResizeEvent*);
  void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent(QMouseEvent*);
  void contentsMouseReleaseEvent(QMouseEvent*);
  void contentsMouseDoubleClickEvent(QMouseEvent*);
  void contentsContextMenuEvent(QContextMenuEvent*);
  void keyPressEvent(QKeyEvent*);
  void focusInEvent(QFocusEvent*);
  void focusOutEvent(QFocusEvent*);

private:
  void updateSizes(QSize s = QSize(0,0));
  void makeFrame(CanvasNode*, bool active);
  void setupPopup();
  void exportToImage();

  std::set<QCanvasText*> m_labelViews;
  QCanvas *m_canvas;
  QPopupMenu* m_popup;
  int m_xMargin, m_yMargin;
  PannerView *m_birdEyeView;
  double m_cvZoom;
  QSize m_canvasNaturalSize;
  double m_zoom;
  CallGraphTip* m_tip;
  bool m_isMoving;
  QPoint m_lastPos;

  GraphExporter m_exporter;

  // widget options
  ZoomPosition m_zoomPosition, m_lastAutoPosition;
  
  DotGraph* m_graph;
  
  CanvasNode* m_focusedNode;

  KGVSimplePrintingCommand* m_printCommand;
};




#endif



