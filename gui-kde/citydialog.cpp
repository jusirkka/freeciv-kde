#include "citydialog.h"
#include "ui_citydialog.h"
#include "citymap.h"
#include "cityview.h"
#include <QPixmap>
#include "sprite.h"
#include "messagebox.h"
#include "cityinfowidget.h"
#include "productiondialog.h"
#include "governordialog.h"
#include "logging.h"
#include <QKeyEvent>


#include "city.h"
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
  , m_production(new ProductionDialog(cities, this))
  , m_governor(new GovernorDialog(cities, this))
{
  m_ui->setupUi(this);
  setWindowFlag(Qt::WindowStaysOnTopHint, false);

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
  // next/prev buttons
  updateButtons();

  connect(m_cities, &CityView::orderingChanged,
          this, &CityDialog::updateButtons);

  connect(m_ui->nextButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->next(m_city));
  });

  connect(m_ui->previousButton, &QPushButton::clicked, this, [=] () {
    changeCity(m_cities->prev(m_city));
  });

  // production/governor buttons
  connect(m_ui->productionButton, &QPushButton::clicked, this, [=] () {
    m_production->changeCity(m_city);
    m_production->show();
  });

  connect(m_ui->governorButton, &QPushButton::clicked, this, [=] () {
    m_governor->changeCity(m_city);
    m_governor->show();
  });

  // filter out enter key in unit lists
  m_ui->supportedFrame->installEventFilter(this);
  m_ui->presentFrame->installEventFilter(this);
}

bool CityDialog::eventFilter(QObject *obj, QEvent *ev) {
  if (ev->type() != QEvent::KeyPress) return QDialog::eventFilter(obj, ev);
  auto k = static_cast<QKeyEvent*>(ev);
  if (k->key() != Qt::Key_Enter && k->key() != Qt::Key_Return) return QDialog::eventFilter(obj, ev);
  if (obj == m_ui->supportedFrame || obj == m_ui->presentFrame) {
    // qCDebug(FC) << "unit list frame";
    auto widget = qobject_cast<UnitListWidget*>(obj);
    auto p = QCursor::pos();
    auto items = widget->findChildren<UnitItem*>();
    for (auto item: items) {
      // qCDebug(FC) << "unit item" << item->rect() << item->mapFromGlobal(p);
      if (item->rect().contains(item->mapFromGlobal(p))) {
        item->handleEnterKey();
        return true;
      }
    }
  }
  return QDialog::eventFilter(obj, ev);
}


CityDialog::~CityDialog()
{
  delete m_ui;
}

city* CityDialog::current() const {
  return m_city;
}

void CityDialog::refresh(city* c) {
  if (c == m_city) {
    changeCity(c);
  } else {
    if (m_production->isVisible()) {
      m_production->refresh(c);
    }
    if (m_governor->isVisible()) {
      m_governor->refresh(c);
    }
  }
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
  // next/prev buttons
  updateButtons();
  // gov/prod dialogs work independently - only refresh
  if (m_production->isVisible()) {
    m_production->refresh(m_city);
  }
  if (m_governor->isVisible()) {
    m_governor->refresh(m_city);
  }

  setWindowTitle(Title(m_city));
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


QString CityDialog::Title(city* c) {

  QString name = city_name_get(c);
  QString pop = population_to_text(city_population(c));
  int s = city_size_get(c);

  if (city_unhappy(c)) {
    /* TRANS: city dialog title */
    return QString(_("%1 - size %2 - %3 citizens - DISORDER")).arg(name).arg(s).arg(pop);
  }

  if (city_celebrating(c)) {
    /* TRANS: city dialog title */
    return QString(_("%1 - size %2 - %3 citizens - celebrating")).arg(name).arg(s).arg(pop);
  }

  if (city_happy(c)) {
    /* TRANS: city dialog title */
    return QString(_("%1 - size %2 - %3 citizens - happy")).arg(name).arg(s).arg(pop);
  }

  /* TRANS: city dialog title */
  return QString(_("%1 - size %2 - %3 citizens")).arg(name).arg(s).arg(pop);
}





