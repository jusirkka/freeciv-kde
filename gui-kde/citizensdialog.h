#pragma once

#include <QDialog>

struct city;
class QToolButton;

namespace Ui {
class CitizensDialog;
}

namespace KV {

class CityView;

class CitizensDialog: public QDialog
{
  Q_OBJECT

public:
  explicit CitizensDialog(CityView* cities, QWidget *parent = nullptr);
  ~CitizensDialog();

  void refresh(city* c);

public slots:

  void changeCity(city* c);

signals:

  void cityChanged(city* c);
  void governorChanged(city*, int);

private slots:

  void updateCityButtons();

private:
  void updateNationalities();
  void updateSpecialists();
  void updateAttitude();
  QToolButton* createSpecialistButton(int sp);
private:
  Ui::CitizensDialog *m_ui;
  CityView* m_cities;
  city* m_city = nullptr;
};

}
