#include "mapitem.h"
#include <QPainter>
#include <QPaintDeviceWindow>
#include "logging.h"

#include "mapview_common.h"
#include "canvas.h"

using namespace KV;

MapItem::MapItem(QGraphicsItem *parent)
  : QGraphicsObject(parent)
{
//  setPos(- mapview.store->map_pixmap.width() / 2,
//         - mapview.store->map_pixmap.height() / 2);
}




void MapItem::paint(QPainter* painter,
                    const QStyleOptionGraphicsItem*/*option*/,
                    QWidget* /*widget*/)
{
  qCDebug(FC) << "MapItem::paint" << painter->device() << scenePos();
  painter->drawPixmap(QPointF(0, 0), mapview.store->map_pixmap);
}

QRectF MapItem::boundingRect() const {
  return QRectF(0, 0,
                mapview.store->map_pixmap.width(),
                mapview.store->map_pixmap.height());
}

