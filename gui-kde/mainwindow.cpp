#include "fc_config.h"

#include "client_main.h"
#include "clinet.h"
#include "control.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "messagebox.h"
#include "state.h"
#include "logging.h"
#include "application.h"
#include "chatlineedit.h"
#include <QFinalState>

using namespace KV;

MainWindow::MainWindow()
  : QMainWindow()
  , m_ui(new Ui::MainWindow)
  , m_currentState(nullptr)
  , m_stateList()
  // , m_chatLine(new ChatLineEdit(this))
{
  m_ui->setupUi(this);
  setWindowTitle(qAppName());

  auto intro = new State::Intro(this);
  connect(intro, &QState::activeChanged, this, &MainWindow::setCurrentState);
  m_states.addState(intro);
  m_stateList << intro;

  auto nw = new State::Network(this);
  connect(nw, &QState::activeChanged, this, &MainWindow::setCurrentState);
  m_states.addState(nw);
  m_stateList << nw;

  auto start = new State::Start(this);
  connect(start, &QState::activeChanged, this, &MainWindow::setCurrentState);
  m_states.addState(start);
  m_stateList << start;

  auto game = new State::Game(this);
  connect(game, &QState::activeChanged, this, &MainWindow::setCurrentState);
  m_states.addState(game);
  m_stateList << game;

  auto final = new QFinalState;
  m_states.addState(final);

  intro->addTransition(m_ui->actionConnectToGame, &QAction::triggered, nw);
  intro->addTransition(m_ui->actionNewGame, &QAction::triggered, nw);
  intro->addTransition(intro, &State::Intro::playing, game);
  intro->addTransition(this, &MainWindow::resetStateMachine, final);

  nw->addTransition(nw, &State::Network::accepted, start);
  nw->addTransition(nw, &State::Network::rejected, intro);
  nw->addTransition(this, &MainWindow::resetStateMachine, final);

  start->addTransition(start, &State::Start::accepted, game);
  start->addTransition(start, &State::Start::rejected, intro);
  start->addTransition(this, &MainWindow::resetStateMachine, final);

  game->addTransition(m_ui->actionConnectToGame, &QAction::triggered, nw);
  game->addTransition(m_ui->actionNewGame, &QAction::triggered, nw);
  game->addTransition(this, &MainWindow::resetStateMachine, final);

  connect(&m_states, &QStateMachine::finished,
          this, &MainWindow::restartStateMachine);

  connect(Application::instance(), &Application::stateChange,
          this, &MainWindow::stateChange);

  m_states.setInitialState(intro);
  m_states.start();

}

void MainWindow::enableGameMenus(bool ok) {
  m_ui->actionSaveGameAs->setEnabled(ok);
  m_ui->actionFullscreen->setEnabled(ok);
  m_ui->actionMinimap->setEnabled(ok);
  m_ui->actionCityOutlines->setEnabled(ok);
  m_ui->actionCityOutput->setEnabled(ok);
  m_ui->actionMapGrid->setEnabled(ok);
  m_ui->actionNationalBorders->setEnabled(ok);
  m_ui->actionNativeTiles->setEnabled(ok);
  m_ui->actionCityFullBar->setEnabled(ok);
  m_ui->actionCityNames->setEnabled(ok);
  m_ui->actionCityGrowth->setEnabled(ok);
  m_ui->actionCityProductionLevels->setEnabled(ok);
  m_ui->actionCityBuyCost->setEnabled(ok);
  m_ui->actionCityTradeRoutes->setEnabled(ok);
  m_ui->actionCenterView->setEnabled(ok);
  m_ui->actionZoomIn->setEnabled(ok);
  m_ui->actionZoomOut->setEnabled(ok);
  m_ui->actionScaleFonts->setEnabled(ok);
  m_ui->actionGotoTile->setEnabled(ok);
  m_ui->actionGotoNearestCity->setEnabled(ok);
  m_ui->actionGoAirlifttoCity->setEnabled(ok);
  m_ui->actionAutoExplore->setEnabled(ok);
  m_ui->actionPatrol->setEnabled(ok);
  m_ui->actionSentry->setEnabled(ok);
  m_ui->actionUnsentryAllOnTile->setEnabled(ok);
  m_ui->actionLoad->setEnabled(ok);
  m_ui->actionUnload->setEnabled(ok);
  m_ui->actionUnloadAllFromTransporter->setEnabled(ok);
  m_ui->actionSetHomeCity->setEnabled(ok);
  m_ui->actionUpgrade->setEnabled(ok);
  m_ui->actionConvert->setEnabled(ok);
  m_ui->actionDisband->setEnabled(ok);
  m_ui->actionWait->setEnabled(ok);
  m_ui->actionDone->setEnabled(ok);
  m_ui->actionFortifyUnit->setEnabled(ok);
  m_ui->actionBuildFortFortressBuoy->setEnabled(ok);
  m_ui->actionBuildAirstripAirbase->setEnabled(ok);
  m_ui->actionPillage->setEnabled(ok);
  m_ui->actionBuildCity->setEnabled(ok);
  m_ui->actionAutoWorker->setEnabled(ok);
  m_ui->actionBuildRoad->setEnabled(ok);
  m_ui->actionTransformToPlains->setEnabled(ok);
  m_ui->actionTransformToGrassland->setEnabled(ok);
  m_ui->actionConnectWithRoad->setEnabled(ok);
  m_ui->actionConnectWithRailway->setEnabled(ok);
  m_ui->actionConnectWithIrrigation->setEnabled(ok);
  m_ui->actionTransformTerrain->setEnabled(ok);
  m_ui->actionCleanPollution->setEnabled(ok);
  m_ui->actionCleanNuclearFallout->setEnabled(ok);
  m_ui->actionHelpBuildWonder->setEnabled(ok);
  m_ui->actionEstablishTraderoute->setEnabled(ok);
  m_ui->actionUnits->setEnabled(ok);
  m_ui->actionPlayers->setEnabled(ok);
  m_ui->actionCities->setEnabled(ok);
  m_ui->actionEconomy->setEnabled(ok);
  m_ui->actionResearch->setEnabled(ok);
  m_ui->actionSpaceship->setEnabled(ok);
  m_ui->actionAchievements->setEnabled(ok);
  m_ui->actionOptions->setEnabled(ok);
}

void MainWindow::setCurrentState(bool active) {
  auto s = qobject_cast<State::Base*>(sender());
  // qCDebug(FC) << "MainWindow::setCurrentState" << s->id() << active;
  if (!active) {
    if (s == m_currentState) {
      // qCDebug(FC) << "no active state";
      m_currentState = nullptr;
    }
  } else {
    qCDebug(FC) << "active state = " << client_pages_name(s->id());
    m_currentState = s;
  }
}

client_pages MainWindow::state() const {return m_currentState->id();}

void MainWindow::stateChange(client_pages page) {
  if (state() == page) return;
  State::Base* to = nullptr;
  for (auto s: m_stateList) {
    if (s->id() == page) {
      to = s;
      break;
    }
  }
  if (to) {
    qCDebug(FC) << "MainWindow::stateChange" << client_pages_name(page);
    m_states.setInitialState(to);
    emit resetStateMachine();
  }
}


void MainWindow::restartStateMachine() {
  m_states.start();
}

MainWindow::~MainWindow() {
  delete m_ui;
}

void MainWindow::closeEvent(QCloseEvent */*event*/) {
}

void MainWindow::on_actionSaveGameAs_triggered() {}
void MainWindow::on_actionLoadScenario_triggered() {}
void MainWindow::on_actionLoadGame_triggered() {}

void MainWindow::on_actionQuit_triggered() {
  bool ok = true;
  if (!m_currentState || m_currentState->id() != PAGE_MAIN) {
    KV::StandardMessageBox ask(centralWidget(),
                               _("Are you sure you want to quit?"),
                               _("Quit?"));
    ok = ask.exec() == QMessageBox::Ok;
  }
  if (ok) {
    start_quitting();
    if (client.conn.used) {
      disconnect_from_server();
    }
    writeSettings();
    qApp->quit();
  }
}

void MainWindow::writeSettings() {}
void MainWindow::readSettings() {}

void MainWindow::on_actionFullscreen_toggled(bool on) {}
void MainWindow::on_actionMinimap_toggled(bool on) {}
void MainWindow::on_actionCityOutlines_toggled(bool on) {}
void MainWindow::on_actionCityOutput_toggled(bool on) {}
void MainWindow::on_actionMapGrid_toggled(bool on) {}
void MainWindow::on_actionNationalBorders_toggled(bool on) {}
void MainWindow::on_actionNativeTiles_toggled(bool on) {}
void MainWindow::on_actionCityFullBar_toggled(bool on) {}
void MainWindow::on_actionCityNames_toggled(bool on) {}
void MainWindow::on_actionCityGrowth_toggled(bool on) {}
void MainWindow::on_actionCityProductionLevels_toggled(bool on) {}
void MainWindow::on_actionCityBuyCost_toggled(bool on) {}
void MainWindow::on_actionCityTradeRoutes_toggled(bool on) {}
void MainWindow::on_actionCenterView_triggered() {}
void MainWindow::on_actionZoomIn_triggered() {}
void MainWindow::on_actionZoomOut_triggered() {}
void MainWindow::on_actionScaleFonts_toggled(bool on) {}

void MainWindow::on_actionGotoTile_triggered() {
  key_unit_goto();
}

void MainWindow::on_actionGotoNearestCity_triggered() {}
void MainWindow::on_actionGoAirlifttoCity_triggered() {}
void MainWindow::on_actionAutoExplore_triggered() {}
void MainWindow::on_actionPatrol_triggered() {}
void MainWindow::on_actionSentry_triggered() {}
void MainWindow::on_actionUnsentryAllOnTile_triggered() {}
void MainWindow::on_actionLoad_triggered() {}
void MainWindow::on_actionUnload_triggered() {}
void MainWindow::on_actionUnloadAllFromTransporter_triggered() {}
void MainWindow::on_actionSetHomeCity_triggered() {}
void MainWindow::on_actionUpgrade_triggered() {}
void MainWindow::on_actionConvert_triggered() {}
void MainWindow::on_actionDisband_triggered() {}
void MainWindow::on_actionWait_triggered() {}
void MainWindow::on_actionDone_triggered() {}
void MainWindow::on_actionFortifyUnit_triggered() {}
void MainWindow::on_actionBuildFortFortressBuoy_triggered() {}
void MainWindow::on_actionBuildAirstripAirbase_triggered() {}
void MainWindow::on_actionPillage_triggered() {}
void MainWindow::on_actionBuildCity_triggered() {}
void MainWindow::on_actionAutoWorker_triggered() {}
void MainWindow::on_actionBuildRoad_triggered() {}
void MainWindow::on_actionTransformToPlains_triggered() {}
void MainWindow::on_actionTransformToGrassland_triggered() {}
void MainWindow::on_actionConnectWithRoad_triggered() {}
void MainWindow::on_actionConnectWithRailway_triggered() {}
void MainWindow::on_actionConnectWithIrrigation_triggered() {}
void MainWindow::on_actionTransformTerrain_triggered() {}
void MainWindow::on_actionCleanPollution_triggered() {}
void MainWindow::on_actionCleanNuclearFallout_triggered() {}
void MainWindow::on_actionHelpBuildWonder_triggered() {}
void MainWindow::on_actionEstablishTraderoute_triggered() {}
void MainWindow::on_actionUnits_triggered() {}
void MainWindow::on_actionPlayers_triggered() {}
void MainWindow::on_actionCities_triggered() {}
void MainWindow::on_actionEconomy_triggered() {}
void MainWindow::on_actionResearch_triggered() {}
void MainWindow::on_actionSpaceship_triggered() {}
void MainWindow::on_actionAchievements_triggered() {}
void MainWindow::on_actionAbout_triggered() {}
void MainWindow::on_actionHandbook_triggered() {}
void MainWindow::on_actionConfigureShortcuts_triggered() {}
void MainWindow::on_actionConfigureToolbar_triggered() {}
void MainWindow::on_actionOptions_triggered() {}
void MainWindow::on_actionShowMenubar_toggled(bool on) {}
void MainWindow::on_actionShowToolbar_toggled(bool on) {}
