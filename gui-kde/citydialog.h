#pragma once

#include <QDialog>

class QTableWidgetItem;

struct city;


namespace Ui {
class CityDialog;
}

namespace KV {

class CityView;
class ProductionDialog;
class GovernorDialog;

class CityDialog : public QDialog
{
  Q_OBJECT

public:
  explicit CityDialog(CityView* cities, QWidget *parent = nullptr);
  ~CityDialog();

  city* current() const;
  void refresh(city* c);

public slots:

  void changeCity(city* c);
  void updateButtons();

signals:

  void cityChanged(city* c);

private slots:

  void sellProperty(QTableWidgetItem*);

private:

  void updateProperty();
  void updateTitle();

private:
  Ui::CityDialog *m_ui;
  city* m_city = nullptr;
  CityView* m_cities;
  ProductionDialog* m_production;
  GovernorDialog* m_governor;
};

}