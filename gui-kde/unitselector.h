#ifndef UNITSELECTOR_H
#define UNITSELECTOR_H

#include <QWidget>
#include <QTimer>

struct tile;
struct unit;

namespace KV {

class UnitSelector: public QWidget
{
  Q_OBJECT

public:

  explicit UnitSelector(tile* t, QWidget *parent = nullptr);

protected:

  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void enterEvent(QEvent *event) override;
  void leaveEvent(QEvent *event) override;

private slots:

  void updateUnits();

private:

  void updateUnitList();
  void createPixmap();

private:

  QPixmap m_pix;
  QPixmap m_pixHigh;
  QSize m_itemSize;
  QList<unit*> m_unitList;

  QFont m_font;
  QFont m_infoFont;

  int m_indexHigh;
  int m_unitOffset;
  tile *m_tile;

  QTimer m_delay;

  bool m_first;
};

}

#endif // UNITSELECTOR_H
