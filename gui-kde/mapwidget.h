#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>

#include "tilespec.h"

namespace KV {

class MapWidget: public QWidget
{
  Q_OBJECT

public:

  explicit MapWidget(QWidget* parent = nullptr);

  void paint(QPainter *painter, QPaintEvent *event);
  void updateCursor(cursor_type);

protected:

  void paintEvent(QPaintEvent *event) override;
  void keyPressEvent(QKeyEvent * event) override;
  void resizeEvent(QResizeEvent * event)  override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;
  void leaveEvent(QEvent *event) override;

private slots:

  void animate();

private:

  int m_cursorFrame;
  int m_cursor;

};

}

#endif // MAPWIDGET_H
