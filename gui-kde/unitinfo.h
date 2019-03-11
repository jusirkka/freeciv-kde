#ifndef UNITINFO_H
#define UNITINFO_H

#include <QWidget>
#include "unit.h"

namespace KV {

class UnitInfo: public QWidget
{
  Q_OBJECT

public:

  UnitInfo(QWidget* p = nullptr);


protected:

  void paintEvent(QPaintEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);


private slots:

  void updateUnits(unit_list *punits);
  void updateArrowPix();

private:

  QPixmap m_pix;
  QPixmap m_arrow;
  QPixmap m_tile;
  QFont m_font;

  unit* m_unit;
  QRect m_selectionArea;
  QString m_label1;
  QString m_label2;
  bool m_highlightPix;
  int m_numUnits;
  int m_skip;
};

} // namespace KV

#endif // UNITINFO_H
