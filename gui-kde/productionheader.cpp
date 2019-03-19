#include "productionheader.h"
#include "ui_productionheader.h"
#include "application.h"
#include <QPixmap>
#include "sprite.h"
#include "messagebox.h"
#include "buildables.h"

#include "citydlg_common.h"
#include "client_main.h"

using namespace KV;

static const char* buttonStyle = "QPushButton{border: 0px;}"
                                 "QPushButton:hover "
                                 "{background-color: rgb(225, 225, 225)}";

ProductionHeader::ProductionHeader(QWidget *parent)
  : QWidget(parent)
  , m_ui(new Ui::ProductionHeader)
{
  m_ui->setupUi(this);
  // progressbar
  m_ui->buildProgress->setFixedHeight(2 * fontMetrics().height() + 2);
  // product button
  m_ui->targetButton->setStyleSheet(buttonStyle);
  m_ui->targetButton->setToolTip(_("Click left button to change current "
                                   "production"));
  connect(m_ui->targetButton, &QPushButton::clicked,
          this, &ProductionHeader::popupTargets);
  // buy button
  m_ui->buyButton->setIcon(Application::Icon("help-donate"));
  connect(m_ui->buyButton, &QPushButton::clicked,
          this, &ProductionHeader::buy);


}

void ProductionHeader::changeCity(city *c) {
  m_city = c;
  bool canDo = m_city != nullptr;
  m_ui->buildProgress->setEnabled(canDo);
  m_ui->targetButton->setEnabled(canDo);
  m_ui->buyButton->setEnabled(canDo);
  if (!canDo) return;

  int cost = city_production_build_shield_cost(m_city);
  m_ui->buildProgress->setRange(0, cost);
  m_ui->buildProgress->setValue(qMin(cost, m_city->shield_stock));
  m_ui->buildProgress->setAlignment(Qt::AlignCenter);

  char buf[1024];
  get_city_dialog_production(m_city, buf, sizeof(buf));
  auto fmt = QString("(%p%) %1\n%2")
      .arg(city_production_name_translation(m_city))
      .arg(buf);
  m_ui->buildProgress->setFormat(fmt);

  QPixmap pix;
  if (m_city->production.kind == VUT_UTYPE) {
    pix = get_unittype_sprite(tileset, m_city->production.value.utype,
                              direction8_invalid())->pm;
  } else {
    pix = get_building_sprite(tileset, m_city->production.value.building)->pm;
  }

  m_ui->targetButton->setIconSize(pix.size());
  m_ui->targetButton->setIcon(QIcon(pix));

  m_ui->buyButton->setEnabled(false);
  if (!client_is_observer() && client_has_player()) {
    int cost = m_city->client.buy_cost;
    auto s = QString(PL_("Buy (%1 gold)", "Buy (%1 gold)", cost)).arg(cost);
    m_ui->buyButton->setText(s);
    if (client_player()->economic.gold >= cost && cost != 0) {
      m_ui->buyButton->setEnabled(true);
    }
  } else {
    m_ui->buyButton->setText(_("Buy"));
  }
}

void ProductionHeader::buy() {

  if (!can_client_issue_orders()) return;
  if (m_city == nullptr) return;

  const char *name = city_production_name_translation(m_city);
  int cost = m_city->client.buy_cost;
  auto text = QString(PL_("Buy %1 for %2 gold?", "Buy %1 for %2 gold?", cost))
      .arg(name)
      .arg(cost);
  int gold = client_player()->economic.gold;
  auto title = QString(PL_("Treasury contains %1 gold.",
                           "Treasury contains %1 gold.", gold))
      .arg(gold);

  StandardMessageBox ask(this, text, title);

  if (ask.exec() == QMessageBox::Ok) {
    city_buy_production(m_city);
  }
}

void ProductionHeader::popupTargets() {
  auto w = new Buildables(m_city, BuildablesFilter::UnitsBuildingsWonders, this);
  connect(w, &Buildables::selected, this, [=] (universal u){
    city_change_production(m_city, &u);
    w->close();
  });
  w->show();
}


ProductionHeader::~ProductionHeader() {
  delete m_ui;
}


