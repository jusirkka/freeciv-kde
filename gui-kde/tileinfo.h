#ifndef TILEINFO_H
#define TILEINFO_H

#include <QWidget>

struct tile;

namespace KV {
class TileInfo : public QWidget
{
  Q_OBJECT
public:
  explicit TileInfo(tile* ptile, QWidget *parent = nullptr);

signals:

public slots:
};

}
#endif // TILEINFO_H
