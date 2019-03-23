#include "governordialog.h"
#include "ui_governordialog.h"
#include "cityview.h"


using namespace KV;

GovernorDialog::GovernorDialog(CityView* cities, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::GovernorDialog)
  , m_cities(cities)
{
  m_ui->setupUi(this);
}

GovernorDialog::~GovernorDialog()
{
  delete m_ui;
}

void GovernorDialog::changeCity(city *c) {
}

void GovernorDialog::refresh(city *c) {
}
