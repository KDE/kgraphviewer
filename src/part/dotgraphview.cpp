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

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#include "dotgraphview.h"

#include "dotgraph.h"
#include "graphelement.h"
#include "pannerview.h"
#include "canvassubgraph.h"
#include "canvasedge.h"
#include "dot2qtconsts.h"
#include "graphnode.h"
#include "canvasnode.h"
#include "graphedge.h"
#include "FontsCache.h"
#include "kgraphviewer_partsettings.h"
#include "simpleprintingcommand.h"
#include "graphexporter.h"
#include "loadagraphthread.h"
#include "layoutagraphthread.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <limits>

#include <graphviz/types.h>

#include <QMatrix>
#include <QPainter>
#include <QStyle>
#include <QImage>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QBitmap>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMenu>
#include <QGraphicsSimpleTextItem>
#include <QScrollBar>
#include <QUuid>

#include <kdebug.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kselectaction.h>
#include <ktoggleaction.h>
#include <kstandarddirs.h>
#include <kactionmenu.h>
    
// DotGraphView defaults

#define DEFAULT_ZOOMPOS      KGraphViewerInterface::Auto
#define KGV_MAX_PANNER_NODES 100

namespace KGraphViewer
{

class DotGraphViewPrivate
{

public:
  DotGraphViewPrivate(KActionCollection* actions, DotGraphView* parent) : 
   m_labelViews(),
    m_popup(0),
    m_zoom(1),
    m_isMoving(false),
    m_exporter(),
    m_zoomPosition(DEFAULT_ZOOMPOS),
    m_lastAutoPosition(KGraphViewerInterface::TopLeft),
    m_graph(0),
    m_printCommand(0),
    m_actions(actions),
    m_detailLevel(DEFAULT_DETAILLEVEL),
    m_defaultNewElement(0),
    m_defaultNewElementPixmap(KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newnode.png")),
    m_editingMode(DotGraphView::None),
    m_newEdgeSource(0),
    m_newEdgeDraft(0),
    m_readWrite(false),
    m_leavedTimer(std::numeric_limits<int>::max()),
    m_highlighting(false),
    m_loadThread(),
    m_layoutThread(),
    m_backgroundColor(QColor("white")),
    q_ptr( parent )
  {
    
  }
  virtual ~DotGraphViewPrivate()
  {
    delete m_birdEyeView;
    m_birdEyeView = 0;
    if (m_popup != 0)
    {
      delete m_popup;
    }
    if (m_canvas)
    {
      Q_Q(DotGraphView);
      q->setScene(0);
      delete m_canvas;
    }
    if (m_graph != 0)
    {
      delete m_graph;
    }
  }
  

  void updateSizes(QSizeF s = QSizeF(0,0));
  void updateBirdEyeView();
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
  KGraphViewerInterface::PannerPosition m_zoomPosition, m_lastAutoPosition;

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

  DotGraphView * const q_ptr;
  Q_DECLARE_PUBLIC(DotGraphView);
};
  
void DotGraphViewPrivate::updateSizes(QSizeF s)
{
  kDebug() ;
  Q_Q(DotGraphView);
  if (m_canvas == 0)
    return;
  if (s == QSizeF(0,0)) s = q->size();
  
  // the part of the canvas that should be visible
  qreal cWidth  = m_canvas->width()  - 2*m_xMargin + 100;
  qreal cHeight = m_canvas->height() - 2*m_yMargin + 100;
  
  // hide birds eye view if no overview needed
  if (//!_data || !_activeItem ||
      !KGraphViewerPartSettings::birdsEyeViewEnabled() ||
      (((cWidth * m_zoom) < s.width()) && (cHeight * m_zoom) < s.height()))
  {
    m_birdEyeView->hide();
    return;
  }
  m_birdEyeView->hide();
  
  // first, assume use of 1/3 of width/height (possible larger)
  double zoom = .33 * s.width() / cWidth;
  if (zoom * cHeight < .33 * s.height()) zoom = .33 * s.height() / cHeight;
  
  // fit to widget size
  if (cWidth  * zoom  > s.width())   zoom = s.width() / (double)cWidth;
  if (cHeight * zoom  > s.height())  zoom = s.height() / (double)cHeight;
  
  // scale to never use full height/width
  zoom = zoom * 3/4;
  
  // at most a zoom of 1/3
  if (zoom > .33) zoom = .33;
  
  if (zoom != m_cvZoom)
  {
    m_cvZoom = zoom;
    
    QMatrix wm;
    wm.scale( zoom, zoom );
    m_birdEyeView->setMatrix(wm);
    
    // make it a little bigger to compensate for widget frame
    m_birdEyeView->resize((cWidth * zoom) + 4,
                          (cHeight * zoom) + 4);
    
  }
  updateBirdEyeView();
  m_birdEyeView->setZoomRect(q->mapToScene(q->viewport()->rect()).boundingRect());
  m_birdEyeView->show();
  QSizeF newCanvasSize = m_canvas->sceneRect().size();
  if (newCanvasSize.width() < q->viewport()->width())
  {
    newCanvasSize.setWidth(q->viewport()->width());
  }
  else if (q->viewport()->width() < m_canvas->sceneRect().size().width())
  {
    newCanvasSize.setWidth(m_canvas->sceneRect().size().width());
  }
  if (newCanvasSize.height() < q->viewport()->height())
  {
    newCanvasSize.setHeight(q->viewport()->height());
  }
  else if (q->viewport()->height() < m_canvas->sceneRect().size().height())
  {
    newCanvasSize.setHeight(m_canvas->sceneRect().size().height());
  }
  //   std::cerr << "done." << std::endl;
}

void DotGraphViewPrivate::updateBirdEyeView()
{
  Q_Q(DotGraphView);
  qreal cvW = m_birdEyeView->width();
  qreal cvH = m_birdEyeView->height();
  qreal x = q->width()- cvW - q->verticalScrollBar()->width()    -2;
  qreal y = q->height()-cvH - q->horizontalScrollBar()->height() -2;
  QPoint oldZoomPos = m_birdEyeView->pos();
  QPoint newZoomPos = QPoint(0,0);
  KGraphViewerInterface::PannerPosition zp = m_zoomPosition;
  if (zp == KGraphViewerInterface::Auto)
  {
    QPointF tl1Pos = q->mapToScene(QPoint(0,0));
    QPointF tl2Pos = q->mapToScene(QPoint(cvW,cvH));
    QPointF tr1Pos = q->mapToScene(QPoint(x,0));
    QPointF tr2Pos = q->mapToScene(QPoint(x+cvW,cvH));
    QPointF bl1Pos = q->mapToScene(QPoint(0,y));
    QPointF bl2Pos = q->mapToScene(QPoint(cvW,y+cvH));
    QPointF br1Pos = q->mapToScene(QPoint(x,y));
    QPointF br2Pos = q->mapToScene(QPoint(x+cvW,y+cvH));
    int tlCols = m_canvas->items(QRectF(tl1Pos.x(),tl1Pos.y(),tl2Pos.x(),tl2Pos.y())).count();
    kDebug() << tr1Pos.x() << tr1Pos.y() << tr2Pos.x() << tr2Pos.y();
    int trCols = m_canvas->items(QRectF(tr1Pos.x(),tr1Pos.y(),tr2Pos.x(),tr2Pos.y())).count();
    int blCols = m_canvas->items(QRectF(bl1Pos.x(),bl1Pos.y(),bl2Pos.x(),bl2Pos.y())).count();
    int brCols = m_canvas->items(QRectF(br1Pos.x(),br1Pos.y(),br2Pos.x(),br2Pos.y())).count();
    int minCols = tlCols;
    zp = m_lastAutoPosition;
    switch(zp)
    {
      case KGraphViewerInterface::TopRight:    minCols = trCols; break;
      case KGraphViewerInterface::BottomLeft:  minCols = blCols; break;
      case KGraphViewerInterface::BottomRight: minCols = brCols; break;
      default:
      case KGraphViewerInterface::TopLeft:     minCols = tlCols; break;
    }
    if (minCols > tlCols) { minCols = tlCols; zp = KGraphViewerInterface::TopLeft; }
    if (minCols > trCols) { minCols = trCols; zp = KGraphViewerInterface::TopRight; }
    if (minCols > blCols) { minCols = blCols; zp = KGraphViewerInterface::BottomLeft; }
    if (minCols > brCols) { minCols = brCols; zp = KGraphViewerInterface::BottomRight; }
    
    m_lastAutoPosition = zp;
  }
  
  switch(zp)
  {
    case KGraphViewerInterface::TopRight:
      newZoomPos = QPoint(x,0);
      break;
    case KGraphViewerInterface::BottomLeft:
      newZoomPos = QPoint(0,y);
      break;
    case KGraphViewerInterface::BottomRight:
      newZoomPos = QPoint(x,y);
      break;
    default:
      break;
  }
  if (newZoomPos != oldZoomPos)
    m_birdEyeView->move(newZoomPos);
}

int DotGraphViewPrivate::displaySubgraph(GraphSubgraph* gsubgraph, int zValue, CanvasElement* parent)
{
  kDebug();
  Q_Q(DotGraphView);
  double scaleX = 1.0, scaleY = 1.0;
  
  if (m_detailLevel == 0)      { scaleX = m_graph->scale() * 0.7; scaleY = m_graph->scale() * 0.7; }
  else if (m_detailLevel == 1) { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
  else if (m_detailLevel == 2) { scaleX = m_graph->scale() * 1.3; scaleY = m_graph->scale() * 1.3; }
  else                        { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
  
  qreal gh = m_graph->height();
  
  if (gsubgraph->canvasSubgraph() == 0)
  {
    kDebug() << "Creating canvas subgraph for" << gsubgraph->id();
    CanvasSubgraph* csubgraph = new CanvasSubgraph(q, gsubgraph, m_canvas, parent);
    csubgraph->initialize(
      scaleX, scaleY, m_xMargin, m_yMargin, gh,
      m_graph->wdhcf(), m_graph->hdvcf());
    gsubgraph->setCanvasSubgraph(csubgraph);
    //       csubgraph->setZValue(gsubgraph->z());
    csubgraph->setZValue(zValue+=2);
    csubgraph->show();
    m_canvas->addItem(csubgraph);
    kDebug() << " one CanvasSubgraph... Done";
  }
  foreach (GraphElement* element, gsubgraph->content())
  {
    GraphNode* gnode = dynamic_cast<GraphNode*>(element);
    if (gnode->canvasNode()==0)
    {
      kDebug() << "Creating canvas node for:" << gnode->id();
      CanvasNode *cnode = new CanvasNode(q, gnode, m_canvas);
      if (cnode == 0) continue;
      cnode->initialize(
        scaleX, scaleY, m_xMargin, m_yMargin, gh,
        m_graph->wdhcf(), m_graph->hdvcf());
      gnode->setCanvasNode(cnode);
      m_canvas->addItem(cnode);
      //       cnode->setZValue(gnode->z());
      cnode->setZValue(zValue+1);
      cnode->show();
    }
    gnode->canvasNode()->computeBoundingRect();
  }
  gsubgraph->canvasSubgraph()->computeBoundingRect();
  
  int newZvalue = zValue;
  foreach(GraphSubgraph* ssg, gsubgraph->subgraphs())
  {
    int hereZvalue = displaySubgraph(ssg, zValue, gsubgraph->canvasSubgraph());
    if (hereZvalue > newZvalue)
      newZvalue = hereZvalue;
  }
  return newZvalue;
}

void DotGraphViewPrivate::setupPopup()
{
  Q_Q(DotGraphView);
  if (m_popup != 0)
  {
    return;
  }
  kDebug() << "DotGraphView::setupPopup";
  m_popup = new QMenu();
  
  m_layoutAlgoSelectAction = new KSelectAction(i18n("Select Layout Algo"),q);
  actionCollection()->addAction("view_layout_algo",m_layoutAlgoSelectAction);
  
  QStringList layoutAlgos;
  KAction* lea = new KAction(i18n(" "), q);
  lea->setWhatsThis(i18n("Specify yourself the layout command to use. Given a dot file, it should produce an xdot file on its standard output."));
  actionCollection()->addAction("layout_specifiy",lea);
  lea->setCheckable(false);
  
  KAction* lda = new KAction(i18n("Dot"), q);
  lda->setWhatsThis(i18n("Layout the graph using the dot program."));
  actionCollection()->addAction("layout_dot",lda);
  lda->setCheckable(false);
  
  KAction* lna = new KAction(i18n("Neato"), q);
  lna->setWhatsThis(i18n("Layout the graph using the neato program."));
  actionCollection()->addAction("layout_neato",lna);
  lna->setCheckable(false);
  
  KAction* lta = new KAction(i18n("Twopi"), q);
  lta->setWhatsThis(i18n("Layout the graph using the twopi program."));
  actionCollection()->addAction("layout_twopi",lta);
  lta->setCheckable(false);
  
  KAction* lfa = new KAction(i18n("Fdp"), q);
  lfa->setWhatsThis(i18n("Layout the graph using the fdp program."));
  actionCollection()->addAction("layout_fdp",lfa);
  lfa->setCheckable(false);
  
  KAction* lca = new KAction(i18n("Circo"), q);
  lca->setWhatsThis(i18n("Layout the graph using the circo program."));
  actionCollection()->addAction("layout_c",lca);
  lca->setCheckable(false);
  
  m_layoutAlgoSelectAction->addAction(lea);
  m_layoutAlgoSelectAction->addAction(lda);
  m_layoutAlgoSelectAction->addAction(lna);
  m_layoutAlgoSelectAction->addAction(lta);
  m_layoutAlgoSelectAction->addAction(lfa);
  m_layoutAlgoSelectAction->addAction(lca);
  
  m_layoutAlgoSelectAction->setCurrentItem(1);
  m_layoutAlgoSelectAction->setEditable(true);
  m_layoutAlgoSelectAction->setToolTip(i18n("Choose a GraphViz layout algorithm or edit your own one."));
  m_layoutAlgoSelectAction->setWhatsThis(i18n(
    "Choose a GraphViz layout algorithm or type in your own command that will "
    "generate a graph in the xdot format on its standard output. For example, to "
    "manually specify the <tt>G</tt> option to the dot command, type in: "
    "<tt>dot -Gname=MyGraphName -Txdot </tt>"));
  QObject::connect(m_layoutAlgoSelectAction, SIGNAL(triggered (const QString &)),
          q, SLOT(slotSelectLayoutAlgo(const QString&)));
  
  
  QMenu* layoutPopup = m_popup->addMenu(i18n("Layout"));
  layoutPopup->addAction(m_layoutAlgoSelectAction);
  QAction* slc = layoutPopup->addAction(i18n("Specify layout command"), q, SLOT(slotLayoutSpecify()));
  slc->setWhatsThis(i18n("Specify yourself the layout command to use. Given a dot file, it should produce an xdot file on its standard output."));
  QAction* rlc = layoutPopup->addAction(i18n("Reset layout command to default"), q, SLOT(slotLayoutReset()));
  rlc->setWhatsThis(i18n("Resets the layout command to use to the default depending on the graph type (directed or not)."));
  
  m_popup->addAction(KIcon("zoom-in"), i18n("Zoom In"), q, SLOT(zoomIn()));
  m_popup->addAction(KIcon("zoom-out"), i18n("Zoom Out"), q, SLOT(zoomOut()));
  
  m_popup->insertSeparator();
  
  KActionMenu* file_exportMenu = new KActionMenu(i18n("Export Graph"), q);
  actionCollection()->addAction("file_export",file_exportMenu);
  file_exportMenu->setToolTip(i18n("Allows the graph to be exported in another format."));
  file_exportMenu->setWhatsThis(i18n(
    "Use the Export Graph menu to export the graph in another format. "
    "There is currently only one export format supported: as a PNG image."));
  
  
  m_popup->addAction(file_exportMenu);
  KAction* exportToImageAction = new KAction(i18n("As Image..."),q);
  exportToImageAction->setWhatsThis(i18n("Export the graph to an image file (currently PNG only.)"));
  actionCollection()->addAction("export_image", exportToImageAction);
  QObject::connect(exportToImageAction,SIGNAL(triggered(bool)), q, SLOT(slotExportImage()));
  
  file_exportMenu->addAction(exportToImageAction);
  
  
  m_popup->insertSeparator();
  
  m_bevEnabledAction = new KToggleAction(
    KIcon(KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-bev.png")),
                                        i18n("Enable Bird's-eye View"), q);
    actionCollection()->addAction("view_bev_enabled",m_bevEnabledAction);
    m_bevEnabledAction->setShortcut(Qt::CTRL+Qt::Key_B);
    m_bevEnabledAction->setWhatsThis(i18n("Enables or disables the Bird's-eye View"));
    QObject::connect(m_bevEnabledAction,
            SIGNAL(toggled(bool)),
            q,
            SLOT(slotBevToggled()));
    m_bevEnabledAction->setCheckable(true);
    m_popup->addAction(m_bevEnabledAction);
    
    m_bevPopup = new KSelectAction(i18n("Birds-eye View"), q);
  m_bevPopup->setWhatsThis(i18n("Allows the Bird's-eye View to be setup."));
  m_popup->addAction(m_bevPopup);
  actionCollection()->addAction("view_bev",m_bevPopup);
  
  KAction* btla = new KAction(i18n("Top Left"), q);
  btla->setWhatsThis(i18n("Puts the Bird's-eye View in the top-left corner."));
  btla->setCheckable(true);
  actionCollection()->addAction("bev_top_left",btla);
  QObject::connect(btla, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          q, SLOT(slotBevTopLeft()));
  KAction* btra = new KAction(i18n("Top Right"), q);
  btra->setWhatsThis(i18n("Puts the Bird's-eye View in the top-right corner."));
  btra->setCheckable(true);
  actionCollection()->addAction("bev_top_right",btra);
  QObject::connect(btra, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          q, SLOT(slotBevTopRight()));
  KAction* bbla = new KAction(i18n("Bottom Left"), q);
  bbla->setWhatsThis(i18n("Puts the Bird's-eye View in the bottom-left corner."));
  bbla->setCheckable(true);
  actionCollection()->addAction("bev_bottom_left",bbla);
  QObject::connect(bbla, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          q, SLOT(slotBevBottomLeft()));
  KAction* bbra = new KAction(i18n("Bottom Right"), q);
  bbra->setWhatsThis(i18n("Puts the Bird's-eye View in the bottom-right corner."));
  bbra->setCheckable(true);
  actionCollection()->addAction("bev_bottom_right",bbra);
  QObject::connect(bbra, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          q, SLOT(slotBevBottomRight()));
  KAction* bba = new KAction(i18n("Automatic"), q);
  bba->setWhatsThis(i18n("Let KGraphViewer automatically choose the position of the Bird's-eye View."));
  bba->setCheckable(true);
  actionCollection()->addAction("bev_automatic",bba);
  QObject::connect(bba, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)),
          q, SLOT(slotBevAutomatic()));
  m_bevPopup->addAction(btla);
  m_bevPopup->addAction(btra);
  m_bevPopup->addAction(bbla);
  m_bevPopup->addAction(bbra);
  m_bevPopup->addAction(bba);
  switch (m_zoomPosition)
  {
    case KGraphViewerInterface::TopLeft:
      btla->setChecked(true);
      break;
    case KGraphViewerInterface::TopRight:
      btra->setChecked(true);
      break;
    case KGraphViewerInterface::BottomLeft:
      bbla->setChecked(true);
      break;
    case KGraphViewerInterface::BottomRight:
      bbra->setChecked(true);
      break;
    case KGraphViewerInterface::Auto:
      bba->setChecked(true);
      break;
  }
  
  
  kDebug() << "    m_bevEnabledAction setting checked to : " << KGraphViewerPartSettings::birdsEyeViewEnabled();
  m_bevEnabledAction->setChecked(KGraphViewerPartSettings::birdsEyeViewEnabled());
  m_bevPopup->setEnabled(KGraphViewerPartSettings::birdsEyeViewEnabled());
}

void DotGraphViewPrivate::exportToImage()
{
  // write current content of canvas as image to file
  if (!m_canvas) return;
  
  QString fn = KFileDialog::getSaveFileName(KUrl(":"),QString("*.png"),0,QString(""));
  
  if (!fn.isEmpty())
  {
    QPixmap pix(m_canvas->sceneRect().size().toSize());
    QPainter p(&pix);
    m_canvas->render( &p );
    pix.save(fn,"PNG");
  }
}


//
// DotGraphView
//
DotGraphView::DotGraphView(KActionCollection* actions, QWidget* parent) : 
    QGraphicsView(parent), d_ptr(new DotGraphViewPrivate(actions, this))
{
  kDebug() << "New node pic=" << KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newnode.png");
  Q_D(DotGraphView);
  d->m_canvas = 0;
  d->m_xMargin = d->m_yMargin = 0;
  d->m_birdEyeView = new PannerView(this);
  d->m_cvZoom = 1;

  // if there are ever graphic glitches to be found, remove this again
  setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontClipPainter |
                        QGraphicsView::DontSavePainterState);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  
  d->m_birdEyeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  d->m_birdEyeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  d->m_birdEyeView->raise();
  d->m_birdEyeView->hide();

  setFocusPolicy(Qt::StrongFocus);
  setBackgroundRole(QPalette::Window);
//   viewport()->setMouseTracking(true);
  
  connect(d->m_birdEyeView, SIGNAL(zoomRectMovedTo(QPointF)),
          this, SLOT(zoomRectMovedTo(QPointF)));
  connect(d->m_birdEyeView, SIGNAL(zoomRectMoveFinished()),
          this, SLOT(zoomRectMoveFinished()));

  setWhatsThis( i18n( 
    "<h1>GraphViz dot format graph visualization</h1>"
    "<p>If the graph is larger than the widget area, an overview "
        "panner is shown in one edge. Choose through the context menu "
        "if the optimal position of this overview should be automatically "
        "computed or put it where you want.</p>"
    "<h2>How to work with it?</h2>"
    "<ul>"
        "<li>To move the graph, you can:"
            "  <ul>"
                "    <li>click & drag it</li>"
                "    <li>use the elevators</li>"
                "    <li>press the arrows keys</li>"
                "    <li>click somewhere in the panner view</li>" 
                "    <li>use the mouse wheel (up and down with no modifier, left and right with the <Alt> key pressed)</li>"
                    "    <li>or click & drag the panner view</li>"
                    "  </ul>"
                "</li>"
            "<li>To zoom, you can either use the zoom in and zoom out toolbar buttons, or click on the <Shift> key while rolling your mouse wheel.</li>"
                "<li>Try the contextual menu (usually by right-clicking) to discover other "
                    "possibilities.</li>"
                "<li>Try the <tt>Print preview</tt> or the <tt>Page setup</tt> buttons to explore the printing options.</li>"
                "</ul>"
            ) );

  readViewConfig();
  
  QMatrix m;
  m.scale(d->m_zoom,d->m_zoom);
  setMatrix(m);
  d->setupPopup();
  setInteractive(true);
  setDragMode(NoDrag);
  setRenderHint(QPainter::Antialiasing);

  connect(&d->m_loadThread, SIGNAL(finished()), this, SLOT(slotAGraphReadFinished()));
  connect(&d->m_layoutThread, SIGNAL(finished()), this, SLOT(slotAGraphLayoutFinished()));
}

DotGraphView::~DotGraphView()
{
  saveViewConfig();
  Q_D(DotGraphView);
  delete d;
}

KGraphViewerInterface::PannerPosition DotGraphView::zoomPos() const { Q_D(const DotGraphView); return d->m_zoomPosition; }

double DotGraphView::zoom() const {Q_D(const DotGraphView); return d->m_zoom;}
KSelectAction* DotGraphView::bevPopup() {Q_D(DotGraphView); return d->m_bevPopup;}

DotGraph* DotGraphView::graph() {Q_D(DotGraphView); return d->m_graph;}
const DotGraph* DotGraphView::graph() const {Q_D(const DotGraphView); return d->m_graph;}

const GraphElement* DotGraphView::defaultNewElement() const {Q_D(const DotGraphView); return d->m_defaultNewElement;}
QPixmap DotGraphView::defaultNewElementPixmap() const {Q_D(const DotGraphView); return d->m_defaultNewElementPixmap;}

void DotGraphView::setDefaultNewElement(GraphElement* elem) {Q_D(DotGraphView); d->m_defaultNewElement = elem;}
void DotGraphView::setDefaultNewElementPixmap(const QPixmap& pm) {Q_D(DotGraphView); d->m_defaultNewElementPixmap = pm;}

bool DotGraphView::isReadWrite() const {Q_D(const DotGraphView); return d->m_readWrite;}
bool DotGraphView::isReadOnly() const {Q_D(const DotGraphView); return !d->m_readWrite;}

bool DotGraphView::highlighting() {Q_D(DotGraphView); return d->m_highlighting;}
void DotGraphView::setHighlighting(bool highlightingValue) {Q_D(DotGraphView); d->m_highlighting = highlightingValue;}

DotGraphView::EditingMode DotGraphView::editingMode() const {Q_D(const DotGraphView); return d->m_editingMode;}

void DotGraphView::setBackgroundColor(const QColor& color)
{
  Q_D(DotGraphView);
  d->m_backgroundColor = color;
  d->m_canvas->setBackgroundBrush(QBrush(d->m_backgroundColor));
}

bool DotGraphView::initEmpty()
{
  kDebug();
  Q_D(DotGraphView);
  d->m_birdEyeView->hide();
  d->m_birdEyeView->setScene(0);
  
  if (d->m_canvas) 
  {
    delete d->m_canvas;
    d->m_canvas = 0;
  }

  if (d->m_graph != 0)
    delete d->m_graph;
  d->m_graph = new DotGraph();
  connect(d->m_graph,SIGNAL(readyToDisplay()),this,SLOT(displayGraph()));

  if (d->m_readWrite)
  {
    d->m_graph->setReadWrite();
  }
  
//   kDebug() << "Parsing " << m_graph->dotFileName() << " with " << m_graph->layoutCommand();
  d->m_xMargin = 50;
  d->m_yMargin = 50;

  QGraphicsScene* newCanvas = new QGraphicsScene();
  QGraphicsSimpleTextItem* item = newCanvas->addSimpleText(i18n("no graph loaded"));
//   kDebug() << "Created canvas " << newCanvas;
  
  d->m_birdEyeView->setScene(newCanvas);
//   std::cerr << "After m_birdEyeView set canvas" << std::endl;
  
  setScene(newCanvas);
  d->m_canvas = newCanvas;
  centerOn(item);

  d->m_cvZoom = 0;

  return true;
}

bool DotGraphView::slotLoadLibrary(graph_t* graph)
{
  kDebug() << "graph_t";
  Q_D(DotGraphView);
  d->m_birdEyeView->setScene(0);

  if (d->m_canvas)
  {
    d->m_canvas->deleteLater();
    d->m_canvas = 0;
  }

  QString layoutCommand = (d->m_graph!=0?d->m_graph->layoutCommand():"");
  if (d->m_graph != 0)
    delete d->m_graph;

  if (layoutCommand.isEmpty())
  layoutCommand = "dot";

  kDebug() << "layoutCommand:" << layoutCommand;
  d->m_graph = new DotGraph(layoutCommand,"");
  d->m_graph->setUseLibrary(true);

  connect(d->m_graph,SIGNAL(readyToDisplay()),this,SLOT(displayGraph()));
  connect(this, SIGNAL(removeEdge(const QString&)), d->m_graph, SLOT(removeEdge(const QString&)));
  connect(this, SIGNAL(removeNodeNamed(const QString&)), d->m_graph, SLOT(removeNodeNamed(const QString&)));
  connect(this, SIGNAL(removeElement(const QString&)), d->m_graph, SLOT(removeElement(const QString&)));

  if (d->m_readWrite)
  {
    d->m_graph->setReadWrite();
  }

  if (layoutCommand.isEmpty())
  {
    layoutCommand = d->m_graph->chooseLayoutProgramForFile(d->m_graph->dotFileName());
  }
  d->m_graph->layoutCommand(layoutCommand);

  GVC_t* gvc = gvContext();
  gvLayout(gvc, graph, layoutCommand.toUtf8().data());
  gvRender (gvc, graph, "xdot", NULL);

  d->m_xMargin = 50;
  d->m_yMargin = 50;

  QGraphicsScene* newCanvas = new QGraphicsScene();
  kDebug() << "Created canvas " << newCanvas;

  d->m_birdEyeView->setScene(newCanvas);
  // std::cerr << "After m_birdEyeView set canvas" << std::endl;

  setScene(newCanvas);
  connect(newCanvas,SIGNAL(selectionChanged ()),this,SLOT(slotSelectionChanged()));
  d->m_canvas = newCanvas;

  d->m_cvZoom = 0;

  d->m_graph->updateWithGraph(graph);

  gvFreeLayout(gvc, graph);
  gvFreeContext(gvc);
  return true;
}

bool DotGraphView::loadDot(const QString& dotFileName)
{
  kDebug() << "'" << dotFileName << "'";
  Q_D(DotGraphView);
  d->m_birdEyeView->setScene(0);

  if (d->m_canvas)
  {
    d->m_canvas->deleteLater();
    d->m_canvas = 0;
  }

  QString layoutCommand = (d->m_graph!=0?d->m_graph->layoutCommand():"");
  if (d->m_graph != 0)
    delete d->m_graph;
  d->m_graph = new DotGraph(layoutCommand,dotFileName);
  connect(d->m_graph,SIGNAL(readyToDisplay()),this,SLOT(displayGraph()));

  if (d->m_readWrite)
  {
    d->m_graph->setReadWrite();
  }
  if (layoutCommand.isEmpty())
  {
    layoutCommand = d->m_graph->chooseLayoutProgramForFile(d->m_graph->dotFileName());
  }
  d->m_graph->layoutCommand(layoutCommand);

//   kDebug() << "Parsing " << m_graph->dotFileName() << " with " << m_graph->layoutCommand();
  d->m_xMargin = 50;
  d->m_yMargin = 50;

  QGraphicsScene* newCanvas = new QGraphicsScene();
  kDebug() << "Created canvas " << newCanvas;

  d->m_birdEyeView->setScene(newCanvas);
//   std::cerr << "After m_birdEyeView set canvas" << std::endl;

  setScene(newCanvas);
  connect(newCanvas,SIGNAL(selectionChanged ()),this,SLOT(slotSelectionChanged()));
  d->m_canvas = newCanvas;

  QGraphicsSimpleTextItem* loadingLabel = newCanvas->addSimpleText(i18n("graph %1 is getting loaded...", dotFileName));
  loadingLabel->setZValue(100);
  centerOn(loadingLabel);

  d->m_cvZoom = 0;

  if (!d->m_graph->parseDot(d->m_graph->dotFileName()))
  {
    kError() << "NOT successfully parsed!" << endl;
    loadingLabel->setText(i18n("error parsing file %1", dotFileName));
    return false;
  }
  return true;
}

bool DotGraphView::loadLibrary(const QString& dotFileName)
{
  kDebug() << "'" << dotFileName << "'";
  Q_D(DotGraphView);
  if (d->m_canvas)
    d->m_canvas->clear();
  QGraphicsSimpleTextItem* loadingLabel = d->m_canvas->addSimpleText(i18n("graph %1 is getting loaded...", dotFileName));
  loadingLabel->setZValue(100);
  centerOn(loadingLabel);

  d->m_loadThread.loadFile(dotFileName);
  
  return true;
}

bool DotGraphView::loadLibrary(graph_t* graph, const QString& layoutCommand)
{
  kDebug() << "graph_t";
  Q_D(DotGraphView);
  d->m_birdEyeView->setScene(0);
  
  if (d->m_canvas)
  {
    d->m_canvas->deleteLater();
    d->m_canvas = 0;
  }
  
  if (d->m_graph != 0)
    delete d->m_graph;

  kDebug() << "layoutCommand:" << layoutCommand;
  d->m_graph = new DotGraph(layoutCommand,"");
  d->m_graph->setUseLibrary(true);
  
  connect(d->m_graph,SIGNAL(readyToDisplay()),this,SLOT(displayGraph()));
  
  if (d->m_readWrite)
  {
    d->m_graph->setReadWrite();
  }
  
  d->m_xMargin = 50;
  d->m_yMargin = 50;
  
  QGraphicsScene* newCanvas = new QGraphicsScene();
  kDebug() << "Created canvas " << newCanvas;
  
  d->m_birdEyeView->setScene(newCanvas);
  setScene(newCanvas);
  connect(newCanvas,SIGNAL(selectionChanged ()),this,SLOT(slotSelectionChanged()));
  d->m_canvas = newCanvas;
  
  d->m_cvZoom = 0;
                                 
  d->m_graph->updateWithGraph(graph);

  return true;
}

void DotGraphView::slotSelectionChanged()
{
  kDebug() << scene()->selectedItems().size();
}

bool DotGraphView::displayGraph()
{
  Q_D(DotGraphView);
  kDebug() << d->m_graph->backColor();
//   hide();
  viewport()->setUpdatesEnabled(false);

  if (d->m_graph->backColor().size() != 0)
  {
    setBackgroundColor(QColor(d->m_graph->backColor()));
  }
  d->m_canvas->clear();

  if (d->m_graph->nodes().size() > KGV_MAX_PANNER_NODES)
  {
    d->m_birdEyeView->setDrawingEnabled(false);
  }
  //  QCanvasEllipse* eItem;
  double scaleX = 1.0, scaleY = 1.0;

  if (d->m_detailLevel == 0)      { scaleX = d->m_graph->scale() * 0.7; scaleY = d->m_graph->scale() * 0.7; }
  else if (d->m_detailLevel == 1) { scaleX = d->m_graph->scale() * 1.0; scaleY = d->m_graph->scale() * 1.0; }
  else if (d->m_detailLevel == 2) { scaleX = d->m_graph->scale() * 1.3; scaleY = d->m_graph->scale() * 1.3; }
  else                        { scaleX = d->m_graph->scale() * 1.0; scaleY = d->m_graph->scale() * 1.0; }

  qreal gh = d->m_graph->height();

  d->m_xMargin = 50;
  d->m_yMargin = 50;


//   m_canvas->setSceneRect(0,0,w+2*m_xMargin, h+2*m_yMargin);
//   m_canvas->setBackgroundBrush(QBrush(QColor(m_graph->backColor())));
  d->m_canvas->setBackgroundBrush(QBrush(d->m_backgroundColor));
  
//   kDebug() << "sceneRect is now " << m_canvas->sceneRect();
  
  kDebug() << "Creating" << d->m_graph->subgraphs().size() << "CanvasSubgraphs from" << d->m_graph;
  int zvalue = -1;
  foreach (GraphSubgraph* gsubgraph, d->m_graph->subgraphs())
  {
    int newZvalue = d->displaySubgraph(gsubgraph, zvalue);
    if (newZvalue > zvalue)
      zvalue = newZvalue;
  }

  kDebug() << "Creating" << d->m_graph->nodes().size() << "nodes from" << d->m_graph;
  GraphNodeMap::const_iterator it = d->m_graph->nodes().constBegin();
  for (; it != d->m_graph->nodes().constEnd();it++)
  {
    const QString& id = it.key();
    GraphNode* gnode = it.value();
    kDebug() << "Handling" << id << (void*)gnode;
    kDebug() << "  gnode id=" << gnode->id();
    kDebug()<<  "  canvasNode=" << (void*)gnode->canvasNode();
    if (gnode->canvasNode()==0)
    {
      kDebug() << "Creating canvas node for" << gnode->id();
      CanvasNode *cnode = new CanvasNode(this, gnode, d->m_canvas);
      if (cnode == 0) continue;
      cnode->initialize(
        scaleX, scaleY, d->m_xMargin, d->m_yMargin, gh,
        d->m_graph->wdhcf(), d->m_graph->hdvcf());
      gnode->setCanvasNode(cnode);
      d->m_canvas->addItem(cnode);
//       cnode->setZValue(gnode->z());
      cnode->setZValue(zvalue+1);
      cnode->show();
    }
    gnode->canvasNode()->computeBoundingRect();
  }

  kDebug() << "Creating" << d->m_graph->edges().size() << "edges from" << d->m_graph;
  foreach (GraphEdge* gedge, d->m_graph->edges())
  {
    kDebug() << "One GraphEdge:" << gedge->id();
    if (gedge->canvasEdge() == 0
      && gedge->fromNode() != 0
      && gedge->toNode() != 0)
    {
      kDebug() << "New CanvasEdge for" << gedge->id();
      kDebug() << "edge fromNode=" << (void*)gedge->fromNode();
      kDebug() << "              "<< gedge->fromNode()->id();
      kDebug() << "edge toNode=" << (void*)gedge->toNode();
      kDebug() << "              "<< gedge->toNode()->id();
      CanvasEdge* cedge = new CanvasEdge(this, gedge, scaleX, scaleY, d->m_xMargin,
          d->m_yMargin, gh, d->m_graph->wdhcf(), d->m_graph->hdvcf());

      gedge->setCanvasEdge(cedge);
  //     std::cerr << "setting z = " << gedge->z() << std::endl;
  //    cedge->setZValue(gedge->z());
      cedge->setZValue(zvalue+2);
      cedge->show();
      d->m_canvas->addItem(cedge);
    }
    if (gedge->canvasEdge() != 0)
      gedge->canvasEdge()->computeBoundingRect();
  }
  kDebug() << "Adding graph render operations: " << d->m_graph->renderOperations().size();
  foreach (const DotRenderOp& dro, d->m_graph->renderOperations())
  {
    if ( dro.renderop == "T" )
    {
//       std::cerr << "Adding graph label '"<<dro.str<<"'" << std::endl;
      const QString& str = dro.str;
      int stringWidthGoal = int(dro.integers[3] * scaleX);
      int fontSize = d->m_graph->fontSize();
      QFont* font = FontsCache::changeable().fromName(d->m_graph->fontName());
      font->setPointSize(fontSize);
      QFontMetrics fm(*font);
      while (fm.width(str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        font->setPointSize(fontSize);
        fm = QFontMetrics(*font);
      }
      QGraphicsSimpleTextItem* labelView = new QGraphicsSimpleTextItem(str, 0, d->m_canvas);
      labelView->setFont(*font);
      labelView->setPos(
                  (scaleX *
                       (
                         (dro.integers[0])
                         + (((dro.integers[2])*(dro.integers[3]))/2)
                         - ( (dro.integers[3])/2 )
                       )
                      + d->m_xMargin ),
                      ((gh - (dro.integers[1]))*scaleY)+ d->m_yMargin);
      /// @todo port that ; how to set text color ?
      labelView->setPen(QPen(Dot2QtConsts::componentData().qtColor(d->m_graph->fontColor())));
      labelView->setFont(*font);
      d->m_labelViews.insert(labelView);
    }
  }

  kDebug() << "Finalizing";
  d->m_cvZoom = 0;
  d->updateSizes();

  centerOn(d->m_canvas->sceneRect().center());

  viewport()->setUpdatesEnabled(true);
  QSet<QGraphicsSimpleTextItem*>::iterator labelViewsIt, labelViewsIt_end;
  labelViewsIt = d->m_labelViews.begin(); labelViewsIt_end = d->m_labelViews.end();
  for (; labelViewsIt != labelViewsIt_end; labelViewsIt++)
  {
    (*labelViewsIt)->show();
  }
  d->m_canvas->update();
  
  emit graphLoaded();

  return true;
}

void DotGraphView::focusInEvent(QFocusEvent*)
{
  Q_D(DotGraphView);
  if (!d->m_canvas) return;

//   m_canvas->update();
}

void DotGraphView::focusOutEvent(QFocusEvent* e)
{
  // trigger updates as in focusInEvent
  focusInEvent(e);
}

void DotGraphView::keyPressEvent(QKeyEvent* e)
{
  Q_D(DotGraphView);
  if (!d->m_canvas) 
  {
    e->ignore();
    return;
  }

  // move canvas...
  if (e->key() == Qt::Key_Home)
    scrollContentsBy(int(-d->m_canvas->width()),0);
  else if (e->key() == Qt::Key_End)
    scrollContentsBy(int(d->m_canvas->width()),0);
  else if (e->key() == Qt::Key_Prior)
    scrollContentsBy(0,-viewport()->height()/2);
  else if (e->key() == Qt::Key_Next)
    scrollContentsBy(0,viewport()->height()/2);
  else if (e->key() == Qt::Key_Left)
    scrollContentsBy(-viewport()->width()/10,0);
  else if (e->key() == Qt::Key_Right)
    scrollContentsBy(viewport()->width()/10,0);
  else if (e->key() == Qt::Key_Down)
    scrollContentsBy(0,viewport()->height()/10);
  else if (e->key() == Qt::Key_Up)
    scrollContentsBy(0,-viewport()->height()/10);
  else 
  {
    e->ignore();
    return;
  }
}

void DotGraphView::wheelEvent(QWheelEvent* e)
{
  Q_D(DotGraphView);
  if (!d->m_canvas) 
  {
    e->ignore();
    return;
  }
  e->accept();
  if (e->state() == Qt::ShiftModifier)
  {
    kDebug() << " + Shift: zooming";
    // move canvas...
    if (e->delta() < 0)
    {
      zoomOut();
    }
    else 
    {
      zoomIn();
    }
  }
  else
  {
    kDebug() << " : scrolling ";
    if (e->orientation() == Qt::Horizontal)
    {
      if (e->delta() < 0)
      {
        kDebug() << "scroll by " <<  -viewport()->width()/10 << 0;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()+viewport()->width()/10);
      }
      else
      {
        kDebug() << "scroll by " <<  viewport()->width()/10 << 0;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-viewport()->width()/10);
      }
    }
    else
    {
      if (e->delta() < 0)
      {
        kDebug() << "scroll by " << 0 << viewport()->width()/10;
        verticalScrollBar()->setValue(verticalScrollBar()->value()+viewport()->height()/10);
      }
      else
      {
        kDebug() << "scroll by " << 0 << -viewport()->width()/10;
        verticalScrollBar()->setValue(verticalScrollBar()->value()-viewport()->height()/10);
      }
    }
  }
}

void DotGraphView::zoomIn()
{
  applyZoom(1.10);
}


void DotGraphView::zoomOut()
{
  applyZoom(.90);
}

void DotGraphView::setZoomFactor(double newZoom)
{
  Q_D(DotGraphView);
  if (newZoom < 0.1 || newZoom > 10)
    return;
  d->m_zoom = newZoom;
  if (d->m_zoom > 1.0 && d->m_zoom < 1.1)
  {
    d->m_zoom = 1;
  }

  const double factor = newZoom / d->m_zoom;
  qreal centerX = (sceneRect().x() + (viewport()->width() / 2))*factor;
  qreal centerY = (sceneRect().y() + (viewport()->height() / 2))*factor;
  
  setUpdatesEnabled(false);
  QMatrix m;
  m.scale(d->m_zoom,d->m_zoom);
  setMatrix(m);
  centerOn(centerX, centerY);
  emit zoomed(d->m_zoom);
  setUpdatesEnabled(true);
  d->updateSizes();
}

void DotGraphView::applyZoom(double factor)
{
  Q_D(DotGraphView);
  setZoomFactor(d->m_zoom * factor);
}

void DotGraphView::scrollContentsBy(int dx, int dy)
{
  Q_D(DotGraphView);
  QGraphicsView::scrollContentsBy(dx, dy);
  if (d->m_birdEyeView && scene()) { // we might be shutting down
    d->m_birdEyeView->moveZoomRectTo(mapToScene(viewport()->rect()).boundingRect().center(), false);
  }
}

void DotGraphView::resizeEvent(QResizeEvent* e)
{
  Q_D(DotGraphView);
  kDebug() << "resizeEvent";
  QGraphicsView::resizeEvent(e);
  if (d->m_canvas) d->updateSizes(e->size());
//   std::cerr << "resizeEvent end" << std::endl;
}

void DotGraphView::zoomRectMovedTo(QPointF newZoomPos)
{
//   kDebug() << "DotGraphView::zoomRectMovedTo " << newZoomPos;
  centerOn(newZoomPos);
}
                    
void DotGraphView::zoomRectMoveFinished()
{
  Q_D(DotGraphView);
//    kDebug() << "zoomRectMoveFinished";
  d->updateBirdEyeView();
//   std::cerr << "zoomRectMoveFinished end" << std::endl;
}

void DotGraphView::mousePressEvent(QMouseEvent* e)
{
  Q_D(DotGraphView);
  if (e->button() != Qt::LeftButton) {
    return;
  }
  kDebug() << e << d->m_editingMode;
  QGraphicsView::mousePressEvent(e);

  if (d->m_editingMode == AddNewElement)
  {
    double scaleX = 1.0, scaleY = 1.0;

    if (d->m_detailLevel == 0)      { scaleX = d->m_graph->scale() * 0.7; scaleY = d->m_graph->scale() * 0.7; }
    else if (d->m_detailLevel == 1) { scaleX = d->m_graph->scale() * 1.0; scaleY = d->m_graph->scale() * 1.0; }
    else if (d->m_detailLevel == 2) { scaleX = d->m_graph->scale() * 1.3; scaleY = d->m_graph->scale() * 1.3; }
    else                        { scaleX = d->m_graph->scale() * 1.0; scaleY = d->m_graph->scale() * 1.0; }

    qreal gh = d->m_graph->height();


    QPointF pos = mapToScene(
        e->pos().x()-d->m_defaultNewElementPixmap.width()/2,
        e->pos().y()-d->m_defaultNewElementPixmap.height()/2);
    GraphNode* newNode = new GraphNode();
    newNode->attributes() = d->m_newElementAttributes;
    if (newNode->attributes().find("id") == newNode->attributes().end())
    {
      newNode->setId(QString("NewNode%1").arg(d->m_graph->nodes().size()));
    }
    if (newNode->attributes().find("label") == newNode->attributes().end())
    {
      newNode->setLabel(newNode->id());
    }
    d->m_graph->nodes().insert(newNode->id(), newNode);
    CanvasNode* newCNode = new CanvasNode(this, newNode, d->m_canvas);
    newCNode->initialize(
      scaleX, scaleY, d->m_xMargin, d->m_yMargin, gh,
      d->m_graph->wdhcf(), d->m_graph->hdvcf());
    newNode->setCanvasNode(newCNode);
    scene()->addItem(newCNode);
    kDebug() << "setting pos to " << pos;
    newCNode->setPos(pos);
    newCNode->setZValue(100);
    newCNode->show();


    d->m_editingMode = None;
    unsetCursor();
    emit newNodeAdded(newNode->id());
  }
  else if (d->m_editingMode == SelectingElements)
  {
  }
  else
  {
    if (d->m_editingMode != None && itemAt(e->pos()) == 0) // click outside any item: unselect all
    {
      if (d->m_editingMode == DrawNewEdge) // was drawing an edge; cancel it
      {
        if (d->m_newEdgeDraft!=0)
        {
          d->m_newEdgeDraft->hide();
          scene()->removeItem(d->m_newEdgeDraft);
          delete d->m_newEdgeDraft;
          d->m_newEdgeDraft = 0;
        }
        d->m_newEdgeSource = 0;
        d->m_editingMode = None;
      }
      else if (d->m_editingMode == AddNewEdge)
      {
        d->m_editingMode = None;
      }
      foreach(GraphEdge* e, d->m_graph->edges())
      {
        if (e->isSelected()) {
          e->setSelected(false);
          e->canvasEdge()->update();
        }
      }
      foreach(GraphNode* n, d->m_graph->nodes())
      {
        if (n->isSelected()) {
          n->setSelected(false);
          n->canvasElement()->update();
        }
      }
      foreach(GraphSubgraph* s, d->m_graph->subgraphs())
      {
        if (s->isSelected()) {
          s->setSelected(false);
          s->canvasElement()->update();
        }
      }
      emit selectionIs(QList<QString>(),QPoint());
    }
    d->m_pressPos = e->globalPos();
    d->m_pressScrollBarsPos = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
  }
  d->m_isMoving = true;
}

void DotGraphView::mouseMoveEvent(QMouseEvent* e)
{
  Q_D(DotGraphView);
  QGraphicsView::mouseMoveEvent(e);
//   kDebug() << scene()->selectedItems().size();

  if (d->m_editingMode == DrawNewEdge)
  {
    if (d->m_newEdgeDraft != 0)
    {
      QPointF src = d->m_newEdgeDraft->line().p1();
      QPointF tgt = mapToScene(e->pos());

//     kDebug() << "Setting new edge draft line to" << QLineF(src,tgt);
      d->m_newEdgeDraft->setLine(QLineF(src,tgt));
    }
  }
  else if (d->m_editingMode == SelectingElements)
  {
//     kDebug() << "selecting";
  }
  else if (e->buttons().testFlag(Qt::LeftButton))
  {
//     kDebug() << (e->globalPos() - d->m_pressPos);
    QPoint diff = e->globalPos() - d->m_pressPos;
    horizontalScrollBar()->setValue(d->m_pressScrollBarsPos.x()-diff.x());
    verticalScrollBar()->setValue(d->m_pressScrollBarsPos.y()-diff.y());
  }
}

void DotGraphView::mouseReleaseEvent(QMouseEvent* e)
{
  Q_D(DotGraphView);
  kDebug() << e << d->m_editingMode;
//   kDebug() << "setDragMode(NoDrag)";
//   setDragMode(NoDrag);
  if (d->m_editingMode == AddNewElement)
  {
    d->m_editingMode = None;
    unsetCursor();
  }
  else if (d->m_editingMode == SelectingElements)
  {
    QGraphicsView::mouseReleaseEvent(e);
    kDebug() << "Stopping selection" << scene() << d->m_canvas;
    QList<QGraphicsItem *> items = scene()->selectedItems();
    QList<QString> selection;
    foreach (QGraphicsItem * item, items)
    {
      CanvasElement* element = dynamic_cast<CanvasElement*>(item);
      element->element()->setSelected(true);
      if (element != 0)
      {
        selection.push_back(element->element()->id());
      }
    }
    d->m_editingMode = None;
    unsetCursor();
    setDragMode(NoDrag);
    if (!selection.isEmpty())
    {
      update();
      emit selectionIs(selection, mapToGlobal( e->pos() ));
    }
  }
  else
  {
    QGraphicsView::mouseReleaseEvent(e);
  }
  d->m_isMoving = false;
}

void DotGraphView::mouseDoubleClickEvent(QMouseEvent* e)
{
  QGraphicsView::mouseDoubleClickEvent(e);
}

void DotGraphView::contextMenuEvent(QContextMenuEvent* e)
{
  Q_D(DotGraphView);
  kDebug();
//   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

  d->m_popup->exec(e->globalPos());
}

void DotGraphView::slotContextMenuEvent(const QString& id, const QPoint& p)
{
  kDebug();
//   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

  emit (contextMenuEvent(id, p));
}

void DotGraphView::slotElementHoverEnter(CanvasElement* element)
{
  kDebug() << element->element()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverEnter(element->element()->id()));
}

void DotGraphView::slotElementHoverLeave(CanvasElement* element)
{
  kDebug() << element->element()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverLeave(element->element()->id()));
}

void DotGraphView::slotElementHoverEnter(CanvasEdge* element)
{
  kDebug() << element->edge()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverEnter(element->edge()->id()));
}

void DotGraphView::slotElementHoverLeave(CanvasEdge* element)
{
  kDebug() << element->edge()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverLeave(element->edge()->id()));
}

void DotGraphView::setLayoutCommand(const QString& command)
{
  Q_D(DotGraphView);
  d->m_graph->layoutCommand(command);
  reload();
}

KGraphViewerInterface::PannerPosition DotGraphView::zoomPos(const QString& s)
{
  KGraphViewerInterface::PannerPosition  res = DEFAULT_ZOOMPOS;
  if (s == QString("KGraphViewerInterface::TopLeft")) res = KGraphViewerInterface::TopLeft;
  if (s == QString("KGraphViewerInterface::TopRight")) res = KGraphViewerInterface::TopRight;
  if (s == QString("KGraphViewerInterface::BottomLeft")) res = KGraphViewerInterface::BottomLeft;
  if (s == QString("KGraphViewerInterface::BottomRight")) res = KGraphViewerInterface::BottomRight;
  if (s == QString("Automatic")) res = KGraphViewerInterface::Auto;

  return res;
}

void DotGraphView::setPannerEnabled(bool enabled)
{
  Q_D(DotGraphView);
  d->m_bevPopup->setEnabled(d->m_bevEnabledAction->isChecked());
  KGraphViewerPartSettings::setBirdsEyeViewEnabled(d->m_bevEnabledAction->isChecked());
  KGraphViewerPartSettings::self()->writeConfig();
  d->updateSizes();
}

void DotGraphView::viewBevActivated(int newZoomPos)
{
  Q_D(DotGraphView);
  d->m_zoomPosition = (KGraphViewerInterface::PannerPosition)newZoomPos;
  d->updateSizes();
  emit(sigViewBevActivated(newZoomPos));
}

QString DotGraphView::zoomPosString(KGraphViewerInterface::PannerPosition p)
{
    if (p == KGraphViewerInterface::TopRight) return QString("KGraphViewerInterface::TopRight");
    if (p == KGraphViewerInterface::BottomLeft) return QString("KGraphViewerInterface::BottomLeft");
    if (p == KGraphViewerInterface::BottomRight) return QString("KGraphViewerInterface::BottomRight");
    if (p == KGraphViewerInterface::Auto) return QString("Automatic");

    return QString("KGraphViewerInterface::TopLeft");
}

void DotGraphView::readViewConfig()
{
  Q_D(DotGraphView);
  KConfigGroup g(KGlobal::config(),"GraphViewLayout");
  
  QVariant dl = DEFAULT_DETAILLEVEL;
  d->m_detailLevel     = g.readEntry("DetailLevel", dl).toInt();
  d->m_zoomPosition  = zoomPos(g.readEntry("KGraphViewerInterface::PannerPosition",
            zoomPosString(DEFAULT_ZOOMPOS)));
  emit(sigViewBevActivated(d->m_zoomPosition));
}

void DotGraphView::saveViewConfig()
{
  Q_D(DotGraphView);
//   kDebug() << "Saving view config";  
  KConfigGroup g(KGlobal::config(), "GraphViewLayout");

    writeConfigEntry(&g, "DetailLevel", d->m_detailLevel, DEFAULT_DETAILLEVEL);
    writeConfigEntry(&g, "KGraphViewerInterface::PannerPosition",
         zoomPosString(d->m_zoomPosition),
         zoomPosString(DEFAULT_ZOOMPOS).utf8().data());
  g.sync();
}

void DotGraphView::pageSetup()
{
  Q_D(DotGraphView);
  if (d->m_printCommand == 0)
  {
    d->m_printCommand = new KGVSimplePrintingCommand(this, 0);
  }
  d->m_printCommand->showPageSetup(d->m_graph->dotFileName());
  return;
}

void DotGraphView::print()
{
  Q_D(DotGraphView);
  if (d->m_printCommand == 0)
  {
    d->m_printCommand = new KGVSimplePrintingCommand(this, 0);
  }
  d->m_printCommand->print(d->m_graph->dotFileName());
  return;
}

void DotGraphView::printPreview()
{
  Q_D(DotGraphView);
  if (d->m_printCommand == 0)
  {
    d->m_printCommand = new KGVSimplePrintingCommand(this, 0);
  }
  d->m_printCommand->showPrintPreview(d->m_graph->dotFileName(), false);
  return;
}

bool DotGraphView::reload()
{
  Q_D(DotGraphView);
  QString fileName = d->m_graph->dotFileName();
  if (d->m_graph->useLibrary())
    return loadLibrary(fileName);
  else
    return loadDot(fileName);
}

void DotGraphView::dirty(const QString& dotFileName)
{
  Q_D(DotGraphView);
//   std::cerr << "SLOT dirty for " << dotFileName << std::endl;
  if (dotFileName == d->m_graph->dotFileName())
  {
    if (KMessageBox::questionYesNo(this, 
                                i18n("The file %1 has been modified on disk.\nDo you want to reload it?",dotFileName),
                                i18n("Reload Confirmation"),
                                KStandardGuiItem::yes(),
                                KStandardGuiItem::no(),
                                "reloadOnChangeMode"   ) == KMessageBox::Yes)
    {
      if (d->m_graph->useLibrary())
        loadLibrary(dotFileName);
      else
        loadDot(dotFileName);
    }
  }
}

KConfigGroup* DotGraphView::configGroup(KConfig* c,
                                         const QString& group, const QString& post)
{
  QStringList gList = c->groupList();
  QString res = group;
  if (gList.contains((group+post).ascii()) ) res += post;
  return new KConfigGroup(c, res);
}

void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
                                     const QString& value, const char* def)
{
  if (!c) return;
  if ((value.isEmpty() && ((def == 0) || (*def == 0))) ||
      (value == QString(def)))
    c->deleteEntry(pKey);
  else
    c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
                                     int value, int def)
{
  if (!c) return;
  if (value == def)
    c->deleteEntry(pKey);
  else
    c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
                                     double value, double def)
{
  if (!c) return;
  if (value == def)
    c->deleteEntry(pKey);
  else
    c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
                                     bool value, bool def)
{
  if (!c) return;
  if (value == def)
    c->deleteEntry(pKey);
  else
    c->writeEntry(pKey, value);
}

const QString& DotGraphView::dotFileName() 
{
  Q_D(DotGraphView);
  return d->m_graph->dotFileName();
}

void DotGraphView::hideToolsWindows()
{
  Q_D(DotGraphView);
  if (d->m_printCommand != 0)
  {
    d->m_printCommand->hidePageSetup();
    d->m_printCommand->hidePrintPreview();
  }
}

void DotGraphView::slotExportImage()
{
  Q_D(DotGraphView);
  d->exportToImage();
}

void DotGraphView::slotLayoutSpecify()
{
  {
  Q_D(DotGraphView);
    bool ok = false;
    QString currentLayoutCommand = d->m_graph->layoutCommand();
    QString layoutCommand = 
    KInputDialog::getText(
      i18n("Layout Command"), 
      i18n("Type in a layout command for the current graph:"), 
      currentLayoutCommand, 
      &ok, 
      this, 
      0, 
      QString(), 
      i18n("Specify here the command that will be used to layout the graph.\n"
      "The command MUST write its results on stdout in xdot format."));
    //       std::cerr << "Got layout command: " << layoutCommand << std::endl;
    if (ok && layoutCommand != currentLayoutCommand)
    {
      //         std::cerr << "Setting new layout command: " << layoutCommand << std::endl;
      setLayoutCommand(layoutCommand);
    }
  }
}

void DotGraphView::slotLayoutReset()
{
  setLayoutCommand("");
}

void DotGraphView::slotSelectLayoutAlgo(const QString& ttext)
{
  QString text = ttext;//.mid(1);
  kDebug() << "DotGraphView::slotSelectLayoutAlgo '" << text << "'";
  if (text == "Dot")
  {
    setLayoutCommand("dot");
  }
  else if (text == "Neato")
  {
    setLayoutCommand("neato");
  }
  else if (text == "Twopi")
  {
    setLayoutCommand("twopi");
  }
  else if (text == "Fdp")
  {
    setLayoutCommand("fdp");
  }
  else if (text == "Circo")
  {
    setLayoutCommand("circo");
  }
  else 
  {
    setLayoutCommand(text);
  }
}

void DotGraphView::slotSelectLayoutDot()
{
  kDebug() << "DotGraphView::slotSelectLayoutDot";
  setLayoutCommand("dot -Txdot");
}

void DotGraphView::slotSelectLayoutNeato()
{
  kDebug() << "DotGraphView::slotSelectLayoutNeato";
  setLayoutCommand("neato -Txdot");
}

void DotGraphView::slotSelectLayoutTwopi()
{
  kDebug() << "DotGraphView::slotSelectLayoutTwopi";
  setLayoutCommand("twopi -Txdot");
}

void DotGraphView::slotSelectLayoutFdp()
{
  kDebug() << "DotGraphView::slotSelectLayoutFdp";
  setLayoutCommand("fdp -Txdot");
}

void DotGraphView::slotSelectLayoutCirco()
{
  kDebug() << "DotGraphView::slotSelectLayoutCirco";
  setLayoutCommand("circo -Txdot");
}

void DotGraphView::slotBevToggled()
{
  Q_D(DotGraphView);
  kDebug() << "DotGraphView::slotBevToggled";
  kDebug() << "    d->m_bevEnabledAction is checked ? " << d->m_bevEnabledAction->isChecked();
  setPannerEnabled(d->m_bevEnabledAction->isChecked());
}

void DotGraphView::slotBevTopLeft()
{
  viewBevActivated(KGraphViewerInterface::TopLeft);
}

void DotGraphView::slotBevTopRight()
{
  viewBevActivated(KGraphViewerInterface::TopRight);
}

void DotGraphView::slotBevBottomLeft()
{
  viewBevActivated(KGraphViewerInterface::BottomLeft);
}

void DotGraphView::slotBevBottomRight()
{
  viewBevActivated(KGraphViewerInterface::BottomRight); 
}

void DotGraphView::slotBevAutomatic()
{
  viewBevActivated(KGraphViewerInterface::Auto);
}

void DotGraphView::slotUpdate()
{
  Q_D(DotGraphView);
  kDebug();
  d->m_graph->update();
}

void DotGraphView::prepareAddNewElement(QMap<QString,QString> attribs)
{
  Q_D(DotGraphView);
  kDebug() ;
  d->m_editingMode = AddNewElement;
  d->m_newElementAttributes = attribs;
  unsetCursor();
  setCursor(QCursor(d->m_defaultNewElementPixmap));
}

void DotGraphView::prepareAddNewEdge(QMap<QString,QString> attribs)
{
  Q_D(DotGraphView);
  kDebug() << attribs;
  bool anySelected = false;
  foreach (GraphEdge* edge, d->m_graph->edges())
  {
    if (edge->isSelected())
    {
      anySelected = true;
      QMap<QString,QString>::const_iterator it = attribs.constBegin();
      for(; it != attribs.constEnd(); it++)
      {
        edge->attributes()[it.key()] = it.value();
      }
    }
  }
  if (anySelected)
  {
    return;
  }
  d->m_editingMode = AddNewEdge;
  d->m_newElementAttributes = attribs;
  unsetCursor();
  QBitmap bm(KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newedge.png"));
  setCursor(QCursor(bm,bm,32,16));
}

void DotGraphView::prepareSelectElements()
{
  Q_D(DotGraphView);
  kDebug();
  d->m_editingMode = SelectingElements;
  setCursor(Qt::CrossCursor);
  setDragMode ( RubberBandDrag );
}

void DotGraphView::createNewEdgeDraftFrom(CanvasElement* node)
{
  Q_D(DotGraphView);
  kDebug() << node->element()->id();
  d->m_editingMode = DrawNewEdge;
  unsetCursor();
  d->m_newEdgeSource = node;

  if (d->m_newEdgeDraft != 0)
  {
    kDebug() << "removing new edge draft";
    d->m_newEdgeDraft->hide();
    scene()->removeItem(d->m_newEdgeDraft);
    delete d->m_newEdgeDraft;
    d->m_newEdgeDraft = 0;
  }
  d->m_newEdgeDraft = new QGraphicsLineItem(QLineF(node->boundingRect().center()+node->pos(),node->boundingRect().center()+node->pos()+QPointF(10,10)));
  scene()->addItem(d->m_newEdgeDraft);
  d->m_newEdgeDraft->setZValue(1000);
  d->m_newEdgeDraft->show();
  kDebug() << d->m_newEdgeDraft->line();
}

void DotGraphView::finishNewEdgeTo(CanvasElement* node)
{
  Q_D(DotGraphView);
  kDebug() << node->element()->id();
  d->m_editingMode = None;
  unsetCursor();

  if (d->m_newEdgeDraft != 0)
  {
    kDebug() << "removing new edge draft";
    d->m_newEdgeDraft->hide();
    scene()->removeItem(d->m_newEdgeDraft);
    delete d->m_newEdgeDraft;
    d->m_newEdgeDraft = 0;
  }

  emit newEdgeFinished(d->m_newEdgeSource->element()->id(),node->element()->id(),d->m_newElementAttributes);

  d->m_newEdgeSource = 0;
}

// void DotGraphView::slotFinishNewEdge(
//       const QString& srcId,
//       const QString& tgtId,
//       const QMap<QString, QString> newElementAttributes)
// {
//   kDebug() ;
// 
//   GraphEdge* gedge  = new GraphEdge();
//   gedge->setFromNode(d->m_graph->nodes()[srcId]);
//   gedge->setToNode(d->m_graph->nodes()[tgtId]);
//   gedge->attributes() = newElementAttributes;
//   foreach (const QString &attrib, newElementAttributes.keys())
//   {
//     if (attrib == "z")
//     {
//       bool ok;
//       gedge->setZ(newElementAttributes[attrib].toDouble(&ok));
//     }
//   }
//   gedge->setId(srcId+tgtId+QString::number(d->m_graph->edges().size()));
//   d->m_graph->edges().insert(gedge->id(), gedge);
// 
//   double scaleX = 1.0, scaleY = 1.0;
// 
//   if (d->m_detailLevel == 0)      { scaleX = d->m_graph->scale() * 0.7; scaleY = d->m_graph->scale() * 0.7; }
//   else if (d->m_detailLevel == 1) { scaleX = d->m_graph->scale() * 1.0; scaleY = d->m_graph->scale() * 1.0; }
//   else if (d->m_detailLevel == 2) { scaleX = d->m_graph->scale() * 1.3; scaleY = d->m_graph->scale() * 1.3; }
//   else                        { scaleX = d->m_graph->scale() * 1.0; scaleY = d->m_graph->scale() * 1.0; }
// 
//   qreal gh = d->m_graph->height();
//   CanvasEdge* cedge = new CanvasEdge(this, gedge, scaleX, scaleY, d->m_xMargin,
//         d->m_yMargin, gh, d->m_graph->wdhcf(), d->m_graph->hdvcf());
// 
//   gedge->setCanvasEdge(cedge);
// //     std::cerr << "setting z = " << gedge->z() << std::endl;
//   cedge->setZValue(gedge->z());
//   cedge->show();
//   scene()->addItem(cedge);
// 
//   emit newEdgeAdded(gedge->fromNode()->id(),gedge->toNode()->id());
// }

void DotGraphView::setReadOnly()
{
  Q_D(DotGraphView);
  kDebug() ;
 d-> m_readWrite = false;
  if (d->m_graph != 0)
  {
    d->m_graph->setReadOnly();
  }
}

void DotGraphView::setReadWrite()
{
  Q_D(DotGraphView);
  kDebug() ;
  d->m_readWrite = true;
  if (d->m_graph != 0)
  {
    d->m_graph->setReadWrite();
  }
}

void DotGraphView::slotEdgeSelected(CanvasEdge* edge, Qt::KeyboardModifiers modifiers)
{
  Q_D(DotGraphView);
  kDebug() << edge->edge()->id();
  QList<QString> selection;
  selection.push_back(edge->edge()->id());
  if (!modifiers.testFlag(Qt::ControlModifier))
  {
    foreach(GraphEdge* e, d->m_graph->edges())
    {
      if (e->canvasEdge() != edge)
      {
        e->setSelected(false);
        e->canvasEdge()->update();
      }
    }
    foreach(GraphNode* n, d->m_graph->nodes())
    {
      n->setSelected(false);
      n->canvasNode()->update();
    }
    foreach(GraphSubgraph* s, d->m_graph->subgraphs())
    {
      s->setElementSelected(0, false, true);
    }
  }
  else
  {
    foreach(GraphEdge* e, d->m_graph->edges())
    {
      if (e->canvasEdge() != edge)
      {
        if (e->isSelected())
        {
          selection.push_back(e->id());
        }
      }
    }
    foreach(GraphNode* n, d->m_graph->nodes())
    {
      if (n->isSelected())
      {
        selection.push_back(n->id());
      }
    }
    foreach(GraphSubgraph* s, d->m_graph->subgraphs())
    {
      if (s->isSelected())
      {
        selection.push_back(s->id());
      }
    }
  }
  emit selectionIs(selection, QPoint());
}

void DotGraphView::slotElementSelected(CanvasElement* element, Qt::KeyboardModifiers modifiers)
{
  Q_D(DotGraphView);
  kDebug();
  QList<QString> selection;
  selection.push_back(element->element()->id());
  if (!modifiers.testFlag(Qt::ControlModifier))
  {
    foreach(GraphEdge* e, d->m_graph->edges())
    {
      if (e->isSelected()) {
        e->setSelected(false);
        e->canvasEdge()->update();
      }
    }
    foreach(GraphNode* e, d->m_graph->nodes())
    {
      if (e->canvasElement() != element)
      {
        if (e->isSelected()) {
          e->setSelected(false);
          e->canvasElement()->update();
        }
      }
    }
    foreach(GraphSubgraph* s, d->m_graph->subgraphs())
    {
      s->setElementSelected(element->element(), true, true);
    }
  }
  else
  {
    foreach(GraphEdge* e, d->m_graph->edges())
    {
      if (e->isSelected())
      {
        selection.push_back(e->id());
      }
    }
    foreach(GraphNode* n, d->m_graph->nodes())
    {
      if (n->isSelected())
      {
        selection.push_back(n->id());
      }
    }
    foreach(GraphSubgraph* s, d->m_graph->subgraphs())
    {
      s->retrieveSelectedElementsIds(selection);
    }
  }
  emit selectionIs(selection, QPoint());
}

void DotGraphView::removeSelectedEdges()
{
  Q_D(DotGraphView);
  kDebug();
  foreach(GraphEdge* e, d->m_graph->edges())
  {
    if (e->isSelected())
    {
      kDebug() << "emiting removeEdge " << e->id();
      d->m_graph->removeEdge(e->id());
      emit removeEdge(e->id());
    }
  }
}

void DotGraphView::removeSelectedNodes()
{
  Q_D(DotGraphView);
  kDebug();
  foreach(GraphNode* e, d->m_graph->nodes())
  {
    if (e->isSelected())
    {
      kDebug() << "emiting removeElement " << e->id();
      d->m_graph->removeElement(e->id());
      emit removeElement(e->id());
    }
  }
}

void DotGraphView::removeSelectedSubgraphs()
{
  Q_D(DotGraphView);
  kDebug();
  foreach(GraphSubgraph* e, d->m_graph->subgraphs())
  {
    if (e->isSelected())
    {
      kDebug() << "emiting removeElement " << e->id();
      d->m_graph->removeElement(e->id());
      emit removeElement(e->id());
    }
  }
}

void DotGraphView::removeSelectedElements()
{
  kDebug();
  removeSelectedNodes();
  removeSelectedEdges();
  removeSelectedSubgraphs();
}

void DotGraphView::timerEvent ( QTimerEvent * event )
{
  Q_D(DotGraphView);
  kDebug() << event->timerId();
  qreal vpercent = verticalScrollBar()->value()*1.0/100;
  qreal hpercent = horizontalScrollBar()->value()*1.0/100;
  if (d->m_scrollDirection == Left)
  {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value()-(5*hpercent));
  }
  else if (d->m_scrollDirection == Right)
  {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value()+(5*hpercent));
  }
  else if (d->m_scrollDirection == Top)
  {
    verticalScrollBar()->setValue(verticalScrollBar()->value()-(5*vpercent));
  }
  else if (d->m_scrollDirection == Bottom)
  {
    verticalScrollBar()->setValue(verticalScrollBar()->value()+(5*vpercent));
  }
}

void DotGraphView::leaveEvent ( QEvent * /*event*/ )
{
  Q_D(DotGraphView);
  kDebug() << mapFromGlobal(QCursor::pos());
  if (d->m_editingMode == DrawNewEdge)
  {
    d->m_leavedTimer = startTimer(10);
    if (mapFromGlobal(QCursor::pos()).x() <= 0)
    {
      d->m_scrollDirection = Left;
    }
    else if (mapFromGlobal(QCursor::pos()).y() <= 0)
    {
      d->m_scrollDirection = Top;
    }
    else if (mapFromGlobal(QCursor::pos()).x() >= width())
    {
      d->m_scrollDirection = Right;
    }
    else if (mapFromGlobal(QCursor::pos()).y() >= height())
    {
      d->m_scrollDirection = Bottom;
    }
  }
}

void DotGraphView::enterEvent ( QEvent * /*event*/ )
{
  Q_D(DotGraphView);
  kDebug();
  if (d->m_leavedTimer != std::numeric_limits<int>::max())
  {
    killTimer(d->m_leavedTimer);
    d->m_leavedTimer = std::numeric_limits<int>::max();
  }
}

void DotGraphView::slotAGraphReadFinished()
{
  Q_D(DotGraphView);
  QString layoutCommand = (d->m_graph!=0?d->m_graph->layoutCommand():"");
  if (layoutCommand.isEmpty())
  {
    if (!d->m_loadThread.dotFileName().isEmpty())
      layoutCommand = d->m_graph->chooseLayoutProgramForFile(d->m_loadThread.dotFileName());
    else
      layoutCommand = "dot";
  }
  d->m_layoutThread.layoutGraph(d->m_loadThread.g(), layoutCommand);
}

void DotGraphView::slotAGraphLayoutFinished()
{
  Q_D(DotGraphView);
  bool result = loadLibrary(d->m_layoutThread.g(), d->m_layoutThread.layoutCommand());
  if (result)
    d->m_graph->dotFileName(d->m_loadThread.dotFileName());

  gvFreeLayout(d->m_layoutThread.gvc(), d->m_layoutThread.g());
  agclose(d->m_layoutThread.g());
}

void DotGraphView::slotSelectNode(const QString& nodeName)
{
  kDebug() << nodeName;
  GraphNode* node = dynamic_cast<GraphNode*>(graph()->elementNamed(nodeName));
  if (node == 0) return;
  node->setSelected(true);
  if (node->canvasNode()!=0)
  {
    node->canvasNode()->modelChanged();
    slotElementSelected(node->canvasNode(),Qt::NoModifier);
  }
}

void DotGraphView::centerOnNode(const QString& nodeId)
{
  GraphNode* node = dynamic_cast<GraphNode*>(graph()->elementNamed(nodeId));
  if (node == 0) return;
  if (node->canvasNode()!=0)
  {
    centerOn(node->canvasNode());
  }
}


}

#include "dotgraphview.moc"

