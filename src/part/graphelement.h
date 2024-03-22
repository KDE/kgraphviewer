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

// lib
#include "dotrenderop.h"
// Qt
#include <QObject>
#include <QMap>
#include <QTextStream>

namespace KGraphViewer
{
class CanvasElement;

/**
 * The base of all Graphviz DOT graph elements (nodes, edges, subgraphs,
 * graphs). It is used to store the element attributes
 */
class GraphElement : public QObject
{
    Q_OBJECT
public:
    GraphElement();
    GraphElement(const GraphElement &element);

    ~GraphElement() override
    {
    }

    inline void setId(const QString &id)
    {
        m_attributes[KEY_ID] = id;
    }
    inline void setStyle(const QString &ls)
    {
        m_attributes[KEY_STYLE] = ls;
    }
    inline void setShape(const QString &lc)
    {
        m_attributes[KEY_SHAPE] = lc;
    }
    inline void setColor(const QString &nt)
    {
        m_attributes[KEY_COLOR] = nt;
    }
    inline void setLineColor(const QString &nt)
    {
        m_attributes[KEY_COLOR] = nt;
    }
    inline void setBackColor(const QString &nc)
    {
        m_attributes[KEY_BGCOLOR] = nc;
    }

    inline QString id() const
    {
        return m_attributes[KEY_ID];
    }
    inline QString style() const
    {
        return m_attributes[KEY_STYLE];
    }
    inline QString shape() const
    {
        return m_attributes[KEY_SHAPE];
    }
    inline QString color() const
    {
        return m_attributes[KEY_COLOR];
    }
    inline QString lineColor() const
    {
        return m_attributes[KEY_COLOR];
    }
    virtual QString backColor() const;

    inline void setLabel(const QString &label)
    {
        m_attributes[KEY_LABEL] = label;
    }
    inline const QString label() const
    {
        return m_attributes[KEY_LABEL];
    }

    inline unsigned int fontSize() const
    {
        return m_attributes[KEY_FONTSIZE].toUInt();
    }
    inline void setFontSize(unsigned int fs)
    {
        m_attributes[KEY_FONTSIZE] = QString::number(fs);
    }
    inline QString fontName() const
    {
        return m_attributes[KEY_FONTNAME];
    }
    inline void setFontName(const QString &fn)
    {
        m_attributes[KEY_FONTNAME] = fn;
    }
    inline QString fontColor() const
    {
        return m_attributes[KEY_FONTCOLOR];
    }
    inline void setFontColor(const QString &fc)
    {
        m_attributes[KEY_FONTCOLOR] = fc;
    }

    inline const DotRenderOpVec &renderOperations() const
    {
        return m_renderOperations;
    };
    void setRenderOperations(const DotRenderOpVec &drov);
    /**
     * indicates the version of the render operations, gets increased every time
     * @c setRenderOperations gets called.
     */
    inline quint32 renderOperationsRevision() const
    {
        return m_renderOperationsRevision;
    };

    inline double z() const
    {
        return m_z;
    }
    inline void setZ(double thez)
    {
        m_z = thez;
    }

    inline QString shapeFile() const
    {
        return m_attributes[KEY_SHAPEFILE];
    }
    inline void setShapeFile(const QString &sf)
    {
        m_attributes[KEY_SHAPEFILE] = sf;
    }

    inline QString url() const
    {
        return m_attributes[KEY_URL];
    }
    inline void setUrl(const QString &theUrl)
    {
        m_attributes[KEY_URL] = theUrl;
    }

    virtual void updateWithElement(const GraphElement &element);

    inline QMap<QString, QString> &attributes()
    {
        return m_attributes;
    }
    inline const QMap<QString, QString> &attributes() const
    {
        return m_attributes;
    }

    inline QList<QString> &originalAttributes()
    {
        return m_originalAttributes;
    }
    inline const QList<QString> &originalAttributes() const
    {
        return m_originalAttributes;
    }

    virtual inline void storeOriginalAttributes()
    {
        m_originalAttributes = m_attributes.keys();
    }

    virtual void removeAttribute(const QString &attribName);

    inline CanvasElement *canvasElement()
    {
        return m_ce;
    }
    inline const CanvasElement *canvasElement() const
    {
        return m_ce;
    }
    inline void setCanvasElement(CanvasElement *ce)
    {
        m_ce = ce;
    }

    inline void setSelected(bool s)
    {
        m_selected = s;
    }
    inline bool isSelected()
    {
        return m_selected;
    }

    bool isVisible() const
    {
        return m_visible;
    }
    void setVisible(bool v)
    {
        m_visible = v;
    }

    void exportToGraphviz(void *element) const;

Q_SIGNALS:
    void changed();

protected:
    QMap<QString, QString> m_attributes;
    QList<QString> m_originalAttributes;

    CanvasElement *m_ce;

    static const QString KEY_ID;
    static const QString KEY_STYLE;
    static const QString KEY_LABEL;
    static const QString KEY_SHAPE;
    static const QString KEY_SHAPEFILE;
    static const QString KEY_COLOR;
    static const QString KEY_BGCOLOR;
    static const QString KEY_URL;
    static const QString KEY_FONTSIZE;
    static const QString KEY_FONTNAME;
    static const QString KEY_FONTCOLOR;
    static const QString KEY_FILLCOLOR;

private:
    double m_z;
    bool m_visible;

    DotRenderOpVec m_renderOperations;
    quint32 m_renderOperationsRevision;

    bool m_selected;
};

QTextStream &operator<<(QTextStream &s, const GraphElement &n);

}

#endif
