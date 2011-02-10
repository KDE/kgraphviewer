/*
    Copyright (C) 2010 Kevin Funk <krf@electrostorm.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef KGRAPHVIZ_DOTGRAPHSCENE_H
#define KGRAPHVIZ_DOTGRAPHSCENE_H

#include <QGraphicsScene>

namespace KGraphViz
{

class DotGraphScene : public QGraphicsScene
{
public:
  explicit DotGraphScene(QObject* parent = 0);
  virtual ~DotGraphScene();

protected:
  virtual void drawForeground(QPainter* painter, const QRectF& rect);
};

}

#endif // KGRAPHVIZ_DOTGRAPHSCENE_H
