#include "citizensdialog.h"
#include "ui_citizensdialog.h"
#include "application.h"
#include "cityview.h"
#include <QMenu>
#include "logging.h"
#include "citydialog.h"
#include "sprite.h"
#include <QToolButton>

#include "city.h"
#include "citizens.h"
#include "specialist.h"
#include "citydlg_common.h"
#include "cma_core.h"

using namespace KV;

CitizensDialog::CitizensDialog(CityView* cities, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::CitizensDialog)
  , m_cities(cities)
{
  m_ui->setupUi(this);
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
  setWindowFlag(Qt::Dialog, false);
  setWindowFlag(Qt::Window, true);

  // specialists box
  // map widget
  connect(this, &CitizensDialog::cityChanged,
          m_ui->mapWidget, &CityMap::changeCity);
  connect(m_ui->mapWidget, &CityMap::governorChanged,
          this, &CitizensDialog::governorChanged);
  // nationalities table
  m_ui->nationsList->setHorizontalHeaderLabels(
        QStringList() << _("#") << _("Flag") << _("Nation"));
  m_ui->nationsList->horizontalHeader()->
      setSectionResizeMode(QHeaderView::ResizeToContents);
  // worker attitude
  // next/prev buttons
  m_ui->nextButton->setEnabled(false);
  m_ui->previousButton->setEnabled(false);


  connect(m_cities, &CityView::orderingChanged,
          this, &CitizensDialog::updateCityButtons);

  connect(m_ui->nextButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->next(m_city));
  });

  connect(m_ui->previousButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->prev(m_city));
  });

}

CitizensDialog::~CitizensDialog() {
  delete m_ui;
}

void CitizensDialog::changeCity(city *c) {

  m_city = c;
  emit cityChanged(m_city);

  // specialists box
  updateSpecialists();
  // map widget: signalled
  // nationalities table
  updateNationalities();
  // worker attitude
  updateAttitude();
  // next/prev buttons
  updateCityButtons();

  setWindowTitle(QString("%1: Citizens").arg(CityDialog::Title(m_city)));
}

void CitizensDialog::refresh(city* c) {
  if (c == m_city) {
    changeCity(c);
  }
}

void CitizensDialog::updateCityButtons() {

  bool canGo = m_cities->hasNext(m_city);
  m_ui->nextButton->setEnabled(canGo);
  m_ui->nextButton->setText(canGo ? city_name_get(m_cities->next(m_city)) : "Next");

  canGo = m_cities->hasPrev(m_city);
  m_ui->previousButton->setEnabled(canGo);
  m_ui->previousButton->setText(canGo ? city_name_get(m_cities->prev(m_city)) : "Previous");

}

void CitizensDialog::updateNationalities() {
  m_ui->nationsList->clearContents();
  int rows = 0;
  m_ui->nationsList->setRowCount(rows);
  citizens_iterate(m_city, pslot, n) {
    m_ui->nationsList->insertRow(rows);

    // number of citizen with nationality n
    auto item = new QTableWidgetItem;
    auto c = citizens_nation_get(m_city, pslot);
    if (c == 0) {
      item->setText("-");
    } else {
      item->setText(QString("%1").arg(c));
    }
    m_ui->nationsList->setItem(rows, 0, item);

    // flag
    item = new QTableWidgetItem;
    auto s = get_nation_flag_sprite(
          get_tileset(), nation_of_player(player_slot_get_player(pslot)));
    if (s != nullptr) {
      int h = fontMetrics().height() + 6;
      item->setData(Qt::DecorationRole, s->pm.scaledToHeight(h));
    } else {
      item->setText("N/A");
    }
    m_ui->nationsList->setItem(rows, 1, item);

    // name of nationality n
    item = new QTableWidgetItem;
    item->setText(
          nation_adjective_for_player(player_slot_get_player(pslot)));
    m_ui->nationsList->setItem(rows, 2, item);

    rows += 1;
  } citizens_iterate_end;
}

void CitizensDialog::updateSpecialists() {
  qDeleteAll(m_ui->specialistsBox->children());
  auto lay = new QHBoxLayout;
  m_ui->specialistsBox->setLayout(lay);
  int spCount = 0;
  specialist_type_iterate(sp) {
    for (int n = 0; n < m_city->specialists[sp]; n++, spCount++) {
      lay->addWidget(createSpecialistButton(sp));
    }
  } specialist_type_iterate_end;
  if (spCount == 0) {
    lay->addWidget(new QLabel("No Specialists."));
  }
  lay->addItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum));
  m_ui->specialistsBox->setEnabled(!cma_is_city_under_agent(m_city, nullptr));
}

QToolButton* CitizensDialog::createSpecialistButton(int sp) {
  auto but = new QToolButton;
  auto s = get_citizen_sprite(
        get_tileset(),
        static_cast<citizen_category>(CITIZEN_SPECIALIST + sp),
        0,
        nullptr);
  if (s != nullptr) {
    but->setIcon(QIcon(s->pm));
  } else {
    but->setText(specialist_abbreviation_translation(specialist_by_number(sp)));
  }
  but->setToolTip(specialist_plural_translation(specialist_by_number(sp)));

  auto menu = new QMenu;
  specialist_type_iterate(other) {
    if (other == sp) continue;
    auto a = new QAction(specialist_plural_translation(specialist_by_number(other)));
    a->setEnabled(city_can_use_specialist(m_city, other));
    connect(a, &QAction::triggered, this, [this, sp, other] () {
      city_change_specialist(m_city, sp, other);
    });
    menu->addAction(a);
  } specialist_type_iterate_end;

  but->setMenu(menu);

  return but;
}

static QVector<int> numbers(city* c, citizen_feeling idx) {
  QVector<int> n;
  n.append(c->feel[CITIZEN_HAPPY][idx]);
  n.append(c->feel[CITIZEN_CONTENT][idx]);
  n.append(c->feel[CITIZEN_UNHAPPY][idx]);
  n.append(c->feel[CITIZEN_ANGRY][idx]);
  return n;
}

void CitizensDialog::updateAttitude() {
  m_ui->base->setNumbers(numbers(m_city, FEELING_BASE));
  m_ui->effects->setNumbers(numbers(m_city, FEELING_EFFECT));
  m_ui->wonders->setNumbers(numbers(m_city, FEELING_FINAL));
  m_ui->luxury->setNumbers(numbers(m_city, FEELING_LUXURY));
  m_ui->martial->setNumbers(numbers(m_city, FEELING_MARTIAL));
  m_ui->nationality->setNumbers(numbers(m_city, FEELING_NATIONALITY));
}

