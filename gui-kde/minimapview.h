#pragma once

#include <QWidget>
#include <QThread>
#include <QSemaphore>


namespace KV {

class MinimapView : public QWidget
{
  Q_OBJECT

public:

  explicit MinimapView(QWidget *parent = nullptr);
  virtual ~MinimapView() = default;

protected:

  void paintEvent(QPaintEvent * event);
  void mousePressEvent(QMouseEvent * event);
  void mouseMoveEvent(QMouseEvent * event);
  void mouseReleaseEvent(QMouseEvent * event);

private:

  void viewportCenterTo(const QPoint& pos);

private:

  // resizing variables
  QPoint m_cursor;
  QPoint m_topLeft;
  QSize m_size;
  bool m_left;
  bool m_upper;
};


}

