#ifndef MINIMAPVIEW_H
#define MINIMAPVIEW_H

#include <QWidget>
#include <QThread>
#include <QSemaphore>


namespace KV {

class MinimapThread;


class MinimapView : public QWidget
{
  Q_OBJECT

  friend class MinimapThread;

public:

  using FrameVector = QVector<QPixmap>;

  explicit MinimapView(QWidget *parent = nullptr);

  void reset();

protected:

  void paintEvent(QPaintEvent * event);
  void resizeEvent(QResizeEvent * event);
  void mousePressEvent(QMouseEvent * event);
  void mouseMoveEvent(QMouseEvent * event);
  void mouseReleaseEvent(QMouseEvent * event);
  void wheelEvent(QWheelEvent * event);
  void moveEvent(QMoveEvent * event);
  void showEvent(QShowEvent * event);

private slots:

  void zoomIn();
  void zoomOut();

private:

  void drawViewport(QPainter* painter);
  qreal scale() const;
  void setScale(qreal scale);

  // worker / consumer interface
  void acquireEmptyFrame();
  void releaseFilledFrame();
  int bufferSize() const;
  FrameVector& buffer();


private:

  qreal m_w_ratio;
  qreal m_h_ratio;
  QBrush m_background;
  QPoint m_cursor1;
  QPoint m_cursor2;
  QPoint m_topLeft;
  QSize m_size;
  QPoint m_position;
  qreal m_scale;
  bool m_left;
  bool m_upper;

  // worker / consumer interface
  MinimapThread* m_thread;
  int m_bufferSize;
  int m_bufferIndex;
  FrameVector m_buffer;
  QSemaphore m_filled;
  QSemaphore m_empty;

};


class MinimapThread : public QThread
{
  Q_OBJECT
public:

  MinimapThread(MinimapView* parent);
  ~MinimapThread();


protected:

  void run() override;

private:

  MinimapView* m_parent;

};


}




#endif // MINIMAPVIEW_H
