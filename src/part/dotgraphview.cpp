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

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <qtooltip.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qwhatsthis.h>
#include <qcanvas.h>
#include <qwmatrix.h>
#include <qpair.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qimage.h>
#include <qpaintdevicemetrics.h>
#include <qurl.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktempfile.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kprinter.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <krun.h>
#include <kcursor.h>

#include "dotgraphview.h"
#include "dotgraph.h"
#include "pannerview.h"
#include "canvassubgraph.h"
#include "canvasedge.h"
#include "dot2qtconsts.h"
#include "graphnode.h"
#include "canvasnode.h"
#include "graphedge.h"
#include "FontsCache.h"
#include "kgraphviewer_partsettings.h"
#include "printing/simpleprintingcommand.h"

// DotGraphView defaults

#define DEFAULT_ZOOMPOS      Auto
#define KGV_MAX_PANNER_NODES 100

using namespace KGraphViewer;
//
// Tooltips for DotGraphView
//

class CallGraphTip: public QToolTip
{
public:
  CallGraphTip( QWidget* p ):QToolTip(p) {}

protected:
    void maybeTip( const QPoint & );
};

void CallGraphTip::maybeTip( const QPoint& pos )
{
//   std::cerr << "CallGraphTip::maybeTip pos  = ("<<pos.x()<<", "<<pos.y()<<")" << std::endl;
  if (!parentWidget()->inherits( "DotGraphView" )) return;
  DotGraphView* cgv = (DotGraphView*)parentWidget();

  QPoint cPos = cgv->inverseWorldMatrix().map(cgv->viewportToContents(pos));
//   std::cerr << "CallGraphTip::maybeTip cPos = ("<<cPos.x()<<", "<<cPos.y()<<")" << std::endl;

  QCanvasItemList l = cgv->canvas()->collisions(cPos);
  if (l.count() == 0) return;
  QCanvasItem* i = l.first();

  CanvasNode* cn = dynamic_cast<CanvasNode*>(i);
  if (cn != 0) 
  {

    QString tipStr;
    if (cn->node() != 0)
    {
      QString id = cn->node()->id();
      QString label = cn->node()->label();
      tipStr = QString("id='%1'\nlabel='%2'").arg(id).arg(label);
    }
    QPoint vPosTL = cgv->contentsToViewport(cgv->worldMatrix().map(i->boundingRect().topLeft()));
    QPoint vPosBR = cgv->contentsToViewport(cgv->worldMatrix().map(i->boundingRect().bottomRight()));
    tip(QRect(vPosTL, vPosBR), tipStr);

    return;
  }

  // redirect from label / arrow to edge
  if (dynamic_cast<CanvasEdge*>(i) != 0) 
  {
    CanvasEdge* ce = (CanvasEdge*)i;
    GraphEdge* e = ce->edge();

    QString tipStr = QString("%1 -> %2\nlabel='%3'")
      .arg(e->fromNode()->id()).arg(e->toNode()->id()).arg(e->label());
    tip(QRect(pos.x()-5,pos.y()-5,pos.x()+5,pos.y()+5), tipStr);
  }
}




//
// DotGraphView
//
DotGraphView::DotGraphView(QWidget* parent, const char* name) : 
    QCanvasView(parent, name), m_exporter(),
    m_zoomPosition(DEFAULT_ZOOMPOS), 
    m_lastAutoPosition(TopLeft),
    m_graph(0),
    m_zoom(1),
    m_focusedNode(0),
    m_isMoving(false),
    m_printCommand(0),
    m_labelViews(),
    m_popup(0),
    m_tip(new CallGraphTip(this))
{
  m_canvas = 0;
  m_xMargin = m_yMargin = 0;
  m_birdEyeView = new PannerView(this);
  m_cvZoom = 1;

  m_exporter.setGraphOptions(this);

  setVScrollBarMode(QScrollView::AlwaysOn);
  setHScrollBarMode(QScrollView::AlwaysOn);
  
  m_birdEyeView->setVScrollBarMode(QScrollView::AlwaysOff);
  m_birdEyeView->setHScrollBarMode(QScrollView::AlwaysOff);
  m_birdEyeView->raise();
  m_birdEyeView->hide();

  setFocusPolicy(QWidget::StrongFocus);
  setBackgroundMode(Qt::NoBackground);
  viewport()->setMouseTracking(true);
  
  connect(this, SIGNAL(contentsMoving(int,int)),
          this, SLOT(contentsMovingSlot(int,int)));
  connect(m_birdEyeView, SIGNAL(zoomRectMoved(int,int)),
          this, SLOT(zoomRectMoved(int,int)));
  connect(m_birdEyeView, SIGNAL(zoomRectMoveFinished()),
          this, SLOT(zoomRectMoveFinished()));

  QWhatsThis::add( this, whatsThis() );

  readViewConfig();
  
  QWMatrix m;
  m.scale(m_zoom,m_zoom);
  setWorldMatrix(m);
}

DotGraphView::~DotGraphView()
{
  saveViewConfig();
  delete m_birdEyeView;
  if (m_popup != 0)
  {
    delete m_popup;
  }
  if (m_canvas) 
  {
    setCanvas(0);
    delete m_canvas;
  }
  if (m_graph != 0)
  {
    delete m_graph;
  }
  delete m_tip;
}

bool DotGraphView::loadDot(const QString& dotFileName)
{
//   std::cerr << "DotGraphView::loadDot " << dotFileName << std::endl;
  QString layoutCommand = (m_graph!=0?m_graph->layoutCommand():"");
  DotGraph* newGraph = new DotGraph(layoutCommand,dotFileName);
  if (layoutCommand.isEmpty())
  {
    layoutCommand = newGraph->chooseLayoutProgramForFile(newGraph->dotFileName());
  }
  newGraph->layoutCommand(layoutCommand);
  
//   std::cerr << "Parsing " << newGraph->dotFileName() << " with " << newGraph->layoutCommand() << std::endl;
  if (newGraph->parseDot(newGraph->dotFileName()))
  {
//     kdDebug() << "Successfully parsed!" << endl;
  }
  else
  {
    KMessageBox::error(this, i18n("%1 NOT successfully parsed!.").arg(dotFileName));
    kdDebug() << "NOT successfully parsed!" << endl;
    return false;
  }

  if (m_graph != 0) 
    delete m_graph;
  m_graph = newGraph;

  hide();
  if (m_canvas) 
  {
    delete m_canvas;
    m_canvas = 0;
  }
  m_birdEyeView->setCanvas(0);
  setCanvas(0);
  
  viewport()->setUpdatesEnabled(false);
  
  if (m_graph->nodes().size() > KGV_MAX_PANNER_NODES)
  {
    m_birdEyeView->setDrawingEnabled(false);
  }
  //  QCanvasEllipse* eItem;
  double scaleX = 1.0, scaleY = 1.0;

  if (_detailLevel == 0)      { scaleX = m_graph->scale() * 0.7; scaleY = m_graph->scale() * 0.7; }
  else if (_detailLevel == 1) { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
  else if (_detailLevel == 2) { scaleX = m_graph->scale() * 1.3; scaleY = m_graph->scale() * 1.3; }
  else                        { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
  
  int gh = int(m_graph->height());
//   kdDebug() << "detail level = " << _detailLevel << " ; scaleX = " << scaleX << endl;
  if (!m_canvas) 
  {
    int w = (int)(scaleX * m_graph->width() / m_graph->horizCellFactor());
    int h = (int)(scaleY * gh / m_graph->vertCellFactor());
    
    m_xMargin = 50;
    m_yMargin = 50;
    
    m_canvas = new QCanvas(int(w+2*m_xMargin), int(h+2*m_yMargin));
    m_canvas->setBackgroundColor(m_graph->backgroundColor());
    m_canvasNaturalSize = m_canvas->size();
    QSize newCanvasSize = m_canvas->size();
    if (newCanvasSize.width() < visibleWidth())
    {
      newCanvasSize.setWidth(visibleWidth());
    }
    else if (visibleWidth() < m_canvasNaturalSize.width())
    {
      newCanvasSize.setWidth(m_canvasNaturalSize.width());
    }
    if (newCanvasSize.height() < visibleHeight())
    {
      newCanvasSize.setHeight(visibleHeight());
    }
    else if (visibleHeight() < m_canvasNaturalSize.height())
    {
      newCanvasSize.setHeight(m_canvasNaturalSize.height());
    }
    if (newCanvasSize != m_canvas->size())
    {
      m_canvas->resize(newCanvasSize.width(), newCanvasSize.height());
    }
  }
  
//   std::set< GraphNode* >& nodes = m_graph->nodesOfCell(0);
  GraphNodeMap& nodes = m_graph->nodes();
//   std::cerr << "Adding " << nodes.size() << " nodes to canvas" << std::endl;
  uint nbNodesShown = 0;
  GraphNodeMap::iterator nodesIt, nodesIt_end;
  nodesIt = nodes.begin();
  nodesIt_end = nodes.end();
  for (; nodesIt != nodesIt_end; nodesIt++)
  {
    GraphNode* gnode = (*nodesIt);
    
    DotRenderOpVec::const_iterator it, it_end;
    it = gnode->renderOperations().begin(); it_end = gnode->renderOperations().end();
    for (; it != it_end; it++)
    {
      if ((*it).renderop != "T" || gnode->shape() == "plaintext")
      {
        CanvasNode *cnode = CanvasNode::dotShapedCanvasNode(
          *it, gnode->renderOperations(), this, gnode, m_canvas, 
          scaleX, scaleY, m_xMargin, m_yMargin, gh, 
          int(m_graph->wdhcf()), int(m_graph->hdvcf()));
        if (cnode == 0) continue;
        
        
        gnode->setCanvasNode(cnode);
        if (dynamic_cast<QCanvasPolygonalItem*>(cnode) != 0)
        {
//           std::cerr << "setting z = " << gnode->z() << std::endl;
          dynamic_cast<QCanvasPolygonalItem*>(cnode)->setZ(gnode->z());
          dynamic_cast<QCanvasPolygonalItem*>(cnode)->show();
          nbNodesShown++;
        }
        break;
      }
    }
  }
//   std::cerr << "  " << nbNodesShown << " nodes shown" << std::endl;
  
  uint nbEdgesShown = 0;
  GraphEdgeMap::iterator edgesIt, edgesIt_end;
  edgesIt = m_graph->edges().begin();
  edgesIt_end = m_graph->edges().end();
  for (; edgesIt != edgesIt_end; edgesIt++)
  {
    GraphEdge* gedge = (*edgesIt).second;
    
/*    if (!( ( m_graph->nodesOfCell(0).find (e->fromNode()) != m_graph->nodesOfCell(0).end() )
           && ( m_graph->nodesOfCell(0).find (e->toNode()) != m_graph->nodesOfCell(0).end() ) ))
      continue;*/
    
    CanvasEdge* cedge = new CanvasEdge(gedge, m_canvas,scaleX, scaleY, m_xMargin, m_yMargin, gh, 
                           int(m_graph->wdhcf()), int(m_graph->hdvcf()));
    
    gedge->setCanvasEdge(cedge);
//     std::cerr << "setting z = " << gedge->z() << std::endl;
    cedge->setZ(gedge->z());
    cedge->show();
    nbEdgesShown++;
  }
//   std::cerr << "  " << nbEdgesShown << " edges shown" << std::endl;
  
  uint nbSubgraphsShown = 0;
  GraphSubgraphMap::iterator subgraphsIt, subgraphsIt_end;
  subgraphsIt = m_graph->subgraphs().begin();
  subgraphsIt_end = m_graph->subgraphs().end();
  for (; subgraphsIt != subgraphsIt_end; subgraphsIt++)
  {
    GraphSubgraph* gsubgraph = (*subgraphsIt);
    
/*    if (!( ( m_graph->nodesOfCell(0).find (e->fromNode()) != m_graph->nodesOfCell(0).end() )
    && ( m_graph->nodesOfCell(0).find (e->toNode()) != m_graph->nodesOfCell(0).end() ) ))
    continue;*/
    
    CanvasSubgraph* csubgraph = new CanvasSubgraph(this, gsubgraph, 
        m_canvas,scaleX, scaleY, m_xMargin, m_yMargin, gh, 
        int(m_graph->wdhcf()), int(m_graph->hdvcf()));
    
    csubgraph->show();
    nbSubgraphsShown++;
  }

//   std::cerr << "Adding graph render operations: " << m_graph->renderOperations().size() << std::endl;
  DotRenderOpVec::const_iterator graphRenderOpsIt, graphRenderOpsIt_end;
  graphRenderOpsIt = m_graph->renderOperations().begin();
  graphRenderOpsIt_end = m_graph->renderOperations().end();
  for (; graphRenderOpsIt != graphRenderOpsIt_end; graphRenderOpsIt++)
  {
    const DotRenderOp& dro = (*graphRenderOpsIt);
    if ( dro.renderop == "T" )
    {
//       std::cerr << "Adding graph label '"<<dro.str<<"'" << std::endl;
      QString str = QString::fromUtf8(dro.str.c_str());
      int stringWidthGoal = int(dro.integers[3] * scaleX);
      int fontSize = m_graph->fontSize();
      QFont* font = FontsCache::changeable().fromName(m_graph->fontName().c_str());
      font->setPointSize(fontSize);
      QFontMetrics fm(*font);
      while (fm.width(str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        font->setPointSize(fontSize);
        fm = QFontMetrics(*font);
      }
      QCanvasText* labelView = new QCanvasText(str, *font, m_canvas);
      labelView->setX(
                  int((scaleX * 
                       (
                         (dro.integers[0]) 
                         + (((dro.integers[2])*(dro.integers[3]))/2)
                         - ( (dro.integers[3])/2 )
                       )
                      )
                      + m_xMargin ));
      labelView->setY(
                  int(((gh - (dro.integers[1]))*scaleY)+ m_yMargin));
      labelView->setColor(Dot2QtConsts::instance().qtColor(m_graph->fontColor()));
      labelView->setFont(*font);
      m_labelViews.insert(labelView);
    }
  }
  
  m_birdEyeView->setCanvas(m_canvas);
//   std::cerr << "After m_birdEyeView set canvas" << std::endl;
  
  setCanvas(m_canvas);
//   std::cerr << "After set canvas" << std::endl;
  

  m_cvZoom = 0;
  updateSizes();
  
  viewport()->setUpdatesEnabled(true);
  show();
  std::set<QCanvasText*>::iterator labelViewsIt, labelViewsIt_end;
  labelViewsIt = m_labelViews.begin(); labelViewsIt_end = m_labelViews.end();
  for (; labelViewsIt != labelViewsIt_end; labelViewsIt++)
  {
    (*labelViewsIt)->show();
  }
//   m_canvas->update();
  return true;
}


QString DotGraphView::whatsThis() const
{
    return i18n( "<h1>GraphViz dot format graph visualization</h1>"
      "<p>If the graph is larger than the widget area, an overview "
      "panner is shown in one edge. Choose throug the context menu "
      "if the optimal position of this overview should be automatically "
      "computed or put it where you want.</p>"
      "<h2>How to work with it ?</h2>"
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
      "<li>To zoom, you can either use the zoom in and zoom out toolbar butons or click on the <Shift> key while rolling your mouse wheel.</li>"
      "<li>Try the contextual menu (usually by right-clicking) to discover other "
      "possibilities.</li>"
      "<li>Try the <tt>Print preview</tt> or the <tt>Page setup</tt> buttons to explore the printing options.</li>"
      "</ul>"
    );
}

void DotGraphView::updateSizes(QSize s)
{
    if (s == QSize(0,0)) s = size();

    // the part of the canvas that should be visible
    int cWidth  = int((m_canvas->width()  - 2*m_xMargin + 100));
    int cHeight = int((m_canvas->height() - 2*m_yMargin + 100));

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

      QWMatrix wm;
      wm.scale( zoom, zoom );
      m_birdEyeView->setWorldMatrix(wm);

      // make it a little bigger to compensate for widget frame
      m_birdEyeView->resize(int(cWidth * zoom) + 4,
                            int(cHeight * zoom) + 4);

      // update ZoomRect in completeView
      contentsMovingSlot(contentsX(), contentsY());
    }

    m_birdEyeView->setContentsPos(int(zoom*(m_xMargin-50)),
          int(zoom*(m_yMargin-50)));

    int cvW = m_birdEyeView->width();
    int cvH = m_birdEyeView->height();
    int x = width()- cvW - verticalScrollBar()->width()    -2;
    int y = height()-cvH - horizontalScrollBar()->height() -2;
    QPoint oldZoomPos = m_birdEyeView->pos();
    QPoint newZoomPos = QPoint(0,0);
    ZoomPosition zp = m_zoomPosition;
    if (zp == Auto) 
    {
      QPoint tl1Pos = viewportToContents(QPoint(0,0));
      QPoint tl2Pos = viewportToContents(QPoint(cvW,cvH));
      QPoint tr1Pos = viewportToContents(QPoint(x,0));
      QPoint tr2Pos = viewportToContents(QPoint(x+cvW,cvH));
      QPoint bl1Pos = viewportToContents(QPoint(0,y));
      QPoint bl2Pos = viewportToContents(QPoint(cvW,y+cvH));
      QPoint br1Pos = viewportToContents(QPoint(x,y));
      QPoint br2Pos = viewportToContents(QPoint(x+cvW,y+cvH));
      int tlCols = m_canvas->collisions(QRect(tl1Pos,tl2Pos)).count();
      int trCols = m_canvas->collisions(QRect(tr1Pos,tr2Pos)).count();
      int blCols = m_canvas->collisions(QRect(bl1Pos,bl2Pos)).count();
      int brCols = m_canvas->collisions(QRect(br1Pos,br2Pos)).count();
      int minCols = tlCols;
      zp = m_lastAutoPosition;
      switch(zp) 
      {
        case TopRight:    minCols = trCols; break;
        case BottomLeft:  minCols = blCols; break;
        case BottomRight: minCols = brCols; break;
        default:
        case TopLeft:     minCols = tlCols; break;
      }
      if (minCols > tlCols) { minCols = tlCols; zp = TopLeft; }
      if (minCols > trCols) { minCols = trCols; zp = TopRight; }
      if (minCols > blCols) { minCols = blCols; zp = BottomLeft; }
      if (minCols > brCols) { minCols = brCols; zp = BottomRight; }
    
      m_lastAutoPosition = zp;
    }

    switch(zp) 
    {
      case TopRight:
        newZoomPos = QPoint(x,0);
      break;
      case BottomLeft:
        newZoomPos = QPoint(0,y);
      break;
      case BottomRight:
        newZoomPos = QPoint(x,y);
      break;
      default:
      break;
    }
    if (newZoomPos != oldZoomPos) 
      m_birdEyeView->move(newZoomPos);
  m_birdEyeView->show();
  QSize newCanvasSize = m_canvas->size();
  if (newCanvasSize.width() < visibleWidth())
  {
    newCanvasSize.setWidth(visibleWidth());
  }
  else if (visibleWidth() < m_canvasNaturalSize.width())
  {
    newCanvasSize.setWidth(m_canvasNaturalSize.width());
  }
  if (newCanvasSize.height() < visibleHeight())
  {
    newCanvasSize.setHeight(visibleHeight());
  }
  else if (visibleHeight() < m_canvasNaturalSize.height())
  {
    newCanvasSize.setHeight(m_canvasNaturalSize.height());
  }
  if (newCanvasSize != m_canvas->size())
  {
    m_canvas->resize(newCanvasSize.width(), newCanvasSize.height());
  }
//   std::cerr << "done." << std::endl;
}

void DotGraphView::focusInEvent(QFocusEvent*)
{
  if (!m_canvas) return;

//   m_canvas->update();
}

void DotGraphView::focusOutEvent(QFocusEvent* e)
{
  // trigger updates as in focusInEvent
  focusInEvent(e);
}

void DotGraphView::keyPressEvent(QKeyEvent* e)
{
  if (!m_canvas) 
  {
    e->ignore();
    return;
  }

  // move canvas...
  if (e->key() == Key_Home)
    scrollBy(-m_canvas->width(),0);
  else if (e->key() == Key_End)
    scrollBy(m_canvas->width(),0);
  else if (e->key() == Key_Prior)
    scrollBy(0,-visibleHeight()/2);
  else if (e->key() == Key_Next)
    scrollBy(0,visibleHeight()/2);
  else if (e->key() == Key_Left)
    scrollBy(-visibleWidth()/10,0);
  else if (e->key() == Key_Right)
    scrollBy(visibleWidth()/10,0);
  else if (e->key() == Key_Down)
    scrollBy(0,visibleHeight()/10);
  else if (e->key() == Key_Up)
    scrollBy(0,-visibleHeight()/10);
  else 
  {
    e->ignore();
    return;
  }
//   m_canvas->update();
}

void DotGraphView::wheelEvent(QWheelEvent* e)
{
  if (!m_canvas) 
  {
    e->ignore();
    return;
  }
  e->accept();
  if (e->state() == ShiftButton)
  {
    // move canvas...
    double factor;
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
    if (e->orientation() == Qt::Horizontal)
    {
      if (e->delta() < 0)
      {
        scrollBy(-visibleWidth()/10,0);
      }
      else
      {
        scrollBy(visibleWidth()/10,0);
      }
    }
    else
    {
      if (e->delta() < 0)
      {
        scrollBy(0,visibleHeight()/10);
      }
      else
      {
        scrollBy(0,-visibleHeight()/10);
      }
    }
//       m_canvas->update();
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

void DotGraphView::applyZoom(double factor)
{
  double oldZoom = m_zoom;
  double newZoom = m_zoom * factor;
  if (newZoom < 0.1 || newZoom > 10)
    return;
  m_zoom = newZoom;
  if (m_zoom > 1.0 && m_zoom < 1.1) 
  {
    m_zoom = 1;
  }
  int centerX = int((contentsX() + (visibleWidth() / 2))*factor);
  int centerY = int((contentsY() + (visibleHeight() / 2))*factor);
  
  setUpdatesEnabled(false);
  QWMatrix m;
  m.scale(m_zoom,m_zoom);
  setWorldMatrix(m);
  center(centerX, centerY);
  emit zoomed(m_zoom);
  setUpdatesEnabled(true);
  updateSizes();
/*  m_canvas->update();
  update();*/
}

void DotGraphView::resizeEvent(QResizeEvent* e)
{
//   std::cerr << "resizeEvent" << std::endl;
  QCanvasView::resizeEvent(e);
  if (m_canvas) updateSizes(e->size());
//   std::cerr << "resizeEvent end" << std::endl;
}

void DotGraphView::contentsMovingSlot(int x, int y)
{
  QRect z(int(x * m_cvZoom / m_zoom), int(y * m_cvZoom / m_zoom),
          int(visibleWidth() * m_cvZoom / m_zoom)-1, int(visibleHeight() * m_cvZoom / m_zoom)-1);
  if (0) qDebug("moving: (%d,%d) => (%d/%d - %dx%d)",
                x, y, z.x(), z.y(), z.width(), z.height());
  m_birdEyeView->setZoomRect(z);
}

void DotGraphView::zoomRectMoved(int dx, int dy)
{
  if (leftMargin()>0) dx = 0;
  if (topMargin()>0) dy = 0;
  scrollBy(int(dx/m_cvZoom*m_zoom),int(dy/m_cvZoom*m_zoom));
//   m_canvas->update();
}

void DotGraphView::zoomRectMoveFinished()
{
//   std::cerr << "zoomRectMoveFinished" << std::endl;
    if (m_zoomPosition == Auto) updateSizes();
//   std::cerr << "zoomRectMoveFinished end" << std::endl;
}

void DotGraphView::contentsMousePressEvent(QMouseEvent* e)
{
  // clicking on the viewport sets focus
  setFocus();

  m_isMoving = true;
  setCursor(KCursor::handCursor());
  m_lastPos = e->globalPos();
}

void DotGraphView::contentsMouseMoveEvent(QMouseEvent* e)
{
//   std::cerr << "contentsMouseMoveEvent" << std::endl;
  if (m_isMoving) 
  {
    setCursor(KCursor::crossCursor());
/*    if (m_focusedNode != 0)
    {
      std::cerr << "removing focus from " << m_focusedNode->node()->id() << std::endl;
      QCanvasItem* cn = dynamic_cast<QCanvasItem*>(m_focusedNode);
      if (cn != 0)
      {
        cn->hide();
      }
      m_focusedNode->hasFocus(false);
      if (cn != 0)
      {
        cn->show();
      }
//       update();
      m_focusedNode = 0;
    }*/
    
    int dx = e->globalPos().x() - m_lastPos.x();
    int dy = e->globalPos().y() - m_lastPos.y();
    scrollBy(-dx, -dy);
    m_lastPos = e->globalPos();
//     m_canvas->update();
//     update();
  }
/*  else
  {
//     std::cerr << "    not moving" << std::endl;
    
    QCanvasItemList l = m_canvas->collisions((e->pos()));
    if (l.count() == 0) 
    {
      if (m_focusedNode != 0)
      {
//         std::cerr << "removing focus from " << m_focusedNode->node()->id() << std::endl;
        QCanvasItem* cn = dynamic_cast<QCanvasItem*>(m_focusedNode);
        if (cn != 0)
        {
          cn->hide();
        }
        m_focusedNode->hasFocus(false);
        if (cn != 0)
        {
          cn->show();
        }
//         update();
        m_focusedNode = 0;
      }
    }
    else if (m_focusedNode != 0 && !m_focusedNode->node()->url().isEmpty())
    {
      QCanvasItem* item = l.first();
      
      CanvasPolygonalNode* cn = dynamic_cast<CanvasPolygonalNode*>(item);
      if (cn != 0 && cn->node() != 0 && cn != m_focusedNode) 
      {
        if (m_focusedNode != 0)
        {
//           std::cerr << "removing focus from " << m_focusedNode->node()->id() << std::endl;
          QCanvasItem* cn = dynamic_cast<QCanvasItem*>(m_focusedNode);
          if (cn != 0)
          {
            cn->hide();
          }
          m_focusedNode->hasFocus(false);
          if (cn != 0)
          {
            cn->show();
          }
//           update();
          m_focusedNode = 0;
        }
//         std::cerr << "setting focus on " << cn->node()->id() << std::endl;
        m_focusedNode = cn;
        m_focusedNode->hasFocus(true);
        QCanvasItem* cn = dynamic_cast<QCanvasItem*>(m_focusedNode);
        if (cn != 0)
        {
          cn->hide();
          cn->show();
        }
//         update();
//         m_canvas->update();
//         update();
      }
    }
//     m_canvas->update();
//     update();
  }*/
}

void DotGraphView::contentsMouseReleaseEvent(QMouseEvent*)
{
//   std::cerr << "contentsMouseReleaseEvent" << std::endl;
  m_isMoving = false;
  setCursor(Qt::ArrowCursor);
  if (m_zoomPosition == Auto) updateSizes();
  if (m_focusedNode != 0 && !m_focusedNode->node()->url().isEmpty())
  {
    KURL url(m_focusedNode->node()->url());
    new KRun(url);
  }
//   std::cerr << "contentsMouseReleaseEvent end" << std::endl;
}

void DotGraphView::contentsMouseDoubleClickEvent(QMouseEvent* e)
{

}

void DotGraphView::contentsContextMenuEvent(QContextMenuEvent* e)
{
  QCanvasItemList l = canvas()->collisions(e->pos());

  setupPopup();

  int r = m_popup->exec(e->globalPos());

  switch(r) 
  {
  case POPUP_EXPORT_IMAGE:
    exportToImage();
    break;
    
  case POPUP_LAYOUT_SPECIFY:    
    {
      bool ok = false;
      QString currentLayoutCommand = m_graph->layoutCommand();
      QString layoutCommand = 
        KInputDialog::text(
                            i18n("Layout Command"), 
                            i18n("Type in a layout command for the current graph:"), 
                            currentLayoutCommand, 
                            &ok, 
                            this, 
                            "chooseLayoutProgram", 
                            0, 
                            QString::null, 
                            i18n("Specify here the command that will be used to layout the graph.\n"
                                 "The command MUST write its results on stdout in xdot format."));
//       std::cerr << "Got layout command: " << layoutCommand << std::endl;
      if (ok && layoutCommand != currentLayoutCommand)
      {
//         std::cerr << "Setting new layout command: " << layoutCommand << std::endl;
        setLayoutCommand(layoutCommand);
      }
    }
    break;

  case POPUP_LAYOUT_RESET:
    {
      setLayoutCommand("");
    }
    break;
    
  case POPUP_LAYOUT_DOT:    
    {
      setLayoutCommand("dot -Txdot");
    }
    break;
    
  case POPUP_LAYOUT_NEATO:    
    {
      setLayoutCommand("neato -Txdot");
    }
    break;
    
  case POPUP_LAYOUT_TWOPI:    
    {
      setLayoutCommand("twopi -Txdot");
    }
    break;

  case POPUP_LAYOUT_FDP:    
    {
      setLayoutCommand("fdp -Txdot");
    }
    break;
  case POPUP_LAYOUT_CIRCO:    
    {
      setLayoutCommand("circo -Txdot");
    }
    break;
    
  case POPUP_BEV_ENABLED:
    viewBevEnabledToggled(!m_popup->isItemChecked(POPUP_BEV_ENABLED));
    break;
  case POPUP_BEV_TOPLEFT: viewBevActivated(TopLeft); break;
  case POPUP_BEV_TOPRIGTH: viewBevActivated(TopRight); break;
  case POPUP_BEV_BOTTOMLEFT: viewBevActivated(BottomLeft); break;
  case POPUP_BEV_BOTTOMRIGHT: viewBevActivated(BottomRight); break;
  case POPUP_BEV_AUTOMATIC: viewBevActivated(Auto); break;

  default: break;
  }

}

void DotGraphView::setLayoutCommand(const QString& command)
{
  m_graph->layoutCommand(command);
  reload();
}

DotGraphView::ZoomPosition DotGraphView::zoomPos(const QString& s)
{
  DotGraphView::ZoomPosition  res = DEFAULT_ZOOMPOS;
  if (s == QString("TopLeft")) res = TopLeft;
  if (s == QString("TopRight")) res = TopRight;
  if (s == QString("BottomLeft")) res = BottomLeft;
  if (s == QString("BottomRight")) res = BottomRight;
  if (s == QString("Automatic")) res = Auto;

  return res;
}

void DotGraphView::viewBevActivated(int newZoomPos)
{
  m_zoomPosition = (ZoomPosition)newZoomPos;
  updateSizes();
  emit(sigViewBevActivated(newZoomPos));
}

QString DotGraphView::zoomPosString(ZoomPosition p)
{
    if (p == TopRight) return QString("TopRight");
    if (p == BottomLeft) return QString("BottomLeft");
    if (p == BottomRight) return QString("BottomRight");
    if (p == Auto) return QString("Automatic");

    return QString("TopLeft");
}

void DotGraphView::readViewConfig()
{
  KConfigGroup g(KGlobal::config(),"GraphViewLayout");
  
  _detailLevel     = g.readNumEntry("DetailLevel", DEFAULT_DETAILLEVEL);
  _layout          = GraphOptions::layout(g.readEntry("Layout",
            layoutString(DEFAULT_LAYOUT)));
  m_zoomPosition  = zoomPos(g.readEntry("ZoomPosition",
            zoomPosString(DEFAULT_ZOOMPOS)));
  emit(sigViewBevActivated(m_zoomPosition));
}

void DotGraphView::saveViewConfig()
{
//   kdDebug() << "Saving view config" << endl;  
  KConfigGroup g(KGlobal::config(), "GraphViewLayout");

    writeConfigEntry(&g, "DetailLevel", _detailLevel, DEFAULT_DETAILLEVEL);
    writeConfigEntry(&g, "Layout",
         layoutString(_layout), layoutString(DEFAULT_LAYOUT).utf8().data());
    writeConfigEntry(&g, "ZoomPosition",
         zoomPosString(m_zoomPosition),
         zoomPosString(DEFAULT_ZOOMPOS).utf8().data());
  g.sync();
}

void DotGraphView::pageSetup()
{
  if (m_printCommand == 0)
  {
    m_printCommand = new KGVSimplePrintingCommand(this, 0);
  }
  m_printCommand->showPageSetup(m_graph->dotFileName());
  return;
}

void DotGraphView::print()
{
  if (m_printCommand == 0)
  {
    m_printCommand = new KGVSimplePrintingCommand(this, 0);
  }
  m_printCommand->print(m_graph->dotFileName());
  return;
}

void DotGraphView::printPreview()
{
  if (m_printCommand == 0)
  {
    m_printCommand = new KGVSimplePrintingCommand(this, 0);
  }
  m_printCommand->showPrintPreview(m_graph->dotFileName(), false);
  return;
}

bool DotGraphView::reload()
{
  QString fileName = m_graph->dotFileName();
  return loadDot(fileName);
}

void DotGraphView::dirty(const QString& dotFileName)
{
//   std::cerr << "SLOT dirty for " << dotFileName << std::endl;
  if (dotFileName == m_graph->dotFileName())
  {
    if (KMessageBox::questionYesNo(this, 
                                i18n("The file %1 has been modified on disk.\nDo you want to reload it ?").arg(dotFileName),
                                i18n("Reload Confirmation"),
                                KStdGuiItem::yes(),
                                KStdGuiItem::no(),
                                "reloadOnChangeMode"   ) == KMessageBox::Yes)
    {
      loadDot(dotFileName);
    }
  }
}

KConfigGroup* DotGraphView::configGroup(KConfig* c,
                                         const QString& group, const QString& post)
{
  QString resGroup = group;
  QStringList gList = c->groupList();
  if (gList.contains((group+post).ascii()) ) resGroup += post;
  return new KConfigGroup(c, group);
}

void DotGraphView::writeConfigEntry(KConfigBase* c, const char* pKey,
                                     const QString& value, const char* def, bool bNLS)
{
  if (!c) return;
  if ((value.isEmpty() && ((def == 0) || (*def == 0))) ||
      (value == QString(def)))
    c->deleteEntry(pKey);
  else
    c->writeEntry(pKey, value, true, false, bNLS);
}

void DotGraphView::writeConfigEntry(KConfigBase* c, const char* pKey,
                                     int value, int def)
{
  if (!c) return;
  if (value == def)
    c->deleteEntry(pKey);
  else
    c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigBase* c, const char* pKey,
                                     double value, double def)
{
  if (!c) return;
  if (value == def)
    c->deleteEntry(pKey);
  else
    c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigBase* c, const char* pKey,
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
  return m_graph->dotFileName();
}

void DotGraphView::hideToolsWindows()
{
  if (m_printCommand != 0)
  {
    m_printCommand->hidePageSetup();
    m_printCommand->hidePrintPreview();
  }
}

void DotGraphView::setupPopup()
{
  if (m_popup != 0)
  {
    return;
  }
  m_popup = new QPopupMenu();

  QPopupMenu* layoutPopup = new QPopupMenu();
  layoutPopup->insertItem(i18n("Dot"), POPUP_LAYOUT_DOT);
  layoutPopup->insertItem(i18n("Neato"), POPUP_LAYOUT_NEATO);
  layoutPopup->insertItem(i18n("Twopi"), POPUP_LAYOUT_TWOPI);
  layoutPopup->insertItem(i18n("Fdp"), POPUP_LAYOUT_FDP);
  layoutPopup->insertItem(i18n("Circo"), POPUP_LAYOUT_CIRCO);
  layoutPopup->insertItem(i18n("Specify layout command"), POPUP_LAYOUT_SPECIFY);
  layoutPopup->insertItem(i18n("Reset layout command to default"), POPUP_LAYOUT_RESET);
  
  m_popup->insertItem(i18n("Layout"), layoutPopup, POPUP_LAYOUT);
  m_popup->insertSeparator();
  
  QPopupMenu* epopup = new QPopupMenu();
  epopup->insertItem(i18n("As Image ..."), POPUP_EXPORT_IMAGE);

  m_popup->insertItem(i18n("Export Graph"), epopup, POPUP_EXPORT);
  m_popup->insertSeparator();


  QPopupMenu* opopup = new QPopupMenu();

  opopup->insertItem(i18n("Top Left"), POPUP_BEV_TOPLEFT);
  opopup->insertItem(i18n("Top Right"), POPUP_BEV_TOPRIGTH);
  opopup->insertItem(i18n("Bottom Left"), POPUP_BEV_BOTTOMLEFT);
  opopup->insertItem(i18n("Bottom Right"), POPUP_BEV_BOTTOMRIGHT);
  opopup->insertItem(i18n("Automatic"), POPUP_BEV_AUTOMATIC);
  opopup->setItemChecked(POPUP_BEV_TOPLEFT,m_zoomPosition == TopLeft);
  opopup->setItemChecked(POPUP_BEV_TOPRIGTH,m_zoomPosition == TopRight);
  opopup->setItemChecked(POPUP_BEV_BOTTOMLEFT,m_zoomPosition == BottomLeft);
  opopup->setItemChecked(POPUP_BEV_BOTTOMRIGHT,m_zoomPosition == BottomRight);
  opopup->setItemChecked(POPUP_BEV_AUTOMATIC,m_zoomPosition == Auto);

  m_popup->insertItem(i18n("Enable Bird's-eye View"), POPUP_BEV_ENABLED);
  m_popup->setAccel(CTRL+Key_B, POPUP_BEV_ENABLED);
  m_popup->insertItem(i18n("Birds-eye View"), opopup, POPUP_BEV);
  m_popup->setCheckable(true);
  m_popup->setItemChecked(POPUP_BEV_ENABLED, KGraphViewerPartSettings::birdsEyeViewEnabled());
  m_popup->setItemEnabled(POPUP_BEV, KGraphViewerPartSettings::birdsEyeViewEnabled());
}

void DotGraphView::viewBevEnabledToggled(bool value)
{
  setupPopup();
  m_popup->setItemChecked(POPUP_BEV_ENABLED, value);
  m_popup->setItemEnabled(POPUP_BEV, value);
  KGraphViewerPartSettings::setBirdsEyeViewEnabled(value);
  KGraphViewerPartSettings::writeConfig();
  updateSizes();
  emit(sigViewBevEnabledToggled(value));
}

void DotGraphView::fileExportActivated(int value)
{
  switch (value)
  {
    case (EXPORT_TO_IMAGE):
      exportToImage();
    break;
    default:;
  }
}

void DotGraphView::exportToImage()
{
  // write current content of canvas as image to file
  if (!m_canvas) return;
    
  QString fn = KFileDialog::getSaveFileName(":","*.png");
  
  if (!fn.isEmpty()) 
  {
    QPixmap pix(m_canvas->size());
    QPainter p(&pix);
    m_canvas->drawArea( m_canvas->rect(), &p );
    pix.save(fn,"PNG");
  }
}

#include "dotgraphview.moc"

