#pragma once

#include <QDialog>
#include <QMap>

#include "cm.h"

namespace Ui {
class GovernorDialog;
}

struct city;
class QSlider;
class QSpinBox;

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

private slots:

  void on_enabledBox_clicked(bool on);

  void on_copyButton_clicked();
  void on_resetButton_clicked();
  void on_applyButton_clicked();

  void on_actionOpen_triggered();
  void on_actionSave_triggered();
  void on_actionDelete_triggered();


private:

  void resetSettingsBox();
  void updateResultsBox();
  void updateCityButtons();
  void updateButtonState();

  bool testParameters(const cm_parameter* p) const;

private:

  using SliderMap = QMap<Output_type_id, QSlider*>;
  using SliderIterator = QMapIterator<Output_type_id, QSlider*>;

  using SpinBoxMap = QMap<Output_type_id, QSpinBox*>;
  using SpinBoxIterator = QMapIterator<Output_type_id, QSpinBox*>;


  Ui::GovernorDialog *m_ui;
  CityView* m_cities;
  city* m_city = nullptr;

  cm_parameter m_parameters;
  cm_parameter m_edited;

  SliderMap m_priority;
  SpinBoxMap m_surplus;

  bool m_editing = false;
};

}
