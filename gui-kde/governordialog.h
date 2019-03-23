#pragma once

#include <QDialog>

namespace Ui {
class GovernorDialog;
}

struct city;

namespace KV {

class CityView;

class GovernorDialog : public QDialog
{
  Q_OBJECT

public:
  explicit GovernorDialog(CityView* cities, QWidget *parent = nullptr);
  ~GovernorDialog();

  void refresh(city* c);


public slots:

  void changeCity(city* c);

signals:

  void cityChanged(city* c);


private:
  Ui::GovernorDialog *m_ui;
  CityView* m_cities;
};

}
