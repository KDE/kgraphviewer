/* This file is part of KGraphViewer.
   Copyright (C) 2007 Gael de Chalendar <kleag@free.fr>

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

#ifndef GRAPH_ELEMENT_H
#define GRAPH_ELEMENT_H

#include "dotrenderop.h"

#include <QVector>
#include <QList>
#include <QMap>
#include <QtCore/QTextStream>

/**
 * The base of all GraphViz dot graph elements (nodes, edges, subgraphs,
 * graphs). It is used to store the element attributes
 */
class GraphElement: public QObject
{
  Q_OBJECT
public:
  GraphElement();
  GraphElement(const GraphElement& element);
  
  virtual ~GraphElement() {}
  
  inline void id(const QString& id) {m_attributes["id"]=id;}
  inline void style(const QString& ls) {m_attributes["style"]=ls;}
  inline void shape(const QString& lc) {m_attributes["shape"]=lc;}
  inline void color(const QString& nt) {m_attributes["color"]=nt;}
  inline void lineColor(const QString& nt) {m_attributes["color"]=nt;}
  inline void backColor(const QString& nc) {m_attributes["bgcolor"]=nc;}
  
  inline QString id() const {return m_attributes["id"];}
  inline QString style() const {return m_attributes["style"];}
  inline QString shape() const {return m_attributes["shape"];}
  inline QString color() const {return m_attributes["color"];}
  inline QString lineColor() const {return m_attributes["color"];}
  virtual QString backColor() const;
  
  inline void label(const QString& label) {m_attributes["label"]=label;}
  inline const QString label() const {return m_attributes["label"];}

  inline unsigned int fontSize() const {return m_attributes["fontsize"].toUInt();}
  inline void fontSize(unsigned int fs) {m_attributes["fontsize"]=QString::number(fs);}
  inline QString fontName() const {return m_attributes["fontname"];}
  inline void fontName(const QString& fn) {m_attributes["fontname"]=fn;}
  inline QString fontColor() const {return m_attributes["fontcolor"];}
  inline void fontColor(const QString& fc) {m_attributes["fontcolor"] = fc;}

  inline DotRenderOpVec& renderOperations() {return m_renderOperations;};
  inline const DotRenderOpVec& renderOperations() const {return m_renderOperations;};
  inline void renderOperations(DotRenderOpVec& drov) {m_renderOperations = drov;};
  
  inline const double z() const {return m_z;}
  inline void z(double thez) {m_z = thez;}
  
  inline QString shapeFile() const {return m_attributes["shapefile"];}
  inline void shapeFile(const QString& sf) {m_attributes["shapefile"] = sf;}
  
  inline QString url() const {return m_attributes["URL"];}
  inline void url(const QString& theUrl) {m_attributes["URL"] = theUrl;}

  virtual void updateWith(const GraphElement& element);

  inline QMap<QString,QString>& attributes() {return m_attributes;}
  inline const QMap<QString,QString>& attributes() const {return m_attributes;}

  inline QList<QString>& originalAttributes() {return m_originalAttributes;}
  inline const QList<QString>& originalAttributes() const {return m_originalAttributes;}

  virtual inline void storeOriginalAttributes() {m_originalAttributes = m_attributes.keys();}

  
Q_SIGNALS:
    void changed();

protected:
  QMap<QString,QString> m_attributes;
  QList<QString> m_originalAttributes;
  
private:
  double m_z;

  DotRenderOpVec m_renderOperations;

};


QTextStream& operator<<(QTextStream& s, const GraphElement& n);

#endif



