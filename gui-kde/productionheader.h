#pragma once

#include <QWidget>

namespace Ui {
class ProductionHeader;
}

struct city;

namespace KV {

class ProductionHeader : public QWidget
{
  Q_OBJECT

public:
  explicit ProductionHeader(QWidget *parent = nullptr);
  ~ProductionHeader();


public slots:

  void changeCity(city* c);

private slots:

  void buy();
  void popupTargets();

private:
  Ui::ProductionHeader *m_ui;
  city* m_city;
};

}

