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

#include "graphelement.h"
#include "canvaselement.h"
#include "dotdefaults.h"
#include "kgraphviewerlib_debug.h"

#include <math.h>

#include <QDebug>

#include <qregularexpression.h>
#include <graphviz/gvc.h>

namespace KGraphViewer
{
const QString GraphElement::KEY_ID = QLatin1String("id");
const QString GraphElement::KEY_STYLE = QLatin1String("style");
const QString GraphElement::KEY_LABEL = QLatin1String("label");
const QString GraphElement::KEY_SHAPE = QLatin1String("shape");
const QString GraphElement::KEY_SHAPEFILE = QLatin1String("shapefile");
const QString GraphElement::KEY_COLOR = QLatin1String("color");
const QString GraphElement::KEY_BGCOLOR = QLatin1String("bgcolor");
const QString GraphElement::KEY_URL = QLatin1String("URL");
const QString GraphElement::KEY_FONTSIZE = QLatin1String("fontsize");
const QString GraphElement::KEY_FONTNAME = QLatin1String("fontname");
const QString GraphElement::KEY_FONTCOLOR = QLatin1String("fontcolor");
const QString GraphElement::KEY_FILLCOLOR = QLatin1String("fillcolor");

GraphElement::GraphElement()
    : QObject()
    , m_attributes()
    , m_originalAttributes()
    , m_ce(nullptr)
    , m_z(1.0)
    , m_renderOperations()
    , m_renderOperationsRevision(0)
    , m_selected(false)
{
    /*  label("");
      id("");
      style(DOT_DEFAULT_STYLE);
      shape(DOT_DEFAULT_SHAPE);
      lineColor(DOT_DEFAULT_LINECOLOR);
      backColor(DOT_DEFAULT_BACKCOLOR);
      fontName(DOT_DEFAULT_FONTNAME);
      fontColor(DOT_DEFAULT_FONTCOLOR);
      url("");
      shapeFile("");*/
    setFontSize(DOT_DEFAULT_FONTSIZE);
}

GraphElement::GraphElement(const GraphElement &element)
    : QObject()
    , m_attributes()
    , m_originalAttributes()
    , m_ce(element.m_ce)
    , m_z(element.m_z)
    , m_renderOperations()
    , m_renderOperationsRevision(0)
    , m_selected(element.m_selected)
{
    updateWithElement(element);
}

void GraphElement::setRenderOperations(const DotRenderOpVec &drov)
{
    m_renderOperations = drov;
    ++m_renderOperationsRevision;
}

void GraphElement::updateWithElement(const GraphElement &element)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << element.id();
    bool modified = false;
    if (element.z() != m_z) {
        m_z = element.z();
        modified = true;
    }
    QMap<QString, QString>::const_iterator it = element.attributes().constBegin();
    for (; it != element.attributes().constEnd(); it++) {
        const QString &attrib = it.key();
        if ((!m_attributes.contains(attrib)) || (m_attributes[attrib] != it.value())) {
            m_attributes[attrib] = it.value();
            if (attrib == "z") {
                bool ok;
                setZ(m_attributes[attrib].toDouble(&ok));
            }
            modified = true;
        }
    }
    if (modified) {
        qCDebug(KGRAPHVIEWERLIB_LOG) << "modified: update render operations";
        setRenderOperations(element.m_renderOperations);
        /*    foreach (DotRenderOp op, m_renderOperations)
            {
              QString msg;
              QTextStream dd(&msg);
              dd << "an op: " << op.renderop << " ";
              foreach (int i, op.integers)
              {
                dd << i << " ";
              }
              dd << op.str;
              qCDebug(KGRAPHVIEWERLIB_LOG) << msg;
            }
            g() << "modified: emiting changed";*/
        emit changed();
    }
    qCDebug(KGRAPHVIEWERLIB_LOG) << "done" << m_renderOperations.size();
}

QString GraphElement::backColor() const
{
    if (m_attributes.find(KEY_FILLCOLOR) != m_attributes.end()) {
        return m_attributes[KEY_FILLCOLOR];
    } else if ((m_attributes.find(KEY_COLOR) != m_attributes.end()) && (m_attributes[KEY_STYLE] == QLatin1String("filled"))) {
        return m_attributes[KEY_COLOR];
    } else {
        return DOT_DEFAULT_NODE_BACKCOLOR;
    }
}

void GraphElement::removeAttribute(const QString &attribName)
{
    qCDebug(KGRAPHVIEWERLIB_LOG) << attribName;
    m_attributes.remove(attribName);
    emit changed();
}

void GraphElement::exportToGraphviz(void *element) const
{
    QMap<QString, QString>::const_iterator it, it_end;
    it = attributes().begin();
    it_end = attributes().end();
    for (; it != it_end; it++) {
        if (!it.value().isEmpty()) {
            if (it.key() == "label") {
                QString label = it.value();
                if (label != "label") {
                    label.replace(QRegularExpression("\n"), "\\n");
                               qCDebug(KGRAPHVIEWERLIB_LOG) << it.key() << "=\"" << label << "\",";
                    agsafeset(element, it.key().toUtf8().data(), label.toUtf8().data(), QString().toUtf8().data());
                }
            } else if (it.key() == "_draw_" || it.key() == "_ldraw_") {
            } else if (originalAttributes().isEmpty() || originalAttributes().contains(it.key())) {
                //         qCDebug(KGRAPHVIEWERLIB_LOG) << it.key() << it.value();

                agsafeset(element, it.key().toUtf8().data(), it.value().toUtf8().data(), QString().toUtf8().data());
            }
        }
    }
}

QTextStream &operator<<(QTextStream &s, const GraphElement &n)
{
    QMap<QString, QString>::const_iterator it, it_end;
    bool firstAttr = true;
    it = n.attributes().begin();
    it_end = n.attributes().end();
    for (; it != it_end; it++) {
        if (!it.value().isEmpty()) {
            if (it.key() == "label") {
                QString label = it.value();
                if (label != "label") {
                    label.replace(QRegularExpression("\n"), "\\n");
                               qCDebug(KGRAPHVIEWERLIB_LOG) << it.key() << "=\"" << label << "\",";
                    if (firstAttr)
                        firstAttr = false;
                    else
                        s << ',';
                    s << it.key() << "=\"" << label << '"';
                }
            } else if (it.key() == "_draw_" || it.key() == "_ldraw_") {
            } else if (n.originalAttributes().isEmpty() || n.originalAttributes().contains(it.key())) {
                //         qCDebug(KGRAPHVIEWERLIB_LOG) << it.key() << it.value();

                if (firstAttr)
                    firstAttr = false;
                else
                    s << ',';
                s << it.key() << "=\"" << it.value() << '"';
            }
        }
    }
    return s;
}

}

#include "moc_graphelement.cpp"
