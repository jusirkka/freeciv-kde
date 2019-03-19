#pragma once

#include <QFrame>
#include <QLabel>

struct city;
struct unit;
class QHBoxLayout;

namespace KV {

class UnitListWidget : public QFrame
{
  Q_OBJECT
public:
  explicit UnitListWidget(QWidget *parent = nullptr);

public slots:

  void changeCity(city* c);

private:

  virtual void createUnits() = 0;

protected:

  city* m_city = nullptr;
  QHBoxLayout* m_lay;

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


protected:

  void mousePressEvent(QMouseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void enterEvent(QEvent *event) override;

  void contextMenuEvent(QContextMenuEvent *ev) override;

protected:

  QPixmap m_pix;
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

