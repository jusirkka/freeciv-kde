#include "cityinfowidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextStream>

#include "city.h"
#include "citydlg_common.h"
#include "specialist.h"
#include "game.h"

using namespace KV;

CityInfoWidget::CityInfoWidget(QWidget* parent)
  : QFrame(parent)
  , m_foodLabel(new QLabel)
  , m_goldLabel(new QLabel)
  , m_granaryLabel(new QLabel)
  , m_corruptionLabel(new QLabel)
  , m_pollutionLabel(new QLabel)
  , m_statusLabel(new QLabel)
{
  m_template_2 = "<table>"
                 "<tr><td><b>%1</b></td><td>%2</td></tr>"
                 "<tr><td><b>%3</b></td><td>%4</td></tr>"
                 "</table>";

  m_template_3 = "<table>"
                 "<tr><td><b>%1</b></td><td>%2</td></tr>"
                 "<tr><td><b>%3</b></td><td>%4</td></tr>"
                 "<tr><td><b>%5</b></td><td>%6</td></tr>"
                 "</table>";

  auto h = new QHBoxLayout;
  h->addWidget(m_foodLabel);
  h->addWidget(m_goldLabel);
  h->addWidget(m_granaryLabel);
  h->addWidget(m_corruptionLabel);

  auto v = new QVBoxLayout;
  v->addWidget(m_pollutionLabel);
  auto spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
  v->addItem(spacer);
  h->addLayout(v);

  v = new QVBoxLayout;
  v->addWidget(m_statusLabel);
  spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
  v->addItem(spacer);
  h->addLayout(v);

  setLayout(h);
}

void CityInfoWidget::changeCity(city *c) {
  m_city = c;

  m_foodLabel->clear();
  m_goldLabel->clear();
  m_granaryLabel->clear();
  m_corruptionLabel->clear();
  m_pollutionLabel->clear();
  m_statusLabel->clear();

  if (m_city == nullptr) return;

  QString food;
  QTextStream s(&food);
  s << m_city->prod[O_FOOD] <<
       forcesign << " (" << m_city->surplus[O_FOOD] << ")";

  QString prod;
  s.setString(&prod);
  s << m_city->prod[O_SHIELD] + m_city->waste[O_SHIELD] <<
       forcesign << " (" << m_city->surplus[O_SHIELD] << ")";

  QString trade;
  s.setString(&trade);
  s << m_city->prod[O_TRADE] + m_city->waste[O_TRADE] <<
       forcesign << " (" << m_city->surplus[O_TRADE] << ")";

  m_foodLabel->setText(m_template_3
                       .arg(_("Food:")).arg(food)
                       .arg(_("Prod:")).arg(prod)
                       .arg(_("Trade:")).arg(trade));



  QString gold;
  s.setString(&gold);
  s << m_city->prod[O_GOLD] <<
       forcesign << " (" << m_city->surplus[O_GOLD] << ")";

  m_goldLabel->setText(m_template_3
                       .arg(_("Gold:")).arg(gold)
                       .arg(_("Luxury:")).arg(m_city->prod[O_LUXURY])
                       .arg(_("Science:")).arg(m_city->prod[O_SCIENCE]));


  QString granary = QString("%1/%2")
      .arg(m_city->food_stock)
      .arg(city_granary_size(city_size_get(m_city)));

  QString changeIn;
  auto granaryturns = city_turns_to_grow(m_city);
  if (granaryturns == 0) {
    /* TRANS: city growth is blocked.  Keep short. */
    changeIn =  _("blocked");
  } else if (granaryturns == FC_INFINITY) {
    /* TRANS: city is not growing.  Keep short. */
    changeIn = _("never");
  } else {
    /* A negative value means we'll have famine in that many turns.
       But that's handled down below. */
    /* TRANS: city growth turns.  Keep short. */
    changeIn = QString(PL_("%1 turn", "%1 turns", abs(granaryturns)))
        .arg(abs(granaryturns));
  }


  m_granaryLabel->setText(m_template_3
                          .arg(_("Granary:")).arg(granary)
                          .arg(_("Change in:")).arg(changeIn)
                          .arg(_("Culture:")).arg(m_city->client.culture));

  QString illness = "N/A";
  if (game.info.illness_on) {
    illness = QString("%1").arg(city_illness_calc(m_city, nullptr, nullptr, nullptr, nullptr));
  }

  m_corruptionLabel->setText(m_template_3
                          .arg(_("Corruption:")).arg(m_city->waste[O_TRADE])
                          .arg(_("Waste:")).arg(m_city->waste[O_SHIELD])
                          .arg(_("Plague Risk:")).arg(illness));


  QString steal = _("Not stolen");
  if (m_city->steal > 0) {
    steal = QString(_("%1 times")).arg(m_city->steal);
  }

  m_pollutionLabel->setText(m_template_2
                          .arg(_("Pollution:")).arg(m_city->pollution)
                          .arg(_("Tech Stolen:")).arg(steal));

  QString people = QString("W/%1:").arg(specialists_abbreviation_string());

  QString specs;
  int numSpecs = 0;
  specialist_type_iterate(sp) {
    numSpecs += m_city->specialists[sp];
    specs += QString("/%1").arg(m_city->specialists[sp]);
  } specialist_type_iterate_end;

  specs = QString("%1%2").arg(city_size_get(m_city) - numSpecs).arg(specs);

  QString feels = QString("%1/%2/%3/%4")
      .arg(m_city->feel[CITIZEN_HAPPY][FEELING_FINAL])
      .arg(m_city->feel[CITIZEN_CONTENT][FEELING_FINAL])
      .arg(m_city->feel[CITIZEN_UNHAPPY][FEELING_FINAL])
      .arg(m_city->feel[CITIZEN_ANGRY][FEELING_FINAL]);

  m_statusLabel->setText(m_template_2
                         .arg(people).arg(specs)
                         .arg("H/C/U/A:").arg(feels));

  // Tooltips
  QString tip;
  char buf[1024];

  // food label
  get_city_dialog_output_text(m_city, O_FOOD, buf, sizeof(buf));
  tip = QString(buf) + "\n";
  get_city_dialog_output_text(m_city, O_SHIELD, buf, sizeof(buf));
  tip += QString(buf) + "\n";
  get_city_dialog_output_text(m_city, O_TRADE, buf, sizeof(buf));
  tip += buf;
  m_foodLabel->setToolTip(tip);

  // gold label
  get_city_dialog_output_text(m_city, O_GOLD, buf, sizeof(buf));
  tip = QString(buf) + "\n";
  get_city_dialog_output_text(m_city, O_SCIENCE, buf, sizeof(buf));
  tip += QString(buf) + "\n";
  get_city_dialog_output_text(m_city, O_LUXURY, buf, sizeof(buf));
  tip += buf;
  m_goldLabel->setToolTip(tip);

  // granary
  get_city_dialog_culture_text(m_city, buf, sizeof(buf));
  m_granaryLabel->setToolTip(buf);

  // corruption
  get_city_dialog_illness_text(m_city, buf, sizeof(buf));
  m_corruptionLabel->setToolTip(buf);

  // pollution
  get_city_dialog_pollution_text(m_city, buf, sizeof(buf));
  m_pollutionLabel->setToolTip(buf);



}
