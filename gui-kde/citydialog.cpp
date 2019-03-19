#include "citydialog.h"
#include "ui_citydialog.h"
#include "city.h"
#include "citymap.h"
#include "productionheader.h"
#include "cityview.h"
#include <QPixmap>
#include "sprite.h"
#include "messagebox.h"
#include "cityinfowidget.h"


#include "climisc.h"
#include "tilespec.h"
#include "client_main.h"
#include "citydlg_common.h"
#include "game.h"

using namespace KV;


CityDialog::CityDialog(CityView* cities, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::CityDialog)
  , m_cities(cities)
{
  m_ui->setupUi(this);
  // map widget
  connect(this, &CityDialog::cityChanged,
          m_ui->mapWidget, &CityMap::changeCity);
  // production header
  connect(this, &CityDialog::cityChanged,
          m_ui->productionHeader, &ProductionHeader::changeCity);
  // property list
  m_ui->propertyList->setColumnCount(2);
  connect(m_ui->propertyList, &QTableWidget::itemDoubleClicked,
          this, &CityDialog::sellProperty);
  // city info
  connect(this, &CityDialog::cityChanged,
          m_ui->infoFrame, &CityInfoWidget::changeCity);
  // supported units
  connect(this, &CityDialog::cityChanged,
          m_ui->supportedFrame, &UnitListWidget::changeCity);
  // present units
  connect(this, &CityDialog::cityChanged,
          m_ui->presentFrame, &UnitListWidget::changeCity);
  // buttons
  updateButtons();

  connect(m_cities, &CityView::orderingChanged,
          this, &CityDialog::updateButtons);

  connect(m_cities, &CityView::manageCity,
          this, &CityDialog::changeCity);
}

CityDialog::~CityDialog()
{
  delete m_ui;
}

city* CityDialog::current() const {
  return m_city;
}

void CityDialog::refresh() {
  changeCity(m_city);
}


void CityDialog::changeCity(city *c) {
  m_city = c;

  emit cityChanged(m_city);
  // map widget signalled
  // production header signalled
  // property list
  updateProperty();
  // city info signalled
  // supported units signalled
  // present units signalled
  // buttons
  updateButtons();

  updateTitle();
}

void CityDialog::updateButtons() {
  bool canDo = m_city != nullptr;

  m_ui->productionButton->setEnabled(canDo);
  m_ui->governorButton->setEnabled(canDo);
  m_ui->moreButton->setEnabled(canDo);
  m_ui->nextButton->setEnabled(canDo);
  m_ui->previousButton->setEnabled(canDo);

  if (!canDo) return;

  bool canGo = m_cities->hasNext(m_city);
  m_ui->nextButton->setEnabled(canGo);
  m_ui->nextButton->setText(canGo ? city_name_get(m_cities->next(m_city)) : "Next");

  canGo = m_cities->hasPrev(m_city);
  m_ui->previousButton->setEnabled(canGo);
  m_ui->previousButton->setText(canGo ? city_name_get(m_cities->prev(m_city)) : "Previous");

}


void CityDialog::sellProperty(QTableWidgetItem* selected) {

  if (!can_client_issue_orders()) return;

  auto uid = selected->data(Qt::UserRole).toInt();
  auto building = cid_decode(uid);
  auto imp = building.value.building;

  auto res = test_player_sell_building_now(client_player(), m_city, imp);
  if (res != TR_SUCCESS) return;

  auto price = impr_sell_gold(imp);
  auto msg = QString(PL_("Sell %1 for %2 gold?", "Sell %1 for %2 gold?", price))
      .arg(city_improvement_name_translation(m_city, imp))
      .arg(price);

  StandardMessageBox ask(this, msg, _("Sell improvement?"));
  if (ask.exec() == QMessageBox::Ok) {
    city_sell_improvement(m_city, improvement_number(imp));
  }
}

void CityDialog::updateProperty() {

  m_ui->propertyList->clear();
  m_ui->propertyList->setRowCount(0);

  if (m_city == nullptr) return;

  universal targets[MAX_NUM_PRODUCTION_TARGETS];
  auto numBuilt = collect_already_built_targets(targets, m_city);
  item props[numBuilt];

  name_and_sort_items(targets, numBuilt, props, false, m_city);

  int h = fontMetrics().height() + 6;

  m_ui->propertyList->setRowCount(numBuilt);

  for (int row = 0; row < numBuilt; row++) {
    universal target = props[row].item;
    if (target.kind != VUT_IMPROVEMENT) continue;

    auto building = new QTableWidgetItem;
    building->setData(Qt::UserRole, cid_encode(target));
    auto pix = get_building_sprite(tileset, target.value.building)->pm;
    pix = pix.scaledToHeight(h);
    building->setData(Qt::DecorationRole, pix);
    QString descr = props[row].descr;
    building->setText(descr);
    QSize s(pix.width() + fontMetrics().width(descr) + 15, h);
    building->setSizeHint(s);
    m_ui->propertyList->setItem(row, 0, building);

    auto upkeep = new QTableWidgetItem;
    upkeep->setData(Qt::UserRole, cid_encode(target));
    upkeep->setTextAlignment(Qt::AlignRight);
    auto cost = city_improvement_upkeep(m_city, target.value.building);
    upkeep->setText(QString::number(cost));
    s.setWidth(fontMetrics().width(QString::number(cost)) + 4);
    upkeep->setSizeHint(s);
    m_ui->propertyList->setItem(row, 1, upkeep);
  }
  m_ui->propertyList->resizeColumnsToContents();
}


void CityDialog::updateTitle() {

  if (m_city == nullptr) return;

  QString name = city_name_get(m_city);
  QString pop = population_to_text(city_population(m_city));
  int s = city_size_get(m_city);
  if (city_unhappy(m_city)) {
    /* TRANS: city dialog title */
    setWindowTitle(QString(_("%1 - size %2 - %3 citizens - DISORDER")).arg(name).arg(s).arg(pop));
  } else if (city_celebrating(m_city)) {
    /* TRANS: city dialog title */
    setWindowTitle(QString(_("%1 - size %2 - %3 citizens - celebrating")).arg(name).arg(s).arg(pop));
  } else if (city_happy(m_city)) {
    /* TRANS: city dialog title */
    setWindowTitle(QString(_("%1 - size %2 - %3 citizens - happy")).arg(name).arg(s).arg(pop));
  } else {
    /* TRANS: city dialog title */
    setWindowTitle(QString(_("%1 - size %2 - %3 citizens")).arg(name).arg(s).arg(pop));
  }
}





