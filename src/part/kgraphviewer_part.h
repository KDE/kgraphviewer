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

Q_SIGNALS:
  void graphLoaded();

public slots:
  void slotHide(KParts::Part* part);
  void slotUpdate();
  void prepareAddNewElement(QMap<QString,QString> attribs);
  void prepareAddNewEdge(QMap<QString,QString> attribs);
  void setReadOnly();
  void setReadWrite();
  void saveTo(const QString& fileName);
  void slotRemoveNode(const QString&);
  void slotAddAttribute(const QString&);
  void slotRemoveAttribute(const QString&,const QString&);
  void slotSetGraphAttributes(QMap<QString,QString> attribs);
  void slotAddNewNode(QMap<QString,QString> attribs);
  void slotAddNewNodeToSubgraph(QMap<QString,QString> attribs,QString subgraph);
  void slotAddNewSubgraph(QMap<QString,QString> attribs);
  void slotAddNewEdge(QString src, QString tgt, QMap<QString,QString> attribs);

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
