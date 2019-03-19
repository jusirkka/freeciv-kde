#pragma once

#include <QWidget>
#include <QPixmap>

struct city;
struct canvas;

namespace KV {

class CityMap : public QWidget
{
  Q_OBJECT
public:
  explicit CityMap(QWidget *parent = nullptr);
  ~CityMap();

protected:

  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

public slots:

  void changeCity(city* c);

signals:

private:

  QPoint m_rect;
  QPoint m_trans;
  city* m_city = nullptr;
  QPixmap m_pixmap;

};

}

