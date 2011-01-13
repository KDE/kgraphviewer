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


#include "dotgraphscene.h"

#include "canvasedge.h"
#include "canvasnode.h"

#include <KDebug>

#include <QPainter>
#include <QGraphicsItem>

using namespace KGraphViz;

DotGraphScene::DotGraphScene(QObject* parent)
  : QGraphicsScene(parent)
{
  kDebug();
}

DotGraphScene::~DotGraphScene()
{
  kDebug();
}


void DotGraphScene::drawForeground(QPainter* painter, const QRectF& rect)
{
#ifdef KGRAPHVIZ_GRAPHICSVIEW_DEBUG
  painter->save();

  foreach(const QGraphicsItem* item, items()) {
    if (dynamic_cast<const CanvasNode*>(item))
      painter->setPen(Qt::red);
    else if (dynamic_cast<const CanvasEdge*>(item))
      painter->setPen(Qt::green);
    else
      painter->setPen(Qt::yellow);

    painter->drawRect(item->boundingRect());
  }

  painter->setPen(QPen(Qt::magenta, 1, Qt::DashLine));
  painter->drawRect(itemsBoundingRect());

  painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
  painter->drawRect(sceneRect());

  painter->restore();
#endif
  
  QGraphicsScene::drawForeground(painter, rect);
}