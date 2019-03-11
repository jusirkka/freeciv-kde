#include "endturnrect.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "spritewidget.h"
#include "govmenu.h"
#include "sprite.h"
#include "application.h"

#include "control.h"
extern "C" {
#include "repodlgs_g.h"
}
#include "client_main.h"
#include "tilespec.h"
#include "text.h"
#include "climisc.h"

static const char *turnAreaStyle =
  "QPushButton {"
    "background-color: transparent;"
    "border: none;"
    "color: rgb(30, 175, 30);"
    "font-size: %1pt;"
  "}"
  "QPushButton:hover {"
    "color: rgb(38, 225, 38);" // Lighter green
  "}"
  "QPushButton:disabled {"
    "color: rgb(172, 175, 175);" // Gray
  "}";

// Highlighted version, inverts hover/not hover
static const char *turnAreaStyleHigh =
  "QPushButton {"
    "background-color: transparent;"
    "border: none;"
    "color: rgb(38, 225, 38);"
    "font-size: %1pt;"
  "}"
  "QPushButton:hover {"
    "color: rgb(30, 175, 30);" // Darker green
  "}"
  "QPushButton:disabled {"
    "color: rgb(172, 175, 175);" // Gray
  "}";

using namespace KV;

EndTurnRect::EndTurnRect(QWidget *parent)
  : QWidget(parent)
  , m_done(new QPushButton)

{
  m_fontSize = QFontMetrics(font()).height() + 1;

  setStyleSheet("QFrame { background-color: rgba(0, 0, 0, 135); }");


  auto hbox = new QHBoxLayout;
  hbox->setSpacing(0);
  hbox->addStretch();

  m_researchIndicator = new SpriteWidget;
  hbox->addWidget(m_researchIndicator);
  connect(m_researchIndicator, &QPushButton::clicked, &science_report_dialog_popup);

  m_governmentIndicator = new SpriteWidget;
  m_governmentIndicator->setMenu(new GovMenu());
  hbox->addWidget(m_governmentIndicator);

  m_pollutionIndicator = new SpriteWidget;
  hbox->addWidget(m_pollutionIndicator);

  m_nuclearIndicator = new SpriteWidget;
  hbox->addWidget(m_nuclearIndicator);
  hbox->addStretch();

  auto layout = new QVBoxLayout;
  layout->addLayout(hbox);

  hbox = new QHBoxLayout;
  hbox->setSpacing(0);
  hbox->addStretch();
  for (int i = 0; i < 10; ++i) {
    m_taxIndicators[i] = new SpriteWidget;
    m_taxIndicators[i]->setId(i);
    connect(m_taxIndicators[i], &SpriteWidget::buttonClicked,
            this, &EndTurnRect::changeTaxRateClick);
    connect(m_taxIndicators[i], &SpriteWidget::wheelRolled,
            this, &EndTurnRect::changeTaxRateWheel);
    hbox->addWidget(m_taxIndicators[i]);
  }
  hbox->addStretch();

  layout->addLayout(hbox);

  m_done->setStyleSheet(QString(turnAreaStyle).arg(m_fontSize));
  connect(m_done, &QPushButton::clicked, &key_end_turn);
  layout->addWidget(m_done, Qt::AlignCenter);
  m_done->setText(_("Turn Done"));
  m_done->setFocusPolicy(Qt::NoFocus);

  setLayout(layout);

  updateArea();

  connect(Application::instance(), &Application::updateTurnDone,
          this, &EndTurnRect::setTurnButtonHighlight);
  connect(Application::instance(), &Application::toggleTurnDone,
          this, &EndTurnRect::setTurnButtonEnabled);
  connect(Application::instance(), &Application::updateGameInfo,
          this, &EndTurnRect::updateArea);
}



void EndTurnRect::setTurnButtonEnabled(bool enabled)
{
  m_done->setEnabled(enabled);
}

void EndTurnRect::setTurnButtonHighlight(bool highlight) {
  if (highlight) {
    m_done->setStyleSheet(
          QString(turnAreaStyleHigh).arg(m_fontSize));
  } else {
    m_done->setStyleSheet(
      QString(turnAreaStyle).arg(m_fontSize));
  }
}


void EndTurnRect::updateArea() {
  if (client_is_global_observer()) {
    hide();
    return;
  }

  show();

  // update indicators
  m_researchIndicator->setSprite(client_research_sprite());
  m_governmentIndicator->setSprite(client_government_sprite());
  m_nuclearIndicator->setSprite(client_cooling_sprite());
  m_pollutionIndicator->setSprite(client_warming_sprite());

  // Update tax rates
  int d = 0;
  auto s = get_tax_sprite(tileset, O_LUXURY);
  for (; d < client.conn.playing->economic.luxury / 10; ++d) {
    m_taxIndicators[d]->setSprite(s);
  }
  s = get_tax_sprite(tileset, O_SCIENCE);
  for (; d < (client.conn.playing->economic.science
              + client.conn.playing->economic.luxury) / 10; ++d) {
    m_taxIndicators[d]->setSprite(s);
  }
  s = get_tax_sprite(tileset, O_GOLD);
  for (; d < 10; ++d) {
    m_taxIndicators[d]->setSprite(s);
  }
  // Set tooltips
  m_governmentIndicator->setToolTip(get_government_tooltip());
  m_nuclearIndicator->setToolTip(get_nuclear_winter_tooltip());
  m_pollutionIndicator->setToolTip(get_global_warming_tooltip());
  m_researchIndicator->setToolTip(get_bulb_tooltip());
  for (d = 0; d < 10; ++d) {
    m_taxIndicators[d]->setToolTip(_("Shows your current luxury/science/tax "
                                     "rates. Use mouse wheel to change them"));
  }
  QFontMetrics fm(font());
  setMinimumWidth(qMax(get_tax_sprite(tileset, O_LUXURY)->pm.width() * 10
                  + 25, fm.width(m_done->text())));
  setMinimumHeight(fm.height() + client_research_sprite()->pm.height()
                   + get_tax_sprite(tileset, O_LUXURY)->pm.height() + 25);
  updateGeometry();
  auto delta = parentWidget()->size() - size();
  move(delta.width(), delta.height());
}

void EndTurnRect::changeTaxRateWheel(int delta, int id)
{
  if (id < client.conn.playing->economic.luxury / 10) {
    changeTaxRate(O_LUXURY, delta);
  } else if (id < (client.conn.playing->economic.science / 10
                   + client.conn.playing->economic.luxury / 10)) {
    changeTaxRate(O_SCIENCE, delta);
  } else {
    changeTaxRate(O_GOLD, delta);
  }
}

void EndTurnRect::changeTaxRateClick(Qt::MouseButton button, int id)
{
  int delta;

  if (button == Qt::LeftButton) {
    delta = -1;
  } else if (button == Qt::RightButton) {
    delta = +1;
  } else {
    return;
  }

  if (id < client.conn.playing->economic.luxury / 10) {
    changeTaxRate(O_LUXURY, delta);
  } else if (id < (client.conn.playing->economic.science / 10
                   + client.conn.playing->economic.luxury / 10)) {
    changeTaxRate(O_SCIENCE, delta);
  } else {
    changeTaxRate(O_GOLD, delta);
  }
}

/****************************************************************************
  Change the given tax rate by delta.
****************************************************************************/
void EndTurnRect::changeTaxRate(output_type_id type, int delta)
{
  int l = client.conn.playing->economic.luxury / 10;
  int s = client.conn.playing->economic.science / 10;

  switch (type) {
  case O_LUXURY:
    l += delta;
    s -= delta;
    break;
  case O_SCIENCE:
    s += delta;
    break;
  case O_GOLD:
    l -= delta;
    break;
  default:
    return;
  }

  // Normalize
  l = qMax(qMin(l, 10), 0);
  s = qMax(qMin(s, 10), 0);

  if (l + s <= 10) {
    dsend_packet_player_rates(&client.conn, (10 - l - s) * 10,
                              l * 10, s * 10);
  }
}

