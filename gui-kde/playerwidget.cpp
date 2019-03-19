#include "playerwidget.h"
#include "playerdialog.h"
#include "treatydialog.h"
#include <QVBoxLayout>
#include "logging.h"
#include "sprite.h"
#include <QMap>
#include "tilespec.h"
#include "application.h"
#include <QApplication>

using namespace KV;

PlayerWidget::PlayerWidget(QWidget *parent)
  : QTabWidget(parent)
  , m_players(new PlayerDialog)
{
  setTabBarAutoHide(true);
  setTabsClosable(false);
  setWindowFlags(Qt::Window);
  setWindowTitle(qAppName() + ": Players");

  // players tab
  auto layout = new QVBoxLayout;
  layout->addWidget(m_players);
  auto players = new QWidget;
  players->setLayout(layout);
  addTab(players, "Players");

  connect(m_players, &PlayerDialog::closeRequest, this, &PlayerWidget::hide);

  connect(Application::instance(), &Application::initMeeting,
          this, &PlayerWidget::initMeeting);
  connect(Application::instance(), &Application::cancelMeeting,
          this, &PlayerWidget::cancelMeeting);
  connect(Application::instance(), &Application::createClause,
          this, &PlayerWidget::createClause);
  connect(Application::instance(), &Application::removeClause,
          this, &PlayerWidget::removeClause);
  connect(Application::instance(), &Application::acceptTreaty,
          this, &PlayerWidget::acceptTreaty);
  connect(Application::instance(), &Application::closeAllTreatyDialogs,
          this, &PlayerWidget::closeAllTreatyDialogs);
}

void PlayerWidget::initMeeting(int counterpart) {
  qCDebug(FC) << "initMeeting" << counterpart;
  cancelMeeting(counterpart);

  auto p = player_by_number(counterpart);
  // create a new tab
  auto layout = new QVBoxLayout;
  layout->addWidget(new TreatyDialog(p));
  auto meeting = new QWidget;
  meeting->setLayout(layout);
  QString label = nation_plural_for_player(p);
  auto icon = QIcon(get_nation_flag_sprite(tileset, nation_of_player(p))->pm);
  m_meetings[counterpart] = addTab(meeting, icon, label);
  setCurrentIndex(m_meetings[counterpart]);

  show();
  raise();

}

void PlayerWidget::cancelMeeting(int counterpart) {
  // qCDebug(FC) << "cancelMeeting" << counterpart;

  if (!m_meetings.contains(counterpart)) return;

  qCDebug(FC) << "cancelMeeting: found" << counterpart;
  auto w = widget(m_meetings[counterpart]);
  removeTab(m_meetings[counterpart]);
  delete w;
  m_meetings.remove(counterpart);
  for (int i = 1; i < count(); i++) {
    auto meeting = widget(i)->findChild<TreatyDialog*>();
    m_meetings[meeting->away()] = i;
  }
}


void PlayerWidget::createClause(int counterpart, const Clause& clause) {
  if (!m_meetings.contains(counterpart)) return;
  auto meeting = widget(m_meetings[counterpart])->findChild<TreatyDialog*>();
  meeting->createClause(clause);
}

void PlayerWidget::removeClause(int counterpart, const Clause& clause) {
  if (!m_meetings.contains(counterpart)) return;
  auto meeting = widget(m_meetings[counterpart])->findChild<TreatyDialog*>();
  meeting->removeClause(clause);
}

void PlayerWidget::acceptTreaty(int counterpart, bool resolution) {
  if (!m_meetings.contains(counterpart)) return;
  auto meeting = widget(m_meetings[counterpart])->findChild<TreatyDialog*>();
  meeting->awayResolution(resolution);
}

void PlayerWidget::closeAllTreatyDialogs() {
  for (auto id: m_meetings.keys()) {
    cancelMeeting(id);
  }
}
