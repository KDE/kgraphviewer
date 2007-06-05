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


#include "kgraphviewer_part.h"

#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kparts/factory.h>
#include <kaccel.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qiconset.h>
#include <qpixmap.h>
#include <qlayout.h>

#include <iostream>

//#include "kgraphviewersettings.h"
#include "kgraphviewer_partsettings.h"

using namespace KGraphViewer;

kgraphviewerPart::kgraphviewerPart( QWidget *parentWidget, const char *widgetName,
                                    QObject *parent, const char *name/*, const QStringList & */)
: KParts::ReadOnlyPart(parent, name), m_watch(new KDirWatch())
{
  // we need an instance
  setInstance( kgraphviewerPartFactory::instance() );

  // this should be your custom internal widget
  m_widget = new DotGraphView( parentWidget, widgetName );
  m_widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  
  // notify the part that this is our internal widget
  setWidget(m_widget);

  KSelectAction* file_exportMenu = 
        new KSelectAction(i18n("Export Graph"),0,
                      actionCollection(),"file_export");
  connect(file_exportMenu,SIGNAL(activated(int)),m_widget,SLOT(fileExportActivated(int)));
  actionCollection()->insert(file_exportMenu);
  QStringList file_exportActions;
  file_exportActions.push_back(i18n("As Image ..."));
  file_exportMenu->setItems(file_exportActions);
  file_exportMenu->setToolTip(i18n("Allows to export the graph to another format."));  
  file_exportMenu->setWhatsThis(i18n(
  "Use the Export Graph menu to export the graph to another format."
  "There is currently only on export format supported: PNG image."));  

  KStdAction::print(this, SLOT(print()), actionCollection());
  KAccel *print_accel = new KAccel( parentWidget, "PrintAccel" );
  print_accel->insert( "Print", i18n("Print"), i18n("Print the graph using current page setup settings"), CTRL+Key_P, 
                       this, SLOT( print() ) );
  print_accel->readSettings();
  
  KStdAction::printPreview(this, SLOT(printPreview()), actionCollection());
  KAccel *printPreview_accel = new KAccel( parentWidget, "PrintPreviewAccel" );
  printPreview_accel->insert( "Print preview", i18n("Print preview"), i18n("Open the print preview window"), CTRL+SHIFT+Key_P, 
                       this, SLOT( printPreview() ) );
  printPreview_accel->readSettings();
  
  KStdAction::redisplay(this, SLOT(reload()), actionCollection());
  KAccel *reload_accel = new KAccel( parentWidget, "ReloadAccel" );
  reload_accel->insert( "Reload", i18n("Reload"), i18n("Reload the current graph from file"), Key_F5, 
                            this, SLOT( reload() ) );
  reload_accel->readSettings();

  KStdAction::zoomIn(this, SLOT(slotZoomIn()), actionCollection());
  KAccel *zoomIn_accel = new KAccel( parentWidget, "ZoomInAccel" );
  zoomIn_accel ->insert( "ZoomIn", i18n("Zoom In"), 
                         i18n("Zoom in by 10&#37; the currently viewed graph"), Key_F7, 
                         this, SLOT( slotZoomIn() ) );
  zoomIn_accel ->readSettings();
  
  KStdAction::zoomOut(this, SLOT(slotZoomOut()), actionCollection());
  KAccel *zoomOut_accel = new KAccel( parentWidget, "ZoomOutAccel" );
  zoomOut_accel ->insert( "Zoom", i18n("Zoom Out"), i18n("Zoom out by 10&#37; the currently viewed graph"), Key_F8, 
                              this, SLOT( slotZoomOut() ) );
  zoomOut_accel ->readSettings();
  
  m_view_bevMenu = 
        new KSelectAction(i18n("Birds-eye View"),0,
                      actionCollection(),"view_bev");
  connect(m_view_bevMenu,SIGNAL(activated(int)),m_widget,SLOT(viewBevActivated(int)));
  connect(m_widget, SIGNAL(sigViewBevActivated(int)),m_view_bevMenu,SLOT(setCurrentItem(int)));
  actionCollection()->insert(m_view_bevMenu);
  QStringList view_bevActions;
  view_bevActions.push_back(i18n("Top Left"));
  view_bevActions.push_back(i18n("Top Right"));
  view_bevActions.push_back(i18n("Bottom Left"));
  view_bevActions.push_back(i18n("Bottom Right"));
  view_bevActions.push_back(i18n("Automatic"));
  m_view_bevMenu->setItems(view_bevActions);
  m_view_bevMenu->setCurrentItem(m_widget->zoomPos());
  m_view_bevMenu->setToolTip(i18n("Chose the position of the bird's eye view."));  
  m_view_bevMenu->setWhatsThis(i18n(
  "Chose the corner of the window where the bird's eye view will be put or "
  "allow the system to compute the better one."));  
  m_view_bevMenu->setEnabled(KGraphViewerPartSettings::birdsEyeViewEnabled());

  KToggleAction* view_bev_enabledAction = 
        new KToggleAction(i18n("Enable Bird's-eye View"), KGlobal::dirs()->findResource("appdata","pics/kgraphviewer-bev.png"),
                               CTRL+Key_B,
                               actionCollection(), "view_bev_enabled");
  connect(view_bev_enabledAction,SIGNAL(toggled(bool)),m_widget,SLOT(viewBevEnabledToggled(bool)));
  connect(m_widget, SIGNAL(sigViewBevEnabledToggled(bool)),view_bev_enabledAction,SLOT(setChecked(bool)));
  connect(m_widget, SIGNAL(sigViewBevEnabledToggled(bool)),m_view_bevMenu,SLOT(setEnabled(bool)));
  view_bev_enabledAction->setChecked(KGraphViewerPartSettings::birdsEyeViewEnabled());
// std::cerr << KGlobal::dirs()->resourceDirs("icon").join(", ") << std::endl;

  m_layoutAlgoSelectAction = 
    new KSelectAction(i18n("Select Layout Algo"),0,this,
                      SLOT(slotSelectLayoutAlgo()),
                      actionCollection(),"view_layout_algo");
//     new KSelectAction("view_layout_algo");
  actionCollection()->insert(m_layoutAlgoSelectAction);
  QStringList layoutAlgos;
  layoutAlgos.push_back("");
  layoutAlgos.push_back("Dot");
  layoutAlgos.push_back("Neato");
  layoutAlgos.push_back("Twopi");
  layoutAlgos.push_back("Fdp");
  layoutAlgos.push_back("Circo");
  m_layoutAlgoSelectAction->setItems(layoutAlgos);
  m_layoutAlgoSelectAction->setCurrentItem(1);
  m_layoutAlgoSelectAction->setEditable(true);
  m_layoutAlgoSelectAction->setToolTip(i18n("Choose a GraphViz layout algorithm or edit your own one."));  
  m_layoutAlgoSelectAction->setWhatsThis(i18n(
  "Choose a GraphViz layout algorithm or type in your own command that will "
  "generate a graph in the xdot format on its standard output. For example, to"
  "manually specify the <tt>G</tt> option to the dot command, type in: "
  "<tt>dot -Gname=MyGraphName -Txdot </tt>"));  

  KAction *pageSetupAction = new KAction(i18n("&Page setup"), "pagesetup",
                               KShortcut(),
                               this, SLOT(slotPageSetup()),
                               actionCollection(), "file_page_setup");

// set our XML-UI resource file
  setXMLFile("kgraphviewer_part.rc");
}

kgraphviewerPart::~kgraphviewerPart()
{
  delete m_watch; 
  delete m_layoutAlgoSelectAction;
  delete m_view_bevMenu;
}


void kgraphviewerPart::print()
{
  m_widget->print();
}

void kgraphviewerPart::printPreview()
{
  m_widget->printPreview();
}

void kgraphviewerPart::slotPageSetup()
{
  m_widget->pageSetup();
}

bool kgraphviewerPart::openFile()
{
//     std::cerr << "kgraphviewerPart::openFile" << std::endl;
  //  m_widget->loadedDot( m_file );
  if (!m_widget->loadDot(m_file))
  {
    m_widget->hide();
    return false;
  }
//   std::cerr << "Watching file " << m_file << std::endl;
  m_watch->addFile(m_file);
  connect(m_watch, SIGNAL(dirty(const QString &)), m_widget, SLOT(dirty(const QString&)));
  QString label = m_file.section('/',-1,-1);
  
  m_widget->show();
  return true;
}

void kgraphviewerPart::reload()
{
    // this slot is called whenever the File->Reload menu is selected,
    // the Reload shortcut is pressed (usually CTRL+O) or the Reload toolbar
    // button is clicked
//   kdDebug() << "Reload action !" << endl;
  m_widget->reload();
}

void kgraphviewerPart::slotZoomIn()
{
  m_widget->zoomIn();
}

void kgraphviewerPart::slotZoomOut()
{
  m_widget->zoomOut();
}

void kgraphviewerPart::slotSelectLayoutAlgo()
{
  QString text = m_layoutAlgoSelectAction->currentText();
  if (text == "Dot")
  {
    m_widget->setLayoutCommand("dot -Txdot");
  }
  else if (text == "Neato")
  {
    m_widget->setLayoutCommand("neato -Txdot");
  }
  else if (text == "Twopi")
  {
    m_widget->setLayoutCommand("twopi -Txdot");
  }
  else if (text == "Fdp")
  {
    m_widget->setLayoutCommand("fdp -Txdot");
  }
  else if (text == "Circo")
  {
    m_widget->setLayoutCommand("circo -Txdot");
  }
  else 
  {
    m_widget->setLayoutCommand(text);
  }
}

void kgraphviewerPart::slotHide(KParts::Part* part)
{
  if (part == this)
  {
    m_widget->hideToolsWindows();
  }
}

/*It's usually safe to leave the factory code alone.. with the
notable exception of the KAboutData data*/
#include <kaboutdata.h>
#include <klocale.h>

KInstance*  kgraphviewerPartFactory::s_instance = 0L;
KAboutData* kgraphviewerPartFactory::s_about = 0L;

kgraphviewerPartFactory::kgraphviewerPartFactory()
    : KParts::Factory()
{
}

kgraphviewerPartFactory::~kgraphviewerPartFactory()
{
    delete s_instance;
    delete s_about;

    s_instance = 0L;
}

KParts::Part* kgraphviewerPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                        QObject *parent, const char *name,
                                                        const char *classname, const QStringList &args )
{
//     Create an instance of our Part
    kgraphviewerPart* obj = new kgraphviewerPart( parentWidget, widgetName, parent, name );

//     See if we are to be read-write or not
//     if (QCString(classname) == "KParts::ReadOnlyPart")
//         obj->setReadWrite(false);

    return obj;
}

KInstance* kgraphviewerPartFactory::instance()
{
    if( !s_instance )
    {
        s_about = new KAboutData( "kgraphviewerpart", I18N_NOOP("kgraphviewerPart"),
                    "1.0.3", I18N_NOOP( "GraphViz dot files viewer" ),
                    KAboutData::License_GPL,
                    "(c) 2005-2006, Gaël de Chalendar <kleag@free.fr>");
        s_instance = new KInstance(s_about);
    }
    return s_instance;
}

extern "C"
{
  void* init_libkgraphviewerpart()
  {
    KGlobal::locale()->insertCatalogue("kgraphviewer"); 
    return new kgraphviewerPartFactory;
  }
};


#include "kgraphviewer_part.moc"
