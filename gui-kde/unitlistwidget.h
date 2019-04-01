#pragma once

#include <QScrollArea>
#include <QLabel>

struct city;
struct unit;
class QHBoxLayout;

namespace KV {

class UnitListWidget : public QScrollArea
{
  Q_OBJECT
public:
  explicit UnitListWidget(QWidget *parent = nullptr);

public slots:

  void changeCity(city* c);

protected:

  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

private:

  virtual void createUnits() = 0;
  void slide();

protected:

  city* m_city = nullptr;
  QHBoxLayout* m_lay;
  int m_minHeight;

private:

  int m_dx;
  int m_lastX;
  QTimer* m_timer;

};

class SupportedUnitsWidget: public UnitListWidget {
  Q_OBJECT
public:
  explicit SupportedUnitsWidget(QWidget *parent = nullptr);


private:

  void createUnits() override;


};

class PresentUnitsWidget: public UnitListWidget {
  Q_OBJECT
public:
  explicit PresentUnitsWidget(QWidget *parent = nullptr);


private:

  void createUnits() override;

};

class UnitItem: public QLabel {
  Q_OBJECT

public:

  UnitItem(unit* punit, QWidget* parent = nullptr);
  void handleEnterKey();

protected:

  void leaveEvent(QEvent *event) override;
  void enterEvent(QEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

  void contextMenuEvent(QContextMenuEvent *ev) override;

protected:

  QPixmap m_pix;
  QPixmap m_hpix;
  unit* m_unit;


};


class PresentUnitItem: public UnitItem {
  Q_OBJECT

public:

  PresentUnitItem(unit* punit, QWidget* parent = nullptr);

};

class SupportedUnitItem: public UnitItem {
  Q_OBJECT

public:

  SupportedUnitItem(unit* punit, int happy_cost = 0, QWidget* parent = nullptr);

};


}

