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

// lib
#include "canvasedge.h"
#include "canvassubgraph.h"
#include "canvaselement.h"
#include "dot2qtconsts.h"
#include "dotgraph.h"
#include "FontsCache.h"
#include "pannerview.h"
#include "graphexporter.h"
#include "layoutagraphthread.h"
#include "loadagraphthread.h"
#include "simpleprintingcommand.h"
#include "kgraphviewer_partsettings.h"
#include "kgraphviewerlib_debug.h"
// KF
#include <KActionCollection>
#include <KActionMenu>
#include <KSelectAction>
#include <KToggleAction>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
// Qt
#include <QApplication>
#include <QSvgGenerator>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QLineEdit>
#include <QScrollBar>
#include <QAction>
#include <QImageWriter>
#include <QBitmap>
#include <QTimerEvent>
#include <QMouseEvent>
#include <QContextMenuEvent>
// Std
#include <cmath>
#include <limits>

// DotGraphView defaults

#define DEFAULT_ZOOMPOS KGraphViewerInterface::Auto
#define KGV_MAX_PANNER_NODES 100

namespace KGraphViewer
{
class DotGraphViewPrivate
{
public:
    DotGraphViewPrivate(KActionCollection *actions, DotGraphView *parent)
        : m_labelViews()
        , m_popup(nullptr)
        , m_zoom(1)
        , m_isMoving(false)
        , m_exporter()
        , m_zoomPosition(DEFAULT_ZOOMPOS)
        , m_lastAutoPosition(KGraphViewerInterface::TopLeft)
        , m_graph(nullptr)
        , m_printCommand(nullptr)
        , m_actions(actions)
        , m_detailLevel(DEFAULT_DETAILLEVEL)
        , m_defaultNewElement(nullptr)
        , m_defaultNewElementPixmap(QStringLiteral(":/kgraphviewerpart/pics/newnode.png"))
        , m_editingMode(DotGraphView::None)
        , m_newEdgeSource(nullptr)
        , m_newEdgeDraft(nullptr)
        , m_readWrite(false)
        , m_leavedTimer(std::numeric_limits<int>::max())
        , m_highlighting(false)
        , m_loadThread()
        , m_layoutThread()
        , m_backgroundColor(QColor("white"))
        , q_ptr(parent)
    {
    }
    ~DotGraphViewPrivate()
    {
        delete m_birdEyeView;
        m_birdEyeView = nullptr;
        delete m_popup;
        if (m_canvas) {
            Q_Q(DotGraphView);
            q->setScene(nullptr);
            delete m_canvas;
        }
        delete m_graph;
    }

    void updateSizes(QSizeF s = QSizeF(0, 0));
    void updateBirdEyeView();
    void setupPopup();
    void exportToImage();
    KActionCollection *actionCollection()
    {
        return m_actions;
    }
    double detailAdjustedScale();
    int displaySubgraph(GraphSubgraph *gsubgraph, int zValue, CanvasElement *parent = nullptr);

    QSet<QGraphicsSimpleTextItem *> m_labelViews;
    QGraphicsScene *m_canvas;
    QMenu *m_popup;
    KSelectAction *m_bevPopup;
    KSelectAction *m_layoutAlgoSelectAction;
    int m_xMargin, m_yMargin;
    PannerView *m_birdEyeView;
    double m_cvZoom;
    double m_zoom;
    bool m_isMoving;
    QPoint m_lastPos;

    GraphExporter m_exporter;

    // widget options
    KGraphViewerInterface::PannerPosition m_zoomPosition, m_lastAutoPosition;

    DotGraph *m_graph;

    KGVSimplePrintingCommand *m_printCommand;

    KToggleAction *m_bevEnabledAction;
    KActionCollection *m_actions;

    int m_detailLevel;

    GraphElement *m_defaultNewElement;

    /** image used for a new node just added in an edited graph because this new node has
     *  still no attribute and thus no render operation */
    QPixmap m_defaultNewElementPixmap;
    DotGraphView::EditingMode m_editingMode;

    CanvasElement *m_newEdgeSource;
    QGraphicsLineItem *m_newEdgeDraft;

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

    DotGraphView *const q_ptr;
    Q_DECLARE_PUBLIC(DotGraphView)
};

void DotGraphViewPrivate::updateSizes(QSizeF s)
{
    Q_Q(DotGraphView);
    if (m_canvas == nullptr)
        return;
    if (s == QSizeF(0, 0))
        s = q->size();

    // the part of the canvas that should be visible
    qreal cWidth = m_canvas->width() - 2 * m_xMargin + 100;
    qreal cHeight = m_canvas->height() - 2 * m_yMargin + 100;

    // hide birds eye view if no overview needed
    if ( //!_data || !_activeItem ||
        !KGraphViewerPartSettings::birdsEyeViewEnabled() || (((cWidth * m_zoom) < s.width()) && (cHeight * m_zoom) < s.height())) {
        m_birdEyeView->hide();
        return;
    }
    m_birdEyeView->hide();

    // first, assume use of 1/3 of width/height (possible larger)
    double zoom = .33 * s.width() / cWidth;
    if (zoom * cHeight < .33 * s.height())
        zoom = .33 * s.height() / cHeight;

    // fit to widget size
    if (cWidth * zoom > s.width())
        zoom = s.width() / (double)cWidth;
    if (cHeight * zoom > s.height())
        zoom = s.height() / (double)cHeight;

    // scale to never use full height/width
    zoom = zoom * 3 / 4;

    // at most a zoom of 1/3
    if (zoom > .33)
        zoom = .33;

    if (zoom != m_cvZoom) {
        m_cvZoom = zoom;

        QTransform wm;
        wm.scale(zoom, zoom);
        m_birdEyeView->setTransform(wm);

        // make it a little bigger to compensate for widget frame
        m_birdEyeView->resize((cWidth * zoom) + 4, (cHeight * zoom) + 4);
    }
    updateBirdEyeView();
    m_birdEyeView->setZoomRect(q->mapToScene(q->viewport()->rect()).boundingRect());
    m_birdEyeView->show();
    QSizeF newCanvasSize = m_canvas->sceneRect().size();
    if (newCanvasSize.width() < q->viewport()->width()) {
        newCanvasSize.setWidth(q->viewport()->width());
    } else if (q->viewport()->width() < m_canvas->sceneRect().size().width()) {
        newCanvasSize.setWidth(m_canvas->sceneRect().size().width());
    }
    if (newCanvasSize.height() < q->viewport()->height()) {
        newCanvasSize.setHeight(q->viewport()->height());
    } else if (q->viewport()->height() < m_canvas->sceneRect().size().height()) {
        newCanvasSize.setHeight(m_canvas->sceneRect().size().height());
    }
    //   std::cerr << "done." << std::endl;
}

void DotGraphViewPrivate::updateBirdEyeView()
{
    Q_Q(DotGraphView);
    qreal cvW = m_birdEyeView->width();
    qreal cvH = m_birdEyeView->height();
    qreal x = q->width() - cvW - q->verticalScrollBar()->width() - 2;
    qreal y = q->height() - cvH - q->horizontalScrollBar()->height() - 2;
    QPoint oldZoomPos = m_birdEyeView->pos();
    QPoint newZoomPos = QPoint(0, 0);
    KGraphViewerInterface::PannerPosition zp = m_zoomPosition;
    if (zp == KGraphViewerInterface::Auto) {
        QPointF tl1Pos = q->mapToScene(QPoint(0, 0));
        QPointF tl2Pos = q->mapToScene(QPoint(cvW, cvH));
        QPointF tr1Pos = q->mapToScene(QPoint(x, 0));
        QPointF tr2Pos = q->mapToScene(QPoint(x + cvW, cvH));
        QPointF bl1Pos = q->mapToScene(QPoint(0, y));
        QPointF bl2Pos = q->mapToScene(QPoint(cvW, y + cvH));
        QPointF br1Pos = q->mapToScene(QPoint(x, y));
        QPointF br2Pos = q->mapToScene(QPoint(x + cvW, y + cvH));
        int tlCols = m_canvas->items(QRectF(tl1Pos.x(), tl1Pos.y(), tl2Pos.x(), tl2Pos.y())).count();
        qCDebug(KGRAPHVIEWERLIB_LOG) << tr1Pos.x() << tr1Pos.y() << tr2Pos.x() << tr2Pos.y();
        int trCols = m_canvas->items(QRectF(tr1Pos.x(), tr1Pos.y(), tr2Pos.x(), tr2Pos.y())).count();
        int blCols = m_canvas->items(QRectF(bl1Pos.x(), bl1Pos.y(), bl2Pos.x(), bl2Pos.y())).count();
        int brCols = m_canvas->items(QRectF(br1Pos.x(), br1Pos.y(), br2Pos.x(), br2Pos.y())).count();
        int minCols = tlCols;
        zp = m_lastAutoPosition;
        switch (zp) {
        case KGraphViewerInterface::TopRight:
            minCols = trCols;
            break;
        case KGraphViewerInterface::BottomLeft:
            minCols = blCols;
            break;
        case KGraphViewerInterface::BottomRight:
            minCols = brCols;
            break;
        default:
        case KGraphViewerInterface::TopLeft:
            minCols = tlCols;
            break;
        }
        if (minCols > tlCols) {
            minCols = tlCols;
            zp = KGraphViewerInterface::TopLeft;
        }
        if (minCols > trCols) {
            minCols = trCols;
            zp = KGraphViewerInterface::TopRight;
        }
        if (minCols > blCols) {
            minCols = blCols;
            zp = KGraphViewerInterface::BottomLeft;
        }
        if (minCols > brCols) {
            minCols = brCols;
            zp = KGraphViewerInterface::BottomRight;
        }

        m_lastAutoPosition = zp;
    }

    switch (zp) {
    case KGraphViewerInterface::TopRight:
        newZoomPos = QPoint(x, 0);
        break;
    case KGraphViewerInterface::BottomLeft:
        newZoomPos = QPoint(0, y);
        break;
    case KGraphViewerInterface::BottomRight:
        newZoomPos = QPoint(x, y);
        break;
    default:
        break;
    }
    if (newZoomPos != oldZoomPos)
        m_birdEyeView->move(newZoomPos);
}

double DotGraphViewPrivate::detailAdjustedScale()
{
    double scale = m_graph->scale();
    switch (m_detailLevel) {
    case 0:
        scale *= 0.7;
        break;
    case 2:
        scale *= 1.3;
        break;
    }
    return scale;
}

int DotGraphViewPrivate::displaySubgraph(GraphSubgraph *gsubgraph, int zValue, CanvasElement *parent)
{
    Q_Q(DotGraphView);
    double scale = detailAdjustedScale();

    qreal gh = m_graph->height();

    if (gsubgraph->canvasSubgraph() == nullptr) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating canvas subgraph for" << gsubgraph->id();
        CanvasSubgraph *csubgraph = new CanvasSubgraph(q, gsubgraph, m_canvas, parent);
        csubgraph->initialize(scale, scale, m_xMargin, m_yMargin, gh);
        gsubgraph->setCanvasSubgraph(csubgraph);
        //       csubgraph->setZValue(gsubgraph->z());
        csubgraph->setZValue(zValue += 2);
        csubgraph->show();
        m_canvas->addItem(csubgraph);
        qCDebug(KGRAPHVIEWERLIB_LOG) << " one CanvasSubgraph... Done";
    }
    for (GraphElement *element : gsubgraph->content()) {
        GraphNode *gnode = dynamic_cast<GraphNode *>(element);
        if (gnode->canvasNode() == nullptr) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating canvas node for:" << gnode->id();
            CanvasNode *cnode = new CanvasNode(q, gnode, m_canvas);
            if (cnode == nullptr)
                continue;
            cnode->initialize(scale, scale, m_xMargin, m_yMargin, gh);
            gnode->setCanvasNode(cnode);
            m_canvas->addItem(cnode);
            //       cnode->setZValue(gnode->z());
            cnode->setZValue(zValue + 1);
            cnode->show();
        }
        gnode->canvasNode()->computeBoundingRect();
    }
    gsubgraph->canvasSubgraph()->computeBoundingRect();

    int newZvalue = zValue;
    for (GraphSubgraph *ssg : gsubgraph->subgraphs()) {
        int hereZvalue = displaySubgraph(ssg, zValue, gsubgraph->canvasSubgraph());
        if (hereZvalue > newZvalue)
            newZvalue = hereZvalue;
    }
    return newZvalue;
}

void DotGraphViewPrivate::setupPopup()
{
    Q_Q(DotGraphView);
    if (m_popup) {
        return;
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::setupPopup";
    m_popup = new QMenu();

    m_layoutAlgoSelectAction = new KSelectAction(i18n("Select Layout Algo"), q);
    actionCollection()->addAction(QStringLiteral("view_layout_algo"), m_layoutAlgoSelectAction);

    QAction *lda = new QAction(i18n("Dot"), q);
    lda->setWhatsThis(i18n("Layout the graph using the dot program."));
    actionCollection()->addAction(QStringLiteral("layout_dot"), lda);
    lda->setCheckable(true);

    QAction *lna = new QAction(i18n("Neato"), q);
    lna->setWhatsThis(i18n("Layout the graph using the neato program."));
    actionCollection()->addAction(QStringLiteral("layout_neato"), lna);
    lna->setCheckable(true);

    QAction *lta = new QAction(i18n("Twopi"), q);
    lta->setWhatsThis(i18n("Layout the graph using the twopi program."));
    actionCollection()->addAction(QStringLiteral("layout_twopi"), lta);
    lta->setCheckable(true);

    QAction *lfa = new QAction(i18n("Fdp"), q);
    lfa->setWhatsThis(i18n("Layout the graph using the fdp program."));
    actionCollection()->addAction(QStringLiteral("layout_fdp"), lfa);
    lfa->setCheckable(true);

    QAction *lca = new QAction(i18n("Circo"), q);
    lca->setWhatsThis(i18n("Layout the graph using the circo program."));
    actionCollection()->addAction(QStringLiteral("layout_c"), lca);
    lca->setCheckable(true);

    m_layoutAlgoSelectAction->addAction(lda);
    m_layoutAlgoSelectAction->addAction(lna);
    m_layoutAlgoSelectAction->addAction(lta);
    m_layoutAlgoSelectAction->addAction(lfa);
    m_layoutAlgoSelectAction->addAction(lca);

    m_layoutAlgoSelectAction->setCurrentAction(lda);
    m_layoutAlgoSelectAction->setEditable(true);
    m_layoutAlgoSelectAction->setToolTip(i18n("Choose a Graphviz layout algorithm or edit your own one."));
    m_layoutAlgoSelectAction->setWhatsThis(
        i18n("Choose a Graphviz layout algorithm or type in your own command that will "
             "generate a graph in the xdot format on its standard output. For example, to "
             "manually specify the <tt>G</tt> option to the dot command, type in: "
             "<tt>dot -Gname=MyGraphName -Txdot </tt>"));
    QObject::connect(m_layoutAlgoSelectAction, &KSelectAction::textTriggered, q, &DotGraphView::slotSelectLayoutAlgo);

    QMenu *layoutPopup = m_popup->addMenu(i18n("Layout"));
    layoutPopup->addAction(m_layoutAlgoSelectAction);
    QAction *slc = layoutPopup->addAction(i18n("Specify layout command"), q, &DotGraphView::slotLayoutSpecify);
    slc->setWhatsThis(i18n("Specify yourself the layout command to use. Given a dot file, it should produce an xdot file on its standard output."));
    QAction *rlc = layoutPopup->addAction(i18n("Reset layout command to default"), q, &DotGraphView::slotLayoutReset);
    rlc->setWhatsThis(i18n("Resets the layout command to use to the default depending on the graph type (directed or not)."));

    m_popup->addAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("Zoom In"), q, &DotGraphView::zoomIn);
    m_popup->addAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom Out"), q, &DotGraphView::zoomOut);

    m_popup->addSeparator();

    KActionMenu *file_exportMenu = new KActionMenu(i18n("Export Graph"), q);
    actionCollection()->addAction(QStringLiteral("file_export"), file_exportMenu);
    file_exportMenu->setToolTip(i18n("Allows the graph to be exported in another format."));
    file_exportMenu->setWhatsThis(
        i18n("Use the Export Graph menu to export the graph in another format. "
             "There is currently only one export format supported: as a PNG image."));

    m_popup->addAction(file_exportMenu);
    QAction *exportToImageAction = new QAction(i18n("As Image..."), q);
    exportToImageAction->setWhatsThis(i18n("Export the graph to an image file."));
    actionCollection()->addAction(QStringLiteral("export_image"), exportToImageAction);
    QObject::connect(exportToImageAction, &QAction::triggered, q, &DotGraphView::slotExportImage);

    file_exportMenu->addAction(exportToImageAction);

    m_popup->addSeparator();

    m_bevEnabledAction = new KToggleAction(QIcon(QStringLiteral(":/kgraphviewerpart/pics/bev.png")), i18n("Enable Bird's-eye View"), q);

    actionCollection()->addAction(QStringLiteral("view_bev_enabled"), m_bevEnabledAction);
    actionCollection()->setDefaultShortcut(m_bevEnabledAction, Qt::CTRL | Qt::Key_B);
    m_bevEnabledAction->setWhatsThis(i18n("Enables or disables the Bird's-eye View"));
    QObject::connect(m_bevEnabledAction, &QAction::triggered, q, &DotGraphView::slotBevToggled);
    m_bevEnabledAction->setCheckable(true);
    m_popup->addAction(m_bevEnabledAction);

    m_bevPopup = new KSelectAction(i18n("Birds-eye View"), q);
    m_bevPopup->setWhatsThis(i18n("Allows the Bird's-eye View to be setup."));
    m_popup->addAction(m_bevPopup);
    actionCollection()->addAction(QStringLiteral("view_bev"), m_bevPopup);

    QAction *btla = new QAction(i18n("Top Left"), q);
    btla->setWhatsThis(i18n("Puts the Bird's-eye View in the top-left corner."));
    btla->setCheckable(true);
    actionCollection()->addAction(QStringLiteral("bev_top_left"), btla);
    QObject::connect(btla, &QAction::triggered, q, &DotGraphView::slotBevTopLeft);
    QAction *btra = new QAction(i18n("Top Right"), q);
    btra->setWhatsThis(i18n("Puts the Bird's-eye View in the top-right corner."));
    btra->setCheckable(true);
    actionCollection()->addAction(QStringLiteral("bev_top_right"), btra);
    QObject::connect(btra, &QAction::triggered, q, &DotGraphView::slotBevTopRight);
    QAction *bbla = new QAction(i18n("Bottom Left"), q);
    bbla->setWhatsThis(i18n("Puts the Bird's-eye View in the bottom-left corner."));
    bbla->setCheckable(true);
    actionCollection()->addAction(QStringLiteral("bev_bottom_left"), bbla);
    QObject::connect(bbla, &QAction::triggered, q, &DotGraphView::slotBevBottomLeft);
    QAction *bbra = new QAction(i18n("Bottom Right"), q);
    bbra->setWhatsThis(i18n("Puts the Bird's-eye View in the bottom-right corner."));
    bbra->setCheckable(true);
    actionCollection()->addAction(QStringLiteral("bev_bottom_right"), bbra);
    QObject::connect(bbra, &QAction::triggered, q, &DotGraphView::slotBevBottomRight);
    QAction *bba = new QAction(i18n("Automatic"), q);
    bba->setWhatsThis(i18n("Let KGraphViewer automatically choose the position of the Bird's-eye View."));
    bba->setCheckable(true);
    actionCollection()->addAction(QStringLiteral("bev_automatic"), bba);
    QObject::connect(bba, &QAction::triggered, q, &DotGraphView::slotBevAutomatic);
    m_bevPopup->addAction(btla);
    m_bevPopup->addAction(btra);
    m_bevPopup->addAction(bbla);
    m_bevPopup->addAction(bbra);
    m_bevPopup->addAction(bba);
    switch (m_zoomPosition) {
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

    qCDebug(KGRAPHVIEWERLIB_LOG) << "    m_bevEnabledAction setting checked to : " << KGraphViewerPartSettings::birdsEyeViewEnabled();
    m_bevEnabledAction->setChecked(KGraphViewerPartSettings::birdsEyeViewEnabled());
    m_bevPopup->setEnabled(KGraphViewerPartSettings::birdsEyeViewEnabled());
}

void DotGraphViewPrivate::exportToImage()
{
    // write current content of canvas as image to file
    if (!m_canvas)
        return;

    QStringList writableMimetypes;
    for (const QByteArray &mimeType : QImageWriter::supportedMimeTypes()) {
        writableMimetypes.append(QString::fromLatin1(mimeType));
    }
    const QString svgMimetype = QStringLiteral("image/svg+xml");
    if (!writableMimetypes.contains(svgMimetype)) {
        writableMimetypes.append(svgMimetype);
    }

    QFileDialog fileDialog(nullptr, i18n("Select file"));
    fileDialog.setMimeTypeFilters(writableMimetypes);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (fileDialog.exec() != QFileDialog::Accepted) {
        return;
    }
    const QString fn = fileDialog.selectedFiles().at(0);

    if (!fn.isEmpty()) {
        if (fn.toLower().endsWith(QLatin1String(".svg"))) {
            QSvgGenerator generator;
            generator.setFileName(fn);
            generator.setSize(m_canvas->sceneRect().size().toSize());
            //       generator.setViewBox();
            generator.setTitle(i18n("Graph SVG Generated by KGraphViewer"));
            generator.setDescription(i18n("Graph SVG Generated by KGraphViewer."));
            QPainter painter;
            painter.begin(&generator);
            m_canvas->render(&painter);
            painter.end();
        } else {
            QPixmap pix(m_canvas->sceneRect().size().toSize());
            QPainter p(&pix);
            m_canvas->render(&p);
            pix.save(fn, nullptr, 100);
        }
    }
}

//
// DotGraphView
//
DotGraphView::DotGraphView(KActionCollection *actions, QWidget *parent)
    : QGraphicsView(parent)
    , d_ptr(new DotGraphViewPrivate(actions, this))
{
    // qCDebug(KGRAPHVIEWERLIB_LOG) << "New node pic=" << KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newnode.png");
    Q_D(DotGraphView);
    d->m_canvas = nullptr;
    d->m_xMargin = d->m_yMargin = 0;
    d->m_birdEyeView = new PannerView(this);
    d->m_cvZoom = 1;

    // if there are ever graphic glitches to be found, remove this again
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    d->m_birdEyeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_birdEyeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->m_birdEyeView->raise();
    d->m_birdEyeView->hide();

    setFocusPolicy(Qt::StrongFocus);
    setBackgroundRole(QPalette::Window);
    //   viewport()->setMouseTracking(true);

    connect(d->m_birdEyeView, &PannerView::zoomRectMovedTo, this, &DotGraphView::zoomRectMovedTo);
    connect(d->m_birdEyeView, &PannerView::zoomRectMoveFinished, this, &DotGraphView::zoomRectMoveFinished);

    setWhatsThis(
        i18n("<h1>Graphviz DOT format graph visualization</h1>"
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
             "</ul>"));

    readViewConfig();

    QTransform m;
    m.scale(d->m_zoom, d->m_zoom);
    setTransform(m);
    d->setupPopup();
    setInteractive(true);
    setDragMode(NoDrag);
    setRenderHint(QPainter::Antialiasing);

    connect(&d->m_loadThread, &LoadAGraphThread::finished, this, &DotGraphView::slotAGraphReadFinished);
    connect(&d->m_layoutThread, &LoadAGraphThread::finished, this, &DotGraphView::slotAGraphLayoutFinished);
}

DotGraphView::~DotGraphView()
{
    saveViewConfig();
    Q_D(DotGraphView);
    delete d;
}

KGraphViewerInterface::PannerPosition DotGraphView::zoomPos() const
{
    Q_D(const DotGraphView);
    return d->m_zoomPosition;
}

double DotGraphView::zoom() const
{
    Q_D(const DotGraphView);
    return d->m_zoom;
}
KSelectAction *DotGraphView::bevPopup()
{
    Q_D(DotGraphView);
    return d->m_bevPopup;
}

DotGraph *DotGraphView::graph()
{
    Q_D(DotGraphView);
    return d->m_graph;
}
const DotGraph *DotGraphView::graph() const
{
    Q_D(const DotGraphView);
    return d->m_graph;
}

const GraphElement *DotGraphView::defaultNewElement() const
{
    Q_D(const DotGraphView);
    return d->m_defaultNewElement;
}
QPixmap DotGraphView::defaultNewElementPixmap() const
{
    Q_D(const DotGraphView);
    return d->m_defaultNewElementPixmap;
}

void DotGraphView::setDefaultNewElement(GraphElement *elem)
{
    Q_D(DotGraphView);
    d->m_defaultNewElement = elem;
}
void DotGraphView::setDefaultNewElementPixmap(const QPixmap &pm)
{
    Q_D(DotGraphView);
    d->m_defaultNewElementPixmap = pm;
}

bool DotGraphView::isReadWrite() const
{
    Q_D(const DotGraphView);
    return d->m_readWrite;
}
bool DotGraphView::isReadOnly() const
{
    Q_D(const DotGraphView);
    return !d->m_readWrite;
}

bool DotGraphView::highlighting()
{
    Q_D(DotGraphView);
    return d->m_highlighting;
}
void DotGraphView::setHighlighting(bool highlightingValue)
{
    Q_D(DotGraphView);
    d->m_highlighting = highlightingValue;
}

DotGraphView::EditingMode DotGraphView::editingMode() const
{
    Q_D(const DotGraphView);
    return d->m_editingMode;
}

void DotGraphView::setBackgroundColor(const QColor &color)
{
    Q_D(DotGraphView);
    d->m_backgroundColor = color;
    d->m_canvas->setBackgroundBrush(QBrush(d->m_backgroundColor));
}

bool DotGraphView::initEmpty()
{
    Q_D(DotGraphView);
    d->m_birdEyeView->hide();
    d->m_birdEyeView->setScene(nullptr);

    if (d->m_canvas) {
        delete d->m_canvas;
        d->m_canvas = nullptr;
    }

    delete d->m_graph;
    d->m_graph = new DotGraph();
    connect(d->m_graph, &DotGraph::readyToDisplay, this, &DotGraphView::displayGraph);

    if (d->m_readWrite) {
        d->m_graph->setReadWrite();
    }

    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Parsing " << m_graph->dotFileName() << " with " << m_graph->layoutCommand();
    d->m_xMargin = 50;
    d->m_yMargin = 50;

    QGraphicsScene *newCanvas = new QGraphicsScene();
    QGraphicsSimpleTextItem *item = newCanvas->addSimpleText(i18n("no graph loaded"));
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Created canvas " << newCanvas;

    d->m_birdEyeView->setScene(newCanvas);
    //   std::cerr << "After m_birdEyeView set canvas" << std::endl;

    setScene(newCanvas);
    d->m_canvas = newCanvas;
    centerOn(item);

    d->m_cvZoom = 0;

    return true;
}

bool DotGraphView::slotLoadLibrary(graph_t *graph)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "graph_t";
    Q_D(DotGraphView);
    d->m_birdEyeView->setScene(nullptr);

    if (d->m_canvas) {
        d->m_canvas->deleteLater();
        d->m_canvas = nullptr;
    }

    QString layoutCommand = (d->m_graph ? d->m_graph->layoutCommand() : QString());
    delete d->m_graph;

    if (layoutCommand.isEmpty())
        layoutCommand = QStringLiteral("dot");

    qCDebug(KGRAPHVIEWERLIB_LOG) << "layoutCommand:" << layoutCommand;
    d->m_graph = new DotGraph(layoutCommand, QString());
    d->m_graph->setUseLibrary(true);

    connect(d->m_graph, &DotGraph::readyToDisplay, this, &DotGraphView::displayGraph);
    connect(this, &DotGraphView::removeEdge, d->m_graph, &DotGraph::removeEdge);
    connect(this, &DotGraphView::removeNodeNamed, d->m_graph, &DotGraph::removeNodeNamed);
    connect(this, &DotGraphView::removeElement, d->m_graph, &DotGraph::removeElement);

    if (d->m_readWrite) {
        d->m_graph->setReadWrite();
    }

    if (layoutCommand.isEmpty()) {
        layoutCommand = d->m_graph->chooseLayoutProgramForFile(d->m_graph->dotFileName());
    }
    d->m_graph->layoutCommand(layoutCommand);

    GVC_t *gvc = gvContext();
    threadsafe_wrap_gvLayout(gvc, graph, layoutCommand.toUtf8().data());
    threadsafe_wrap_gvRender(gvc, graph, "xdot", nullptr);

    d->m_xMargin = 50;
    d->m_yMargin = 50;

    QGraphicsScene *newCanvas = new QGraphicsScene();
    qCDebug(KGRAPHVIEWERLIB_LOG) << "Created canvas " << newCanvas;

    d->m_birdEyeView->setScene(newCanvas);
    // std::cerr << "After m_birdEyeView set canvas" << std::endl;

    setScene(newCanvas);
    connect(newCanvas, &QGraphicsScene::selectionChanged, this, &DotGraphView::slotSelectionChanged);
    d->m_canvas = newCanvas;

    d->m_cvZoom = 0;

    d->m_graph->updateWithGraph(graph);
    d->m_layoutAlgoSelectAction->setCurrentAction(d->m_graph->layoutCommand(), Qt::CaseInsensitive);

    gvFreeLayout(gvc, graph);
    gvFreeContext(gvc);
    return true;
}

bool DotGraphView::loadDot(const QString &dotFileName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "'" << dotFileName << "'";
    Q_D(DotGraphView);
    d->m_birdEyeView->setScene(nullptr);

    if (d->m_canvas) {
        d->m_canvas->deleteLater();
        d->m_canvas = nullptr;
    }

    QString layoutCommand = (d->m_graph ? d->m_graph->layoutCommand() : QString());
    delete d->m_graph;

    d->m_graph = new DotGraph(layoutCommand, dotFileName);
    connect(d->m_graph, &DotGraph::readyToDisplay, this, &DotGraphView::displayGraph);

    if (d->m_readWrite) {
        d->m_graph->setReadWrite();
    }
    if (layoutCommand.isEmpty()) {
        layoutCommand = d->m_graph->chooseLayoutProgramForFile(d->m_graph->dotFileName());
    }
    d->m_graph->layoutCommand(layoutCommand);

    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Parsing " << m_graph->dotFileName() << " with " << m_graph->layoutCommand();
    d->m_xMargin = 50;
    d->m_yMargin = 50;

    QGraphicsScene *newCanvas = new QGraphicsScene();
    qCDebug(KGRAPHVIEWERLIB_LOG) << "Created canvas " << newCanvas;

    d->m_birdEyeView->setScene(newCanvas);
    //   std::cerr << "After m_birdEyeView set canvas" << std::endl;

    setScene(newCanvas);
    connect(newCanvas, &QGraphicsScene::selectionChanged, this, &DotGraphView::slotSelectionChanged);
    d->m_canvas = newCanvas;

    QGraphicsSimpleTextItem *loadingLabel = newCanvas->addSimpleText(i18n("graph %1 is getting loaded...", dotFileName));
    loadingLabel->setZValue(100);
    centerOn(loadingLabel);

    d->m_cvZoom = 0;

    if (!d->m_graph->parseDot(d->m_graph->dotFileName())) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "NOT successfully parsed!";
        loadingLabel->setText(i18n("error parsing file %1", dotFileName));
        return false;
    }
    d->m_layoutAlgoSelectAction->setCurrentAction(d->m_graph->layoutCommand(), Qt::CaseInsensitive);
    return true;
}

bool DotGraphView::loadLibrarySync(const QString &dotFileName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "loading sync: '" << dotFileName << "'";
    Q_D(DotGraphView);
    if (d->m_canvas)
        d->m_canvas->clear();
    QGraphicsSimpleTextItem *loadingLabel = d->m_canvas->addSimpleText(i18n("graph %1 is getting loaded...", dotFileName));
    loadingLabel->setZValue(100);
    centerOn(loadingLabel);

    // HACK: store filename in m_loadThread data structure for pick up by slotAGraphLayoutFinished()
    // Both methods loadLibrarySync(QString) and loadLibrary(QString) after loading the file
    // activate the m_layoutThread. And it is only the handler of that thread's finished signal.
    // slotAGraphLayoutFinished(), which then completes the loading of the file and stores the
    // filename with the graph data, taking it from m_loadThread.
    // As intermediate solution to a rewrite of the loading code, for now the filename is simply
    // also stored in the m_loadThread object from this code path, so the rest of the existing code
    // does not need to be further adapted.
    d->m_loadThread.setDotFileName(dotFileName);

    qCDebug(KGRAPHVIEWERLIB_LOG) << dotFileName;
    FILE *fp = fopen(dotFileName.toUtf8().data(), "r");
    if (!fp) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "Failed to open file " << dotFileName;
        return false;
    }
    graph_t *graph = agread(fp, nullptr);
    if (!graph) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "Failed to read file, retrying to work around graphviz bug(?)";
        rewind(fp);
        graph = agread(fp, nullptr);
    }
    fclose(fp);
    if (!graph) {
        qCWarning(KGRAPHVIEWERLIB_LOG) << "Failed to read file " << dotFileName;
        return false;
    }

    QString layoutCommand = (d->m_graph ? d->m_graph->layoutCommand() : QString());
    if (layoutCommand.isEmpty()) {
        layoutCommand = d->m_graph ? d->m_graph->chooseLayoutProgramForFile(dotFileName) : QStringLiteral("dot");
    }
    d->m_layoutThread.layoutGraph(graph, layoutCommand);

    return true;
}

bool DotGraphView::loadLibrary(const QString &dotFileName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "'" << dotFileName << "'";
    Q_D(DotGraphView);
    if (d->m_canvas)
        d->m_canvas->clear();
    QGraphicsSimpleTextItem *loadingLabel = d->m_canvas->addSimpleText(i18n("graph %1 is getting loaded...", dotFileName));
    loadingLabel->setZValue(100);
    centerOn(loadingLabel);

    d->m_loadThread.loadFile(dotFileName);

    return true;
}

bool DotGraphView::loadLibrary(graph_t *graph, const QString &layoutCommand)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "graph_t";
    Q_D(DotGraphView);
    d->m_birdEyeView->setScene(nullptr);

    if (d->m_canvas) {
        d->m_canvas->deleteLater();
        d->m_canvas = nullptr;
    }

    delete d->m_graph;
    d->m_graph = nullptr;

    if (!graph)
        return false;

    qCDebug(KGRAPHVIEWERLIB_LOG) << "layoutCommand:" << layoutCommand;
    d->m_graph = new DotGraph(layoutCommand, QString());
    d->m_graph->setUseLibrary(true);

    connect(d->m_graph, &DotGraph::readyToDisplay, this, &DotGraphView::displayGraph);

    if (d->m_readWrite) {
        d->m_graph->setReadWrite();
    }

    d->m_xMargin = 50;
    d->m_yMargin = 50;

    QGraphicsScene *newCanvas = new QGraphicsScene();
    qCDebug(KGRAPHVIEWERLIB_LOG) << "Created canvas " << newCanvas;

    d->m_birdEyeView->setScene(newCanvas);
    setScene(newCanvas);
    connect(newCanvas, &QGraphicsScene::selectionChanged, this, &DotGraphView::slotSelectionChanged);
    d->m_canvas = newCanvas;

    d->m_cvZoom = 0;

    d->m_graph->updateWithGraph(graph);
    d->m_layoutAlgoSelectAction->setCurrentAction(d->m_graph->layoutCommand(), Qt::CaseInsensitive);

    return true;
}

void DotGraphView::slotSelectionChanged()
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << scene()->selectedItems().size();
}

bool DotGraphView::displayGraph()
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << d->m_graph->backColor();
    //   hide();
    viewport()->setUpdatesEnabled(false);

    if (d->m_graph->backColor().size() != 0) {
        setBackgroundColor(QColor(d->m_graph->backColor()));
    }

    if (d->m_graph->nodes().size() > KGV_MAX_PANNER_NODES) {
        d->m_birdEyeView->setDrawingEnabled(false);
    }
    //  QCanvasEllipse* eItem;
    double scale = d->detailAdjustedScale();

    qreal gh = d->m_graph->height();

    d->m_xMargin = 50;
    d->m_yMargin = 50;

    //   m_canvas->setSceneRect(0,0,w+2*m_xMargin, h+2*m_yMargin);
    //   m_canvas->setBackgroundBrush(QBrush(QColor(m_graph->backColor())));
    d->m_canvas->setBackgroundBrush(QBrush(d->m_backgroundColor));

    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "sceneRect is now " << m_canvas->sceneRect();

    qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating" << d->m_graph->subgraphs().size() << "CanvasSubgraphs from" << d->m_graph;
    int zvalue = -1;
    for (GraphSubgraph *gsubgraph : d->m_graph->subgraphs()) {
        int newZvalue = d->displaySubgraph(gsubgraph, zvalue);
        if (newZvalue > zvalue)
            zvalue = newZvalue;
    }

    qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating" << d->m_graph->nodes().size() << "nodes from" << d->m_graph;
    GraphNodeMap::const_iterator it = d->m_graph->nodes().constBegin();
    for (; it != d->m_graph->nodes().constEnd(); it++) {
        const QString &id = it.key();
        GraphNode *gnode = it.value();
        qCDebug(KGRAPHVIEWERLIB_LOG) << "Handling" << id << (void *)gnode;
        qCDebug(KGRAPHVIEWERLIB_LOG) << "  gnode id=" << gnode->id();
        qCDebug(KGRAPHVIEWERLIB_LOG) << "  canvasNode=" << (void *)gnode->canvasNode();
        if (gnode->canvasNode() == nullptr) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating canvas node for" << gnode->id();
            CanvasNode *cnode = new CanvasNode(this, gnode, d->m_canvas);
            if (cnode == nullptr)
                continue;
            cnode->initialize(scale, scale, d->m_xMargin, d->m_yMargin, gh);
            gnode->setCanvasNode(cnode);
            d->m_canvas->addItem(cnode);
            //       cnode->setZValue(gnode->z());
            cnode->setZValue(zvalue + 1);
            cnode->show();
        }
        gnode->canvasNode()->computeBoundingRect();
    }

    qCDebug(KGRAPHVIEWERLIB_LOG) << "Creating" << d->m_graph->edges().size() << "edges from" << d->m_graph;
    for (GraphEdge *gedge : d->m_graph->edges()) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "One GraphEdge:" << gedge->id();
        if (gedge->canvasEdge() == nullptr && gedge->fromNode() && gedge->toNode()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "New CanvasEdge for" << gedge->id();
            qCDebug(KGRAPHVIEWERLIB_LOG) << "edge fromNode=" << (void *)gedge->fromNode();
            qCDebug(KGRAPHVIEWERLIB_LOG) << "              " << gedge->fromNode()->id();
            qCDebug(KGRAPHVIEWERLIB_LOG) << "edge toNode=" << (void *)gedge->toNode();
            qCDebug(KGRAPHVIEWERLIB_LOG) << "              " << gedge->toNode()->id();
            CanvasEdge *cedge = new CanvasEdge(this, gedge, scale, scale, d->m_xMargin, d->m_yMargin, gh, d->m_graph->wdhcf(), d->m_graph->hdvcf());

            gedge->setCanvasEdge(cedge);
            //     std::cerr << "setting z = " << gedge->z() << std::endl;
            //    cedge->setZValue(gedge->z());
            cedge->setZValue(zvalue + 2);
            cedge->show();
            d->m_canvas->addItem(cedge);
        }
        if (gedge->canvasEdge())
            gedge->canvasEdge()->computeBoundingRect();
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << "Adding graph render operations: " << d->m_graph->renderOperations().size();
    for (const DotRenderOp &dro : d->m_graph->renderOperations()) {
        if (dro.renderop == QLatin1String("T")) {
            //       std::cerr << "Adding graph label '"<<dro.str<<"'" << std::endl;
            const QString &str = dro.str;
            int stringWidthGoal = int(dro.integers[3] * scale);
            int fontSize = d->m_graph->fontSize();
            QFont *font = FontsCache::changeable().fromName(d->m_graph->fontName());
            font->setPointSize(fontSize);
            QFontMetrics fm(*font);
            while (fm.horizontalAdvance(str) > stringWidthGoal && fontSize > 1) {
                fontSize--;
                font->setPointSize(fontSize);
                fm = QFontMetrics(*font);
            }
            QGraphicsSimpleTextItem *labelView = new QGraphicsSimpleTextItem(str, d->m_canvas->activePanel());
            labelView->setFont(*font);
            labelView->setPos((scale * ((dro.integers[0]) + (((dro.integers[2]) * (dro.integers[3])) / 2) - ((dro.integers[3]) / 2)) + d->m_xMargin), ((gh - (dro.integers[1])) * scale) + d->m_yMargin);
            /// @todo port that ; how to set text color ?
            labelView->setPen(QPen(Dot2QtConsts::componentData().qtColor(d->m_graph->fontColor())));
            labelView->setFont(*font);
            d->m_labelViews.insert(labelView);
        }
    }

    qCDebug(KGRAPHVIEWERLIB_LOG) << "Finalizing";
    d->m_cvZoom = 0;
    d->updateSizes();

    centerOn(d->m_canvas->sceneRect().center());

    viewport()->setUpdatesEnabled(true);
    QSet<QGraphicsSimpleTextItem *>::iterator labelViewsIt, labelViewsIt_end;
    labelViewsIt = d->m_labelViews.begin();
    labelViewsIt_end = d->m_labelViews.end();
    for (; labelViewsIt != labelViewsIt_end; labelViewsIt++) {
        (*labelViewsIt)->show();
    }
    d->m_canvas->update();

    Q_EMIT graphLoaded();

    return true;
}

void DotGraphView::focusInEvent(QFocusEvent *)
{
    Q_D(DotGraphView);
    if (!d->m_canvas)
        return;

    //   m_canvas->update();
}

void DotGraphView::focusOutEvent(QFocusEvent *e)
{
    // trigger updates as in focusInEvent
    focusInEvent(e);
}

void DotGraphView::scrollViewPercent(bool horizontal, int percent)
{
    QScrollBar *scrollbar = horizontal ? horizontalScrollBar() : verticalScrollBar();
    int amount = horizontal ? viewport()->width() : viewport()->height();
    amount = amount * percent / 100;
    scrollbar->setValue(scrollbar->value() + amount);
}

void DotGraphView::keyPressEvent(QKeyEvent *e)
{
    Q_D(DotGraphView);
    if (!d->m_canvas) {
        e->ignore();
        return;
    }

    // move canvas...
    if (e->key() == Qt::Key_Home)
        verticalScrollBar()->setValue(verticalScrollBar()->minimum());
    else if (e->key() == Qt::Key_End)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    else if (e->key() == Qt::Key_PageUp)
        scrollViewPercent(false, -50);
    else if (e->key() == Qt::Key_PageDown)
        scrollViewPercent(false, 50);
    else if (e->key() == Qt::Key_Left)
        scrollViewPercent(true, -10);
    else if (e->key() == Qt::Key_Right)
        scrollViewPercent(true, 10);
    else if (e->key() == Qt::Key_Down)
        scrollViewPercent(false, 10);
    else if (e->key() == Qt::Key_Up)
        scrollViewPercent(false, -10);
    else {
        e->ignore();
        return;
    }
}

void DotGraphView::wheelEvent(QWheelEvent *e)
{
    Q_D(DotGraphView);
    if (!d->m_canvas) {
        e->ignore();
        return;
    }
    e->accept();
    if (QApplication::keyboardModifiers() == Qt::ShiftModifier || QApplication::keyboardModifiers() == Qt::ControlModifier) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << " + Shift/Ctrl: zooming";
        // move canvas...
        const int delta = e->angleDelta().y();
        if (delta < 0) {
            zoomOut();
        } else if (delta > 0) {
            zoomIn();
        }
    } else {
        qCDebug(KGRAPHVIEWERLIB_LOG) << " : scrolling ";
        const bool horizontal = (std::abs(e->angleDelta().x()) > std::abs(e->angleDelta().y()));
        const int delta = horizontal ? e->angleDelta().x() : e->angleDelta().y();
        scrollViewPercent(horizontal, delta < 0 ? 10 : -10);
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
    if (d->m_zoom > 1.0 && d->m_zoom < 1.1) {
        d->m_zoom = 1;
    }

    setUpdatesEnabled(false);
    QTransform m;
    m.scale(d->m_zoom, d->m_zoom);
    setTransform(m);
    Q_EMIT zoomed(d->m_zoom);
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

void DotGraphView::resizeEvent(QResizeEvent *e)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "resizeEvent";
    QGraphicsView::resizeEvent(e);
    if (d->m_canvas)
        d->updateSizes(e->size());
    //   std::cerr << "resizeEvent end" << std::endl;
}

void DotGraphView::zoomRectMovedTo(QPointF newZoomPos)
{
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::zoomRectMovedTo " << newZoomPos;
    centerOn(newZoomPos);
}

void DotGraphView::zoomRectMoveFinished()
{
    Q_D(DotGraphView);
    //    qCDebug(KGRAPHVIEWERLIB_LOG) << "zoomRectMoveFinished";
    d->updateBirdEyeView();
    //   std::cerr << "zoomRectMoveFinished end" << std::endl;
}

void DotGraphView::mousePressEvent(QMouseEvent *e)
{
    Q_D(DotGraphView);
    if (e->button() != Qt::LeftButton) {
        return;
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << e << d->m_editingMode;
    QGraphicsView::mousePressEvent(e);

    if (d->m_editingMode == AddNewElement) {
        double scale = d->detailAdjustedScale();

        qreal gh = d->m_graph->height();

        QPointF pos = mapToScene(e->pos().x() - d->m_defaultNewElementPixmap.width() / 2, e->pos().y() - d->m_defaultNewElementPixmap.height() / 2);
        GraphNode *newNode = new GraphNode();
        newNode->attributes() = d->m_newElementAttributes;
        if (newNode->attributes().find(QStringLiteral("id")) == newNode->attributes().end()) {
            newNode->setId(QStringLiteral("NewNode%1").arg(d->m_graph->nodes().size()));
        }
        if (newNode->attributes().find(QStringLiteral("label")) == newNode->attributes().end()) {
            newNode->setLabel(newNode->id());
        }
        d->m_graph->nodes().insert(newNode->id(), newNode);
        CanvasNode *newCNode = new CanvasNode(this, newNode, d->m_canvas);
        newCNode->initialize(scale, scale, d->m_xMargin, d->m_yMargin, gh);
        newNode->setCanvasNode(newCNode);
        scene()->addItem(newCNode);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "setting pos to " << pos;
        newCNode->setPos(pos);
        newCNode->setZValue(100);
        newCNode->show();

        d->m_editingMode = None;
        unsetCursor();
        Q_EMIT newNodeAdded(newNode->id());
    } else if (d->m_editingMode == SelectingElements) {
    } else {
        if (d->m_editingMode != None && itemAt(e->pos()) == nullptr) // click outside any item: unselect all
        {
            if (d->m_editingMode == DrawNewEdge) // was drawing an edge; cancel it
            {
                if (d->m_newEdgeDraft) {
                    d->m_newEdgeDraft->hide();
                    scene()->removeItem(d->m_newEdgeDraft);
                    delete d->m_newEdgeDraft;
                    d->m_newEdgeDraft = nullptr;
                }
                d->m_newEdgeSource = nullptr;
                d->m_editingMode = None;
            } else if (d->m_editingMode == AddNewEdge) {
                d->m_editingMode = None;
            }
            for (GraphEdge *e : d->m_graph->edges()) {
                if (e->isSelected()) {
                    e->setSelected(false);
                    e->canvasEdge()->update();
                }
            }
            for (GraphNode *n : d->m_graph->nodes()) {
                if (n->isSelected()) {
                    n->setSelected(false);
                    n->canvasElement()->update();
                }
            }
            for (GraphSubgraph *s : d->m_graph->subgraphs()) {
                if (s->isSelected()) {
                    s->setSelected(false);
                    s->canvasElement()->update();
                }
            }
            Q_EMIT selectionIs(QList<QString>(), QPoint());
        }
        d->m_pressPos = e->globalPosition().toPoint();
        d->m_pressScrollBarsPos = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
    }
    d->m_isMoving = true;
}

void DotGraphView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(DotGraphView);
    QGraphicsView::mouseMoveEvent(e);
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << scene()->selectedItems().size();

    if (d->m_editingMode == DrawNewEdge) {
        if (d->m_newEdgeDraft) {
            QPointF src = d->m_newEdgeDraft->line().p1();
            QPointF tgt = mapToScene(e->pos());

            //     qCDebug(KGRAPHVIEWERLIB_LOG) << "Setting new edge draft line to" << QLineF(src,tgt);
            d->m_newEdgeDraft->setLine(QLineF(src, tgt));
        }
    } else if (d->m_editingMode == SelectingElements) {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << "selecting";
    } else if (e->buttons().testFlag(Qt::LeftButton)) {
        //     qCDebug(KGRAPHVIEWERLIB_LOG) << (e->globalPos() - d->m_pressPos);
        QPoint diff = e->globalPosition().toPoint() - d->m_pressPos;
        horizontalScrollBar()->setValue(d->m_pressScrollBarsPos.x() - diff.x());
        verticalScrollBar()->setValue(d->m_pressScrollBarsPos.y() - diff.y());
    }
}

void DotGraphView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << e << d->m_editingMode;
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "setDragMode(NoDrag)";
    //   setDragMode(NoDrag);
    if (d->m_editingMode == AddNewElement) {
        d->m_editingMode = None;
        unsetCursor();
    } else if (d->m_editingMode == SelectingElements) {
        QGraphicsView::mouseReleaseEvent(e);
        qCDebug(KGRAPHVIEWERLIB_LOG) << "Stopping selection" << scene() << d->m_canvas;
        QList<QGraphicsItem *> items = scene()->selectedItems();
        QList<QString> selection;
        for (QGraphicsItem *item : items) {
            CanvasElement *element = dynamic_cast<CanvasElement *>(item);
            element->element()->setSelected(true);
            if (element) {
                selection.push_back(element->element()->id());
            }
        }
        d->m_editingMode = None;
        unsetCursor();
        setDragMode(NoDrag);
        if (!selection.isEmpty()) {
            update();
            Q_EMIT selectionIs(selection, mapToGlobal(e->pos()));
        }
    } else {
        QGraphicsView::mouseReleaseEvent(e);
    }
    d->m_isMoving = false;
}

void DotGraphView::mouseDoubleClickEvent(QMouseEvent *e)
{
    QGraphicsView::mouseDoubleClickEvent(e);
}

void DotGraphView::contextMenuEvent(QContextMenuEvent *e)
{
    Q_D(DotGraphView);
    //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

    d->m_popup->exec(e->globalPos());
}

void DotGraphView::slotContextMenuEvent(const QString &id, const QPoint &p)
{
    //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

    Q_EMIT contextMenuEvent(id, p);
}

void DotGraphView::slotElementHoverEnter(CanvasElement *element)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << element->element()->id();
    //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

    Q_EMIT hoverEnter(element->element()->id());
}

void DotGraphView::slotElementHoverLeave(CanvasElement *element)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << element->element()->id();
    //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

    Q_EMIT hoverLeave(element->element()->id());
}

void DotGraphView::slotElementHoverEnter(CanvasEdge *element)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << element->edge()->id();
    //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

    Q_EMIT hoverEnter(element->edge()->id());
}

void DotGraphView::slotElementHoverLeave(CanvasEdge *element)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << element->edge()->id();
    //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

    Q_EMIT hoverLeave(element->edge()->id());
}

void DotGraphView::setLayoutCommand(const QString &command)
{
    Q_D(DotGraphView);
    d->m_graph->layoutCommand(command);
    reload();
}

KGraphViewerInterface::PannerPosition DotGraphView::zoomPos(const QString &s)
{
    KGraphViewerInterface::PannerPosition res = DEFAULT_ZOOMPOS;
    if (s == QStringLiteral("KGraphViewerInterface::TopLeft"))
        res = KGraphViewerInterface::TopLeft;
    if (s == QStringLiteral("KGraphViewerInterface::TopRight"))
        res = KGraphViewerInterface::TopRight;
    if (s == QStringLiteral("KGraphViewerInterface::BottomLeft"))
        res = KGraphViewerInterface::BottomLeft;
    if (s == QStringLiteral("KGraphViewerInterface::BottomRight"))
        res = KGraphViewerInterface::BottomRight;
    if (s == QStringLiteral("Automatic"))
        res = KGraphViewerInterface::Auto;

    return res;
}

void DotGraphView::setPannerEnabled(bool enabled)
{
    Q_UNUSED(enabled);
    Q_D(DotGraphView);
    d->m_bevPopup->setEnabled(d->m_bevEnabledAction->isChecked());
    KGraphViewerPartSettings::setBirdsEyeViewEnabled(d->m_bevEnabledAction->isChecked());
    KGraphViewerPartSettings::self()->save();
    d->updateSizes();
}

void DotGraphView::viewBevActivated(int newZoomPos)
{
    Q_D(DotGraphView);
    d->m_zoomPosition = (KGraphViewerInterface::PannerPosition)newZoomPos;
    d->updateSizes();
    Q_EMIT sigViewBevActivated(newZoomPos);
}

QString DotGraphView::zoomPosString(KGraphViewerInterface::PannerPosition p)
{
    if (p == KGraphViewerInterface::TopRight)
        return QStringLiteral("KGraphViewerInterface::TopRight");
    if (p == KGraphViewerInterface::BottomLeft)
        return QStringLiteral("KGraphViewerInterface::BottomLeft");
    if (p == KGraphViewerInterface::BottomRight)
        return QStringLiteral("KGraphViewerInterface::BottomRight");
    if (p == KGraphViewerInterface::Auto)
        return QStringLiteral("Automatic");

    return QStringLiteral("KGraphViewerInterface::TopLeft");
}

void DotGraphView::readViewConfig()
{
    Q_D(DotGraphView);
    KConfigGroup g(KSharedConfig::openConfig(), QStringLiteral("GraphViewLayout"));

    QVariant dl = DEFAULT_DETAILLEVEL;
    d->m_detailLevel = g.readEntry("DetailLevel", dl).toInt();
    d->m_zoomPosition = zoomPos(g.readEntry("KGraphViewerInterface::PannerPosition", zoomPosString(DEFAULT_ZOOMPOS)));
    Q_EMIT sigViewBevActivated(d->m_zoomPosition);
}

void DotGraphView::saveViewConfig()
{
    Q_D(DotGraphView);
    //   qCDebug(KGRAPHVIEWERLIB_LOG) << "Saving view config";
    KConfigGroup g(KSharedConfig::openConfig(), QStringLiteral("GraphViewLayout"));

    writeConfigEntry(&g, "DetailLevel", d->m_detailLevel, DEFAULT_DETAILLEVEL);
    writeConfigEntry(&g, "KGraphViewerInterface::PannerPosition", zoomPosString(d->m_zoomPosition), zoomPosString(DEFAULT_ZOOMPOS).toUtf8().data());
    g.sync();
}

void DotGraphView::pageSetup()
{
    Q_D(DotGraphView);
    if (d->m_printCommand == nullptr) {
        d->m_printCommand = new KGVSimplePrintingCommand(this, 0);
    }
    d->m_printCommand->showPageSetup(d->m_graph->dotFileName());
    return;
}

void DotGraphView::print()
{
    Q_D(DotGraphView);
    if (d->m_printCommand == nullptr) {
        d->m_printCommand = new KGVSimplePrintingCommand(this, 0);
    }
    d->m_printCommand->print(d->m_graph->dotFileName());
    return;
}

void DotGraphView::printPreview()
{
    Q_D(DotGraphView);
    if (d->m_printCommand == nullptr) {
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

void DotGraphView::dirty(const QString &dotFileName)
{
    Q_D(DotGraphView);
    //   std::cerr << "SLOT dirty for " << dotFileName << std::endl;
    if (dotFileName == d->m_graph->dotFileName()) {
        if (QMessageBox::question(this, i18n("Reload Confirmation"), i18n("The file %1 has been modified on disk.\nDo you want to reload it?", dotFileName)) == QMessageBox::Yes) {
            if (d->m_graph->useLibrary())
                loadLibrary(dotFileName);
            else
                loadDot(dotFileName);
        }
    }
}

KConfigGroup *DotGraphView::configGroup(KConfig *c, const QString &group, const QString &post)
{
    QStringList gList = c->groupList();
    QString res = group;
    if (gList.contains(group + post))
        res += post;
    return new KConfigGroup(c, res);
}

void DotGraphView::writeConfigEntry(KConfigGroup *c, const char *pKey, const QString &value, const char *def)
{
    if (!c)
        return;
    if ((value.isEmpty() && ((def == nullptr) || (*def == 0))) || (value == QString::fromUtf8(def)))
        c->deleteEntry(pKey);
    else
        c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigGroup *c, const char *pKey, int value, int def)
{
    if (!c)
        return;
    if (value == def)
        c->deleteEntry(pKey);
    else
        c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigGroup *c, const char *pKey, double value, double def)
{
    if (!c)
        return;
    if (value == def)
        c->deleteEntry(pKey);
    else
        c->writeEntry(pKey, value);
}

void DotGraphView::writeConfigEntry(KConfigGroup *c, const char *pKey, bool value, bool def)
{
    if (!c)
        return;
    if (value == def)
        c->deleteEntry(pKey);
    else
        c->writeEntry(pKey, value);
}

const QString &DotGraphView::dotFileName()
{
    Q_D(DotGraphView);
    return d->m_graph->dotFileName();
}

void DotGraphView::hideToolsWindows()
{
    Q_D(DotGraphView);
    if (d->m_printCommand) {
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
        QString layoutCommand = QInputDialog::getText(this,
                                                      i18n("Layout Command"),
                                                      i18n("Specify here the command that will be used to layout the graph.\n"
                                                           "The command MUST write its results on stdout in xdot format."),
                                                      QLineEdit::Normal,
                                                      currentLayoutCommand,
                                                      &ok);
        //       std::cerr << "Got layout command: " << layoutCommand << std::endl;
        if (ok && layoutCommand != currentLayoutCommand) {
            //         std::cerr << "Setting new layout command: " << layoutCommand << std::endl;
            if (!d->m_layoutAlgoSelectAction->setCurrentAction(layoutCommand, Qt::CaseInsensitive)) {
                QAction *new_action = d->m_layoutAlgoSelectAction->addAction(layoutCommand);
                d->m_layoutAlgoSelectAction->setCurrentAction(new_action);
                slotSelectLayoutAlgo(layoutCommand);
            }
        }
    }
}

void DotGraphView::slotLayoutReset()
{
    Q_D(DotGraphView);
    d->m_layoutAlgoSelectAction->setCurrentAction(QStringLiteral("Dot"));
    slotSelectLayoutAlgo(QStringLiteral("Dot"));
}

void DotGraphView::slotSelectLayoutAlgo(const QString &ttext)
{
    QString text = ttext; //.mid(1);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::slotSelectLayoutAlgo '" << text << "'";
    if (text == QLatin1String("Dot")) {
        setLayoutCommand(QStringLiteral("dot"));
    } else if (text == QLatin1String("Neato")) {
        setLayoutCommand(QStringLiteral("neato"));
    } else if (text == QLatin1String("Twopi")) {
        setLayoutCommand(QStringLiteral("twopi"));
    } else if (text == QLatin1String("Fdp")) {
        setLayoutCommand(QStringLiteral("fdp"));
    } else if (text == QLatin1String("Circo")) {
        setLayoutCommand(QStringLiteral("circo"));
    } else {
        setLayoutCommand(text);
    }
}

void DotGraphView::slotSelectLayoutDot()
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::slotSelectLayoutDot";
    setLayoutCommand(QStringLiteral("dot -Txdot"));
}

void DotGraphView::slotSelectLayoutNeato()
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::slotSelectLayoutNeato";
    setLayoutCommand(QStringLiteral("neato -Txdot"));
}

void DotGraphView::slotSelectLayoutTwopi()
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::slotSelectLayoutTwopi";
    setLayoutCommand(QStringLiteral("twopi -Txdot"));
}

void DotGraphView::slotSelectLayoutFdp()
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::slotSelectLayoutFdp";
    setLayoutCommand(QStringLiteral("fdp -Txdot"));
}

void DotGraphView::slotSelectLayoutCirco()
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::slotSelectLayoutCirco";
    setLayoutCommand(QStringLiteral("circo -Txdot"));
}

void DotGraphView::slotBevToggled()
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << "DotGraphView::slotBevToggled";
    qCDebug(KGRAPHVIEWERLIB_LOG) << "    d->m_bevEnabledAction is checked ? " << d->m_bevEnabledAction->isChecked();
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
    d->m_graph->update();
    d->m_layoutAlgoSelectAction->setCurrentAction(d->m_graph->layoutCommand(), Qt::CaseInsensitive);
}

void DotGraphView::prepareAddNewElement(QMap<QString, QString> attribs)
{
    Q_D(DotGraphView);
    d->m_editingMode = AddNewElement;
    d->m_newElementAttributes = attribs;
    unsetCursor();
    setCursor(QCursor(d->m_defaultNewElementPixmap));
}

void DotGraphView::prepareAddNewEdge(QMap<QString, QString> attribs)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribs;
    bool anySelected = false;
    for (GraphEdge *edge : d->m_graph->edges()) {
        if (edge->isSelected()) {
            anySelected = true;
            QMap<QString, QString>::const_iterator it = attribs.constBegin();
            for (; it != attribs.constEnd(); it++) {
                edge->attributes()[it.key()] = it.value();
            }
        }
    }
    if (anySelected) {
        return;
    }
    d->m_editingMode = AddNewEdge;
    d->m_newElementAttributes = attribs;
    unsetCursor();
    QBitmap bm(QStringLiteral(":/kgraphviewerpart/pics/newedge.png"));
    setCursor(QCursor(bm, bm, 32, 16));
}

void DotGraphView::prepareSelectElements()
{
    Q_D(DotGraphView);
    d->m_editingMode = SelectingElements;
    setCursor(Qt::CrossCursor);
    setDragMode(RubberBandDrag);
}

void DotGraphView::createNewEdgeDraftFrom(CanvasElement *node)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << node->element()->id();
    d->m_editingMode = DrawNewEdge;
    unsetCursor();
    d->m_newEdgeSource = node;

    if (d->m_newEdgeDraft) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "removing new edge draft";
        d->m_newEdgeDraft->hide();
        scene()->removeItem(d->m_newEdgeDraft);
        delete d->m_newEdgeDraft;
        d->m_newEdgeDraft = nullptr;
    }
    d->m_newEdgeDraft = new QGraphicsLineItem(QLineF(node->boundingRect().center() + node->pos(), node->boundingRect().center() + node->pos() + QPointF(10, 10)));
    scene()->addItem(d->m_newEdgeDraft);
    d->m_newEdgeDraft->setZValue(1000);
    d->m_newEdgeDraft->show();
    qCDebug(KGRAPHVIEWERLIB_LOG) << d->m_newEdgeDraft->line();
}

void DotGraphView::finishNewEdgeTo(CanvasElement *node)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << node->element()->id();
    d->m_editingMode = None;
    unsetCursor();

    if (d->m_newEdgeDraft) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "removing new edge draft";
        d->m_newEdgeDraft->hide();
        scene()->removeItem(d->m_newEdgeDraft);
        delete d->m_newEdgeDraft;
        d->m_newEdgeDraft = nullptr;
    }

    Q_EMIT newEdgeFinished(d->m_newEdgeSource->element()->id(), node->element()->id(), d->m_newElementAttributes);

    d->m_newEdgeSource = nullptr;
}

// void DotGraphView::slotFinishNewEdge(
//       const QString& srcId,
//       const QString& tgtId,
//       const QMap<QString, QString> newElementAttributes)
// {
//   qCDebug(KGRAPHVIEWERLIB_LOG) ;
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
//   double scale = detailAdjustedScale();
//
//   qreal gh = d->m_graph->height();
//   CanvasEdge* cedge = new CanvasEdge(this, gedge, scale, scale, d->m_xMargin,
//         d->m_yMargin, gh, d->m_graph->wdhcf(), d->m_graph->hdvcf());
//
//   gedge->setCanvasEdge(cedge);
// //     std::cerr << "setting z = " << gedge->z() << std::endl;
//   cedge->setZValue(gedge->z());
//   cedge->show();
//   scene()->addItem(cedge);
//
//   Q_EMIT newEdgeAdded(gedge->fromNode()->id(),gedge->toNode()->id());
// }

void DotGraphView::setReadOnly()
{
    Q_D(DotGraphView);
    d->m_readWrite = false;
    if (d->m_graph) {
        d->m_graph->setReadOnly();
    }
}

void DotGraphView::setReadWrite()
{
    Q_D(DotGraphView);
    d->m_readWrite = true;
    if (d->m_graph) {
        d->m_graph->setReadWrite();
    }
}

void DotGraphView::slotEdgeSelected(CanvasEdge *edge, Qt::KeyboardModifiers modifiers)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << edge->edge()->id();
    QList<QString> selection;
    selection.push_back(edge->edge()->id());
    if (!modifiers.testFlag(Qt::ControlModifier)) {
        for (GraphEdge *e : d->m_graph->edges()) {
            if (e->canvasEdge() != edge) {
                e->setSelected(false);
                e->canvasEdge()->update();
            }
        }
        for (GraphNode *n : d->m_graph->nodes()) {
            n->setSelected(false);
            n->canvasNode()->update();
        }
        for (GraphSubgraph *s : d->m_graph->subgraphs()) {
            s->setElementSelected(nullptr, false, true);
        }
    } else {
        for (GraphEdge *e : d->m_graph->edges()) {
            if (e->canvasEdge() != edge) {
                if (e->isSelected()) {
                    selection.push_back(e->id());
                }
            }
        }
        for (GraphNode *n : d->m_graph->nodes()) {
            if (n->isSelected()) {
                selection.push_back(n->id());
            }
        }
        for (GraphSubgraph *s : d->m_graph->subgraphs()) {
            if (s->isSelected()) {
                selection.push_back(s->id());
            }
        }
    }
    Q_EMIT selectionIs(selection, QPoint());
}

void DotGraphView::slotElementSelected(CanvasElement *element, Qt::KeyboardModifiers modifiers)
{
    Q_D(DotGraphView);
    QList<QString> selection;
    selection.push_back(element->element()->id());
    if (!modifiers.testFlag(Qt::ControlModifier)) {
        for (GraphEdge *e : d->m_graph->edges()) {
            if (e->isSelected()) {
                e->setSelected(false);
                e->canvasEdge()->update();
            }
        }
        for (GraphNode *e : d->m_graph->nodes()) {
            if (e->canvasElement() != element) {
                if (e->isSelected()) {
                    e->setSelected(false);
                    e->canvasElement()->update();
                }
            }
        }
        for (GraphSubgraph *s : d->m_graph->subgraphs()) {
            s->setElementSelected(element->element(), true, true);
        }
    } else {
        for (GraphEdge *e : d->m_graph->edges()) {
            if (e->isSelected()) {
                selection.push_back(e->id());
            }
        }
        for (GraphNode *n : d->m_graph->nodes()) {
            if (n->isSelected()) {
                selection.push_back(n->id());
            }
        }
        for (GraphSubgraph *s : d->m_graph->subgraphs()) {
            s->retrieveSelectedElementsIds(selection);
        }
    }
    Q_EMIT selectionIs(selection, QPoint());
}

void DotGraphView::removeSelectedEdges()
{
    Q_D(DotGraphView);
    for (GraphEdge *e : d->m_graph->edges()) {
        if (e->isSelected()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "emiting removeEdge " << e->id();
            d->m_graph->removeEdge(e->id());
            Q_EMIT removeEdge(e->id());
        }
    }
}

void DotGraphView::removeSelectedNodes()
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG);
    for (GraphNode *e : d->m_graph->nodes()) {
        if (e->isSelected()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "emiting removeElement " << e->id();
            d->m_graph->removeElement(e->id());
            Q_EMIT removeElement(e->id());
        }
    }
}

void DotGraphView::removeSelectedSubgraphs()
{
    Q_D(DotGraphView);
    for (GraphSubgraph *e : d->m_graph->subgraphs()) {
        if (e->isSelected()) {
            qCDebug(KGRAPHVIEWERLIB_LOG) << "emiting removeElement " << e->id();
            d->m_graph->removeElement(e->id());
            Q_EMIT removeElement(e->id());
        }
    }
}

void DotGraphView::removeSelectedElements()
{
    removeSelectedNodes();
    removeSelectedEdges();
    removeSelectedSubgraphs();
}

void DotGraphView::timerEvent(QTimerEvent *event)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << event->timerId();
    qreal vpercent = verticalScrollBar()->value() * 1.0 / 100;
    qreal hpercent = horizontalScrollBar()->value() * 1.0 / 100;
    if (d->m_scrollDirection == Left) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (5 * hpercent));
    } else if (d->m_scrollDirection == Right) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (5 * hpercent));
    } else if (d->m_scrollDirection == Top) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (5 * vpercent));
    } else if (d->m_scrollDirection == Bottom) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + (5 * vpercent));
    }
}

void DotGraphView::leaveEvent(QEvent * /*event*/)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG) << mapFromGlobal(QCursor::pos());
    if (d->m_editingMode == DrawNewEdge) {
        d->m_leavedTimer = startTimer(10);
        if (mapFromGlobal(QCursor::pos()).x() <= 0) {
            d->m_scrollDirection = Left;
        } else if (mapFromGlobal(QCursor::pos()).y() <= 0) {
            d->m_scrollDirection = Top;
        } else if (mapFromGlobal(QCursor::pos()).x() >= width()) {
            d->m_scrollDirection = Right;
        } else if (mapFromGlobal(QCursor::pos()).y() >= height()) {
            d->m_scrollDirection = Bottom;
        }
    }
}

void DotGraphView::enterEvent(QEnterEvent * /*event*/)
{
    Q_D(DotGraphView);
    qCDebug(KGRAPHVIEWERLIB_LOG);
    if (d->m_leavedTimer != std::numeric_limits<int>::max()) {
        killTimer(d->m_leavedTimer);
        d->m_leavedTimer = std::numeric_limits<int>::max();
    }
}

void DotGraphView::slotAGraphReadFinished()
{
    Q_D(DotGraphView);
    QString layoutCommand = (d->m_graph ? d->m_graph->layoutCommand() : QString());
    if (layoutCommand.isEmpty()) {
        if (!d->m_loadThread.dotFileName().isEmpty())
            layoutCommand = d->m_graph->chooseLayoutProgramForFile(d->m_loadThread.dotFileName());
        else
            layoutCommand = QStringLiteral("dot");
    }
    d->m_layoutThread.layoutGraph(d->m_loadThread.g(), layoutCommand);
    d->m_loadThread.processed_finished();
}

void DotGraphView::slotAGraphLayoutFinished()
{
    Q_D(DotGraphView);
    graph_t *g = d->m_layoutThread.g();
    bool result = loadLibrary(g, d->m_layoutThread.layoutCommand());
    if (result)
        // file name can be taken from m_loadThread both in sync and async loading cases
        // see comment in loadLibrarySync()
        d->m_graph->dotFileName(d->m_loadThread.dotFileName());
    else {
        Q_ASSERT(!d->m_canvas);
        QGraphicsScene *newCanvas = new QGraphicsScene();
        QGraphicsSimpleTextItem *loadingLabel = newCanvas->addSimpleText(i18n("Failed to open %1", d->m_loadThread.dotFileName()));
        loadingLabel->setZValue(100);
        centerOn(loadingLabel);
        setScene(newCanvas);
        d->m_canvas = newCanvas;
    }

    if (g) {
        gvFreeLayout(d->m_layoutThread.gvc(), g);
        agclose(g);
    }
    d->m_layoutThread.processed_finished();
}

void DotGraphView::slotSelectNode(const QString &nodeName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << nodeName;
    GraphNode *node = dynamic_cast<GraphNode *>(graph()->elementNamed(nodeName));
    if (node == nullptr)
        return;
    node->setSelected(true);
    if (node->canvasNode()) {
        node->canvasNode()->modelChanged();
        slotElementSelected(node->canvasNode(), Qt::NoModifier);
    }
}

void DotGraphView::centerOnNode(const QString &nodeId)
{
    GraphNode *node = dynamic_cast<GraphNode *>(graph()->elementNamed(nodeId));
    if (node == nullptr)
        return;
    if (node->canvasNode()) {
        centerOn(node->canvasNode());
    }
}

}

#include "moc_dotgraphview.cpp"
