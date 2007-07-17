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
#include <kactioncollection.h>
#include <kconfiggroup.h>

#include <QGraphicsView>
#include <QSet>

#include "graphexporter.h"


class CanvasNode;
class PannerView;
class DotGraph;
class KGVSimplePrintingCommand;
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

#define DEFAULT_DETAILLEVEL 1
/**
 * A CanvasView showing a part of the call graph
 * and another zoomed out CanvasView in a border acting as
 * a panner to select to visible part (only if needed)
 */
class DotGraphView: public QGraphicsView
{
 Q_OBJECT

public:
 enum ZoomPosition { TopLeft, TopRight, BottomLeft, BottomRight, Auto };

  DotGraphView(KActionCollection* actions, QWidget* parent=0);
  virtual ~DotGraphView();

  void readViewConfig();
  void saveViewConfig();

  QWidget* widget() { return this; }

  ZoomPosition zoomPos() const { return m_zoomPosition; }
  static ZoomPosition zoomPos(QString);
  static QString zoomPosString(ZoomPosition);
  
  static KConfigGroup* configGroup(KConfig*, QString prefix, QString postfix);
  static void writeConfigEntry(KConfigGroup*, const char* pKey, QString value,
                               const char* def);
  static void writeConfigEntry(KConfigGroup*, const char* pKey,
                               int value, int def);
  static void writeConfigEntry(KConfigGroup*, const char* pKey,
                               bool value, bool def);
  static void writeConfigEntry(KConfigGroup*, const char* pKey,
                               double value, double def);
  
  virtual void wheelEvent(QWheelEvent* e);
    
  void applyZoom(double factor);
  
  void setLayoutCommand(const QString& command);
    
  const QString& dotFileName();

  void hideToolsWindows();
  inline double zoom() const {return m_zoom;}
  inline KSelectAction* bevPopup() {return m_bevPopup;}

signals:
  void zoomed(double factor);
  void sigViewBevEnabledToggled(bool value);
  void sigViewBevActivated(int newPos);

public slots:
  void zoomIn();
  void zoomOut();  
  void zoomRectMovedTo(QPointF newZoomPos);
  void zoomRectMoveFinished();
  bool loadDot(const QString& dotFileName);
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
  void slotBevEnabled();
  void slotBevTopLeft();
  void slotBevTopRight();
  void slotBevBottomLeft();
  void slotBevBottomRight();
  void slotBevAutomatic();
  void slotUpdate();

protected:
  void resizeEvent(QResizeEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void contextMenuEvent(QContextMenuEvent*);
  void keyPressEvent(QKeyEvent*);
  void focusInEvent(QFocusEvent*);
  void focusOutEvent(QFocusEvent*);

private:
  void updateSizes(QSizeF s = QSizeF(0,0));
  void makeFrame(CanvasNode*, bool active);
  void setupPopup();
  void exportToImage();
  KActionCollection* actionCollection() {return m_actions;}
  
  QSet<QGraphicsSimpleTextItem*> m_labelViews;
  QGraphicsScene* m_canvas;
  QMenu* m_popup;
  KSelectAction* m_bevPopup;
  KSelectAction* m_layoutAlgoSelectAction;
  int m_xMargin, m_yMargin;
  PannerView *m_birdEyeView;
  double m_cvZoom;
  QSizeF m_canvasNaturalSize;
  double m_zoom;
  bool m_isMoving;
  QPoint m_lastPos;

  GraphExporter m_exporter;

  // widget options
  ZoomPosition m_zoomPosition, m_lastAutoPosition;
  
  DotGraph* m_graph;
  
  CanvasNode* m_focusedNode;

  KGVSimplePrintingCommand* m_printCommand;
  
  KToggleAction* m_bevEnabledAction;
  KActionCollection* m_actions;

  int m_detailLevel;
};




#endif



