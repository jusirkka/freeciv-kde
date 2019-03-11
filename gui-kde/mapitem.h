#ifndef MAPITEM_H
#define MAPITEM_H

#include <QGraphicsObject>

namespace KV {

class MapItem: public QGraphicsObject
{
  Q_OBJECT

public:

  explicit MapItem(QGraphicsItem* parent = nullptr);

  void paint(QPainter* painter,
             const QStyleOptionGraphicsItem* option,
             QWidget* widget = nullptr) override;
  QRectF boundingRect() const override;
};

}


#endif // MAPITEM_H
