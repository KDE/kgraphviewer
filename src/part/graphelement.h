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
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
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
class GraphElement: public QMap<QString,QString>
{
public:
  GraphElement();
  
  virtual ~GraphElement() {}
  
  inline void id(const QString& id) {(*this)["id"]=id;}
  inline void style(const QString& ls) {(*this)["style"]=ls;}
  inline void shape(const QString& lc) {(*this)["shape"]=lc;}
  inline void color(const QString& nt) {(*this)["color"]=nt;}
  inline void lineColor(const QString& nt) {(*this)["color"]=nt;}
  inline void backColor(const QString& nc) {(*this)["bgcolor"]=nc;}
  
  inline const QString id() const {return (*this)["id"];}
  inline const QString style() const {return (*this)["style"];}
  inline const QString shape() const {return (*this)["shape"];}
  inline const QString color() const {return (*this)["color"];}
  inline const QString lineColor() const {return (*this)["color"];}
  inline const QString backColor() const {return (*this)["bgcolor"];}
  
  inline void label(const QString& label) {(*this)["label"]=label;}
  inline const QString label() const {return (*this)["label"];}

  inline unsigned int fontSize() const {return (*this)["fontsize"].toUInt();}
  inline void fontSize(unsigned int fs) {(*this)["fontsize"]=QString::number(fs);}
  inline const QString fontName() const {return (*this)["fontname"];}
  inline void fontName(const QString& fn) {(*this)["fontname"]=fn;}
  inline const QString fontColor() const {return (*this)["fontcolor"];}
  inline void fontColor(const QString& fc) {(*this)["fontcolor"] = fc;}

  inline DotRenderOpVec& renderOperations() {return m_renderOperations;};
  inline const DotRenderOpVec& renderOperations() const {return m_renderOperations;};
  inline void renderOperations(DotRenderOpVec& drov) {m_renderOperations = drov;};
  
  inline const double z() const {return m_z;}
  inline void z(double thez) {m_z = thez;}
  
  inline const QString shapeFile() const {return (*this)["shapefile"];}
  inline void shapeFile(const QString& sf) {(*this)["shapefile"] = sf;}
  
  inline const QString url() const {return (*this)["URL"];}
  inline void url(const QString& theUrl) {(*this)["URL"] = theUrl;}
  
private:
  double m_z;

  DotRenderOpVec m_renderOperations;
};


QTextStream& operator<<(QTextStream& s, const GraphElement& n);

#endif



