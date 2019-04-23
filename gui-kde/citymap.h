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
  QSize minimumSizeHint() const override;
  void resizeEvent(QResizeEvent *event) override;

public slots:

  void changeCity(city* c);

signals:

  void governorChanged(city* c, int);

private:

  void createDisplayPixmap(const QSize& s);

private:

  city* m_city = nullptr;
  QPixmap m_displayPixmap;
  QPixmap m_cityPixmap;

};

}

