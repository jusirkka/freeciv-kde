#include "playerdialog.h"
#include "playerwidget.h"
#include "treatydialog.h"
#include <QVBoxLayout>
#include "logging.h"
#include "sprite.h"
#include <QMap>
#include <QTableWidget>
#include "tilespec.h"
#include "application.h"
#include <QApplication>
#include <KConfigGroup>
#include <KWindowConfig>
#include <KSharedConfig>
#include <QWindow>

#include "client_main.h"

using namespace KV;

PlayerDialog::PlayerDialog(QWidget *parent)
  : QDialog(parent)
  , m_players(new PlayerWidget)
  , m_tabs(new QTabWidget)
{
  auto topLayout = new QVBoxLayout;
  topLayout->addWidget(m_tabs);
  setLayout(topLayout);

  setWindowTitle(qAppName() + ": Players");
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
  setWindowFlag(Qt::Dialog, false);
  setWindowFlag(Qt::Window, true);

  m_tabs->setTabBarAutoHide(true);
  m_tabs->setTabsClosable(false);
  // players tab
  auto layout = new QVBoxLayout;
  layout->addWidget(m_players);
  auto players = new QWidget;
  players->setLayout(layout);
  m_tabs->addTab(players, "Players");

  connect(m_players, &PlayerWidget::closeRequest, this, &PlayerDialog::hide);

  connect(Application::instance(), &Application::initMeeting,
          this, &PlayerDialog::initMeeting);
  connect(Application::instance(), &Application::cancelMeeting,
          this, &PlayerDialog::cancelMeeting);
  connect(Application::instance(), &Application::createClause,
          this, &PlayerDialog::createClause);
  connect(Application::instance(), &Application::removeClause,
          this, &PlayerDialog::removeClause);
  connect(Application::instance(), &Application::acceptTreaty,
          this, &PlayerDialog::acceptTreaty);
  connect(Application::instance(), &Application::closeAllTreatyDialogs,
          this, &PlayerDialog::closeAllTreatyDialogs);

  setMinimumWidth(800);
  setMinimumHeight(450);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "PlayerDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());
}

PlayerDialog::~PlayerDialog() {
  KConfigGroup cnf(KSharedConfig::openConfig(), "PlayerDialog");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
}

void PlayerDialog::initMeeting(int counterpart, int initiator) {
  // qCDebug(FC) << "initMeeting" << counterpart;
  cancelMeeting(counterpart, InvalidPlayer);

  auto p = player_by_number(counterpart);
  // create a new tab
  auto layout = new QVBoxLayout;
  layout->addWidget(new TreatyDialog(p));
  auto meeting = new QWidget;
  meeting->setLayout(layout);
  QString label = nation_plural_for_player(p);
  auto icon = QIcon(get_nation_flag_sprite(get_tileset(), nation_of_player(p))->pm);
  m_meetings[counterpart].index = m_tabs->addTab(meeting, icon, label);
  m_meetings[counterpart].initiator = initiator;
  m_tabs->setCurrentIndex(m_meetings[counterpart].index);

  show();
  raise();

}

void PlayerDialog::cancelMeeting(int counterpart, int canceler) {
  // qCDebug(FC) << "cancelMeeting" << counterpart;

  if (!m_meetings.contains(counterpart)) return;

  // qCDebug(FC) << "cancelMeeting: found" << counterpart;
  int initiator = m_meetings[counterpart].initiator;

  auto w = m_tabs->widget(m_meetings[counterpart].index);
  m_tabs->removeTab(m_meetings[counterpart].index);
  delete w;
  m_meetings.remove(counterpart);
  for (int i = 1; i < m_tabs->count(); i++) {
    auto meeting = m_tabs->widget(i)->findChild<TreatyDialog*>();
    m_meetings[meeting->away()].index = i;
  }

  if (!m_meetings.isEmpty() || canceler == InvalidPlayer) return;

  // hide player dialog if it was popped up by other player's meeting request
  int me = player_number(client_player());
  if (canceler == me && initiator != me) {
    hide();
  }
}


void PlayerDialog::createClause(int counterpart, const Clause& clause) {
  if (!m_meetings.contains(counterpart)) return;
  auto meeting = m_tabs->widget(m_meetings[counterpart].index)->findChild<TreatyDialog*>();
  meeting->createClause(clause);
}

void PlayerDialog::removeClause(int counterpart, const Clause& clause) {
  if (!m_meetings.contains(counterpart)) return;
  auto meeting = m_tabs->widget(m_meetings[counterpart].index)->findChild<TreatyDialog*>();
  meeting->removeClause(clause);
}

void PlayerDialog::acceptTreaty(int counterpart, bool resolution) {
  if (!m_meetings.contains(counterpart)) return;
  auto meeting = m_tabs->widget(m_meetings[counterpart].index)->findChild<TreatyDialog*>();
  meeting->awayResolution(resolution);
}

void PlayerDialog::closeAllTreatyDialogs() {
  for (auto id: m_meetings.keys()) {
    cancelMeeting(id, InvalidPlayer);
  }
}
