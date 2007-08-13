/* This file is part of KGraphViewer.
   Copyright (C) 2005 Gael de Chalendar <kleag@free.fr>

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

#ifndef _KGRAPHVIEWERPART_H_
#define _KGRAPHVIEWERPART_H_

#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <kaction.h>
#include <ktabwidget.h>
#include <kdirwatch.h>

#include "dotgraphview.h"

class QWidget;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Gael de Chalendar <kleag@free.fr>
 */
class kgraphviewerPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    kgraphviewerPart(QWidget *parentWidget, QObject *parent);

    
    /**
     * Destructor
     */
    virtual ~kgraphviewerPart();

  // Return information about the part
  static KAboutData* createAboutData();

  inline DotGraph* graph() {return m_widget->graph();}
  inline const DotGraph* graph() const {return m_widget->graph();}


public slots:
  void slotHide(KParts::Part* part);
  void slotUpdate();
  void prepareAddNewElement();
  void prepareAddNewEdge();
  void setReadOnly();
  void setReadWrite();

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

private:
  DotGraphView *m_widget;
  KDirWatch* m_watch;
  QString m_file;
};

class KComponentData;
class KAboutData;

class kgraphviewerPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    kgraphviewerPartFactory();
    virtual ~kgraphviewerPartFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget, 
                                            QObject *parent, 
                                            const char *classname, const QStringList &args );
    static KComponentData componentData();
 
private:
    static KComponentData s_instance;
    static KAboutData* s_about;
};

#endif // _KGRAPHVIEWERPART_H_
