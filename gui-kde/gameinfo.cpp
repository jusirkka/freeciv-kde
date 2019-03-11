#include "gameinfo.h"
#include <QLabel>
#include <QTimer>
#include <QSpacerItem>
#include <QHBoxLayout>
#include "application.h"

#include "client_main.h"
#include "game.h"
#include "text.h"
#include "research.h"

using namespace KV;

static const char *gameInfoStyle =
  "color: rgb(232, 255, 0); background: transparent;";
static const char *researchInfoStyle =
  "color: rgb(40, 138, 200); background: transparent;";
static const char *blinkStyle =
  "color: rgb(120, 220, 255); background: transparent;";
static const char *epochStyle =
  "color: rgb(180, 0, 0); background: transparent;";

GameInfo::GameInfo(QWidget *parent)
  : QWidget(parent)
  , m_turn(new QLabel)
  , m_gold(new QLabel)
  , m_turnTime(new QLabel)
  , m_research(new QLabel)
  , m_researchTimer(new QTimer(this))
  , m_blinkState(false)
{
  QSpacerItem *si;

  setStyleSheet("QFrame { background-color: rgba(0, 0, 0, 195); }");

  auto layout = new QHBoxLayout;
  QSpacerItem spacer(10, 0);

  m_gold->setStyleSheet(gameInfoStyle);
  layout->addWidget(m_gold);
  layout->addSpacerItem(new QSpacerItem(spacer));

  m_research->setStyleSheet(researchInfoStyle);
  layout->addWidget(m_research);
  layout->addSpacerItem(new QSpacerItem(spacer));

  m_turn->setStyleSheet(gameInfoStyle);
  layout->addWidget(m_turn);
  layout->addSpacerItem(new QSpacerItem(spacer));

  m_turnTime->setStyleSheet(epochStyle);
  layout->addWidget(m_turnTime);
  layout->addSpacerItem(new QSpacerItem(spacer));

  m_researchTimer->setSingleShot(true);
  setLayout(layout);

  connect(m_researchTimer, &QTimer::timeout,
          this, &GameInfo::blink);

  updateInfo();
  connect(Application::instance(), &Application::updateGameInfo,
          this, &GameInfo::updateInfo);
  connect(Application::instance(), &Application::updateTurnTimeout,
          this, &GameInfo::updateTurnTime);
}

void GameInfo::updateInfo() {

  QString y;
  if (game.info.year < 0) {
    y = QString("%1 %2").arg(-game.info.year).arg(game.calendar.negative_year_label);
  } else {
    y = QString("%1 %2").arg(game.info.year).arg(game.calendar.positive_year_label);
  };

  auto s = QString(_("%1 (Turn:%2)")).arg(y).arg(game.info.turn);
  m_turn->setText(s);

  if (client_is_global_observer()) {
    return;
  }

  if (client.conn.playing != nullptr) {
    if (player_get_expected_income(client.conn.playing) > 0) {
      s = QString(_("Gold:%1 (+%2)"))
          .arg(client.conn.playing->economic.gold)
          .arg(player_get_expected_income(client.conn.playing));
    } else {
      s = QString(_("Gold:%1 (%2)"))
          .arg(client.conn.playing->economic.gold)
          .arg(player_get_expected_income(client.conn.playing));
    }
    m_gold->setText(s);
  }

  s = get_bulb_tooltip();
  s.remove(0, s.indexOf('\n') + 1);
  s.remove(s.indexOf('('), s.count());
  m_research->setText(s);

  if (nullptr != client.conn.playing) {
    research *r = research_get(client_player());
    if (r->researching == A_UNSET && r->tech_goal == A_UNSET
        && r->techs_researched < game.control.num_tech_types
        && !m_researchTimer->isActive()) {
      m_researchTimer->start(700);
      m_blinkState = true;
    }
  }

  resize(sizeHint());
}

void GameInfo::updateTurnTime() {
  QString text = get_timeout_label_text();
  if (text.toLower().trimmed() == "off") {
    m_turnTime->setText("");
  } else {
    m_turnTime->setText(text);
  }
  resize(sizeHint());
}

void GameInfo::blink() {
  if (client_is_observer()) {
    m_research->setStyleSheet(researchInfoStyle);
    m_researchTimer->stop();
    return;
  }

  if (m_blinkState) {
    m_research->setStyleSheet(blinkStyle);
  } else {
    m_research->setStyleSheet(researchInfoStyle);
  }

  if (client.conn.playing != nullptr) {
    research *r = research_get(client_player());
    if (m_researchTimer->isActive() && (r->researching != A_UNSET
                                        || r->tech_goal != A_UNSET)) {
      m_researchTimer->stop();
      m_research->setStyleSheet(researchInfoStyle);
    } else {
      m_researchTimer->start(700);
      m_blinkState = !m_blinkState;
    }
  }
}
