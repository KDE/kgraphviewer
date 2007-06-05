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

#ifndef _KGRAPHVIEWERPART_H_
#define _KGRAPHVIEWERPART_H_

#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <kaction.h>
#include <ktabwidget.h>
#include <kdirwatch.h>

#include "dotgraphview.h"

class QWidget;
class QPainter;
class KURL;
class KSelectAction;
/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Gaël de Chalendar <kleag@free.fr>
 */
class kgraphviewerPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kgraphviewerPart(QWidget *parentWidget, const char *widgetName,
                     QObject *parent, const char *name/*, const QStringList &args*/);

    
    /**
     * Destructor
     */
    virtual ~kgraphviewerPart();

  // Return information about the part
  static KAboutData* createAboutData();

public slots:
  void slotHide(KParts::Part* part);

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();


protected slots:
  void print();
  void printPreview();
  void slotPageSetup();
  void reload();
  void slotZoomIn();
  void slotZoomOut();
  void slotSelectLayoutAlgo();

private:
  DotGraphView *m_widget;
  KDirWatch* m_watch;
  KSelectAction* m_layoutAlgoSelectAction;
  KSelectAction* m_view_bevMenu;
};

class KInstance;
class KAboutData;

class kgraphviewerPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    kgraphviewerPartFactory();
    virtual ~kgraphviewerPartFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const char *classname, const QStringList &args );
    static KInstance* instance();
 
private:
    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif // _KGRAPHVIEWERPART_H_
