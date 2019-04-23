#include "governordialog.h"
#include "ui_governordialog.h"
#include "cityview.h"
#include "citydialog.h"
#include "messagebox.h"
#include "inputbox.h"
#include <QMenu>

#include "cma_fec.h"
#include "client_main.h"
#include "specialist.h"
extern "C" {
#include "citydlg_g.h"
}


using namespace KV;

GovernorDialog::GovernorDialog(CityView* cities, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::GovernorDialog)
  , m_cities(cities)
{
  m_ui->setupUi(this);
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
  setWindowFlag(Qt::Dialog, false);
  setWindowFlag(Qt::Window, true);

  m_priority[O_FOOD] = m_ui->fpri;
  m_priority[O_SHIELD] = m_ui->ppri;
  m_priority[O_TRADE] = m_ui->tpri;
  m_priority[O_GOLD] = m_ui->gpri;
  m_priority[O_LUXURY] = m_ui->lpri;
  m_priority[O_SCIENCE] = m_ui->spri;

  SliderIterator itp(m_priority);
  while (itp.hasNext()) {
    itp.next();

    connect(itp.value(), &QSlider::valueChanged, this, [=] (int v) {
      m_edited.factor[itp.key()] = v;
      updateButtonState();
      updateResultsBox();
    });
  }

  m_surplus[O_FOOD] = m_ui->fmin;
  m_surplus[O_SHIELD] = m_ui->pmin;
  m_surplus[O_TRADE] = m_ui->tmin;
  m_surplus[O_GOLD] = m_ui->gmin;
  m_surplus[O_LUXURY] = m_ui->lmin;
  m_surplus[O_SCIENCE] = m_ui->smin;

  SpinBoxIterator its(m_surplus);
  while (its.hasNext()) {
    its.next();

    connect(its.value(), QOverload<int>::of(&QSpinBox::valueChanged), this, [=] (int v) {
      m_edited.minimal_surplus[its.key()] = v;
      updateButtonState();
      updateResultsBox();
    });
  }

  connect(m_ui->celButton, &QToolButton::clicked, this, [=] (bool on) {
    m_edited.require_happy = on;
    updateButtonState();
    updateResultsBox();
  });

  connect(m_ui->cpri, &QSlider::valueChanged, this, [=] (int v) {
    m_edited.happy_factor = v;
    updateButtonState();
    updateResultsBox();
  });

  // copy
  m_ui->copyButton->setEnabled(false);

  // open, save, delete
  m_ui->openButton->setDefaultAction(m_ui->actionOpen);
  m_ui->saveButton->setDefaultAction(m_ui->actionSave);
  m_ui->deleteButton->setDefaultAction(m_ui->actionDelete);

  // next/prev buttons
  m_ui->nextButton->setEnabled(false);
  m_ui->previousButton->setEnabled(false);

  connect(m_cities, &CityView::orderingChanged,
          this, &GovernorDialog::updateCityButtons);

  connect(m_ui->nextButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->next(m_city));
  });

  connect(m_ui->previousButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->prev(m_city));
  });
}

GovernorDialog::~GovernorDialog()
{
  delete m_ui;
}

void GovernorDialog::on_enabledBox_clicked(bool on) {
  if (on) {
    cm_init_parameter(&m_parameters);
    if (testParameters(&m_parameters)) {
      cma_put_city_under_agent(m_city, &m_parameters);
    } else {
      // apply fail safe parameters
      for (int e = 0; e < O_LAST; e++) {
        m_parameters.minimal_surplus[e] = -20;
      }
      cma_put_city_under_agent(m_city, &m_parameters);
    }
    cm_copy_parameter(&m_edited, &m_parameters);
    m_editing = false;
    refresh_city_dialog(m_city);
  } else {
    cma_release_city(m_city);
  }
}

void GovernorDialog::on_copyButton_clicked() {
  m_editing = true;
  cm_copy_parameter(&m_edited, &m_parameters);
  refresh_city_dialog(m_city);
}

void GovernorDialog::on_resetButton_clicked() {
  resetSettingsBox();
  refresh_city_dialog(m_city);
}

void GovernorDialog::on_applyButton_clicked() {
  cm_copy_parameter(&m_parameters, &m_edited);
  if (testParameters(&m_parameters)) {
    cma_put_city_under_agent(m_city, &m_parameters);
  } else {
    // this shouldn't really happen
    MessageBox ask(this, "The edited settings are not valid anymore", "Cannot apply");
    ask.setStandardButtons(QMessageBox::Ok);
    ask.exec();
  }
  refresh_city_dialog(m_city);
}

void GovernorDialog::on_actionSave_triggered() {
  KV::InputBox ask(this,
                   _("What should we name the new governor preset?"),
                   _("Save current settings"),
                   _("New preset"));

  if (ask.exec() != QDialog::Accepted) return;
  auto name = ask.input().trimmed();
  if (name.isEmpty()) return;

  m_editing = false;

  cmafec_preset_add(name.toUtf8(), &m_edited);
  refresh_city_dialog(m_city);
}

bool GovernorDialog::testParameters(const cm_parameter* p) const {
  auto res = cm_result_new(m_city);
  cm_query_result(m_city, p, res, false);
  bool valid = res->found_a_valid;
  cm_result_destroy(res);
  return valid;
}

void GovernorDialog::on_actionOpen_triggered() {
  QMenu menu;
  int num = cmafec_preset_num();
  for (int i = 0; i < num; i++) {
    auto a = new QAction(cmafec_preset_get_descr(i));
    a->setEnabled(testParameters(cmafec_preset_get_parameter(i)));
    a->setData(i);
    menu.addAction(a);
  }

  auto a = menu.exec(QCursor::pos());
  if (a == nullptr) return;
  int sel = a->data().toInt();

  governorChanged(m_city, sel);
}

void GovernorDialog::governorChanged(city* c, int preset) {
  if (c != m_city) {
    changeCity(c);
  }
  if (!testParameters(cmafec_preset_get_parameter(preset))) return;

  cm_copy_parameter(&m_parameters, cmafec_preset_get_parameter(preset));
  cm_copy_parameter(&m_edited, cmafec_preset_get_parameter(preset));
  cma_put_city_under_agent(m_city, &m_parameters);
  m_editing = false;
  refresh_city_dialog(m_city);
}

void GovernorDialog::on_actionDelete_triggered() {
  QMenu menu;
  int num = cmafec_preset_num();
  for (int i = 0; i < num; i++) {
    auto a = new QAction(cmafec_preset_get_descr(i));
    connect(a, &QAction::triggered, this, [this, i] () {
      StandardMessageBox ask(this,
                             QString("Do you really want delete %1?")
                             .arg(cmafec_preset_get_descr(i)),
                             "Delete preset");
      if (ask.exec() == QMessageBox::Ok) {
        cmafec_preset_remove(i);
      }
    });
    menu.addAction(a);
  }

  menu.exec(QCursor::pos());
  refresh_city_dialog(m_city);
}


void GovernorDialog::refresh(city* c) {
  if (c != m_city) return;

  cm_parameter par;
  bool cma = cma_is_city_under_agent(m_city, &par);
  m_ui->enabledBox->blockSignals(true);
  m_ui->enabledBox->setChecked(cma);
  m_ui->enabledBox->blockSignals(false);

  m_ui->governorFrame->setEnabled(cma);

  if (cma) {
    int idx = -1;
    if (!m_editing) {
      // preset - find it
      idx = cmafec_preset_get_index_of_parameter(&par);
      m_editing = idx < 0;
    }
    m_ui->copyButton->setEnabled(!m_editing);
    m_ui->settingsBox->setEnabled(m_editing);
    if (!m_editing) {
      m_ui->nameLabel->setText(cmafec_preset_get_descr(idx));
    } else {
      m_ui->nameLabel->setText("Untitled");
    }

    if (cm_are_parameter_equal(&m_parameters, &m_edited)) {
      // no edits: initialize controls & results
      cm_copy_parameter(&m_parameters, &par);
      cm_copy_parameter(&m_edited, &par);
      resetSettingsBox();
      updateResultsBox();
    }
  }
  updateButtonState();
  setWindowTitle(QString("%1: Governor").arg(CityDialog::Title(m_city)));
}

void GovernorDialog::changeCity(city *c) {
  m_city = c;

  cm_parameter par;
  bool cma = cma_is_city_under_agent(m_city, &par);
  m_ui->enabledBox->blockSignals(true);
  m_ui->enabledBox->setChecked(cma);
  m_ui->enabledBox->blockSignals(false);

  m_ui->governorFrame->setEnabled(cma);

  if (cma) {
    // test if there's a preset
    int idx = cmafec_preset_get_index_of_parameter(&par);
    m_editing = idx < 0;
    m_ui->copyButton->setEnabled(!m_editing);
    m_ui->settingsBox->setEnabled(m_editing);

    if (!m_editing) {
      m_ui->nameLabel->setText(cmafec_preset_get_descr(idx));
    } else {
      m_ui->nameLabel->setText("Untitled");
    }

    cm_copy_parameter(&m_parameters, &par);
    cm_copy_parameter(&m_edited, &par);
    resetSettingsBox();
    updateResultsBox();
  }

  updateButtonState();
  setWindowTitle(QString("%1: Governor").arg(CityDialog::Title(m_city)));
}


void GovernorDialog::updateCityButtons() {
  bool canGo = m_cities->hasNext(m_city);
  m_ui->nextButton->setEnabled(canGo);
  m_ui->nextButton->setText(canGo ? city_name_get(m_cities->next(m_city)) : "Next");

  canGo = m_cities->hasPrev(m_city);
  m_ui->previousButton->setEnabled(canGo);
  m_ui->previousButton->setText(canGo ? city_name_get(m_cities->prev(m_city)) : "Previous");
}


void GovernorDialog::resetSettingsBox() {

  cm_copy_parameter(&m_edited, &m_parameters);

  SpinBoxIterator its(m_surplus);
  while (its.hasNext()) {
    its.next();
    its.value()->blockSignals(true);
    its.value()->setValue(m_edited.minimal_surplus[its.key()]);
    its.value()->blockSignals(false);
  }

  SliderIterator itp(m_priority);
  while (itp.hasNext()) {
    itp.next();
    itp.value()->blockSignals(true);
    itp.value()->setValue(m_edited.factor[itp.key()]);
    itp.value()->blockSignals(false);
  }

  m_ui->celButton->blockSignals(true);
  m_ui->celButton->setChecked(m_edited.require_happy);
  m_ui->celButton->blockSignals(false);

  m_ui->cpri->blockSignals(true);
  m_ui->cpri->setValue(m_edited.happy_factor);
  m_ui->cpri->blockSignals(false);
}

static QString get_city_growth_string(city *pcity, int surplus);
static QString get_prod_complete_string(city *pcity, int surplus);

void GovernorDialog::updateResultsBox() {
  auto res = cm_result_new(m_city);
  cm_query_result(m_city, &m_edited, res, false);

  m_ui->resultsBox->setEnabled(res->found_a_valid);

  if (!res->found_a_valid) {
    cm_result_destroy(res);
    return;
  }

  QString s;
  const QString line = "<b>%1:</b> %2<br/>";

  s += line.arg("F/P/T").arg(QString("%1/%2/%3")
                             .arg(res->surplus[O_FOOD])
                             .arg(res->surplus[O_SHIELD])
                             .arg(res->surplus[O_TRADE]));

  s += line.arg("G/L/S").arg(QString("%1/%2/%3")
                             .arg(res->surplus[O_GOLD])
                             .arg(res->surplus[O_LUXURY])
                             .arg(res->surplus[O_SCIENCE]));

  s += line
      .arg("People (W/%1)").arg(specialists_abbreviation_string())
      .arg(QString("%1/%2%3")
           .arg(city_size_get(m_city) - cm_result_specialists(res))
           .arg(specialists_string(res->specialists))
           .arg(res->happy ? " happy" : ""));

  s += line
      .arg("City grows")
      .arg(get_city_growth_string(m_city, res->surplus[O_FOOD]));

  s += line
      .arg("Production completed")
      .arg(get_prod_complete_string(m_city, res->surplus[O_SHIELD]));

  m_ui->resultsLabel->setText(s);

  cm_result_destroy(res);
}

void GovernorDialog::updateButtonState() {
  if (!m_ui->enabledBox->isChecked()) {
    // cma not enabled
    updateCityButtons();
    m_ui->resetButton->setEnabled(false);
    m_ui->applyButton->setEnabled(false);
    m_ui->closeButton->setEnabled(true);
    return;
  }
  // cma is on - are there any edits?
  if (m_editing && !cm_are_parameter_equal(&m_parameters, &m_edited)) {
    m_ui->enabledBox->setEnabled(false);

    m_ui->actionOpen->setEnabled(false);
    m_ui->actionSave->setEnabled(false);
    m_ui->actionDelete->setEnabled(can_client_issue_orders());

    m_ui->previousButton->setEnabled(false);
    m_ui->nextButton->setEnabled(false);

    m_ui->resetButton->setEnabled(true);
    m_ui->applyButton->setEnabled(m_ui->resultsBox->isEnabled());

    m_ui->closeButton->setEnabled(false);

    return;
  }

  // cma is on, no unsaved edits
  m_ui->enabledBox->setEnabled(true);

  m_ui->actionOpen->setEnabled(can_client_issue_orders());
  m_ui->actionSave->setEnabled(can_client_issue_orders() && m_editing);
  m_ui->actionDelete->setEnabled(can_client_issue_orders());

  updateCityButtons();

  m_ui->resetButton->setEnabled(false);
  m_ui->applyButton->setEnabled(false);

  m_ui->closeButton->setEnabled(true);

}

// nicked & adapted from cma_fec.c
static QString get_city_growth_string(city *pcity, int surplus) {
  int stock, cost, turns;

  if (surplus == 0) return "never";

  stock = pcity->food_stock;
  cost = city_granary_size(city_size_get(pcity));

  stock += surplus;

  if (stock >= cost) {
    turns = 1;
  } else if (surplus > 0) {
    turns = ((cost - stock - 1) / surplus) + 1 + 1;
  } else {
    if (stock < 0) {
      turns = -1;
    } else {
      turns = (stock / surplus);
    }
  }

  return QString(PL_("%1 turn", "%1 turns", turns)).arg(turns);
}

static QString get_prod_complete_string(city *pcity, int surplus) {
  int stock, cost, turns;

  if (surplus <= 0) return "never";

  if (city_production_has_flag(pcity, IF_GOLD)) {
    return improvement_name_translation(pcity->production.value.building);
  }

  stock = pcity->shield_stock + surplus;
  cost = city_production_build_shield_cost(pcity);

  if (stock >= cost) {
    turns = 1;
  } else if (surplus > 0) {
    turns = ((cost - stock - 1) / surplus) + 1 + 1;
  } else {
    if (stock < 0) {
      turns = -1;
    } else {
      turns = (stock / surplus);
    }
  }

  return QString(PL_("%1 turn", "%1 turns", turns)).arg(turns);
}
