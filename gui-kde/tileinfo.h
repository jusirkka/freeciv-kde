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

protected:

  void paintEvent(QPaintEvent *event) override;

private:

  QFont m_infoFont;
  QStringList m_lines;

};



}
#endif // TILEINFO_H
