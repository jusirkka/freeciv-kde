#pragma once

#include <QDialog>

class QItemSelection;
class QMimeData;

struct city;

namespace Ui {
class ProductionDialog;
}

namespace KV {

class CityView;
class WorkModel;
class BuildablesFilter;


class ProductionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ProductionDialog(CityView* cities, QWidget *parent = nullptr);
  ~ProductionDialog();

  void refresh(city* c);

public slots:

  void changeCity(city* c);

signals:

  void cityChanged(city* c);

private slots:

  void on_actionCut_triggered();
  void on_actionCopy_triggered();
  void on_actionPaste_triggered();
  void on_actionOpen_triggered();
  void on_actionSave_triggered();
  void on_futureCheckBox_toggled(bool);
  void on_unitsCheckBox_toggled(bool);
  void on_wondersCheckBox_toggled(bool);
  void on_buildingsCheckBox_toggled(bool);

  void updateEditButtons(const QItemSelection&, const QItemSelection&);
  void updateCityButtons();

private:
  Ui::ProductionDialog *m_ui;
  CityView* m_cities;
  WorkModel* m_workModel;
  BuildablesFilter* m_filter;
  city* m_city = nullptr;
  QMimeData* m_clipboard = nullptr;
};

}
