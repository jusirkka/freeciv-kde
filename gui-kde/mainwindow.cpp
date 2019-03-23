#include "fc_config.h"

extern "C" {
#include "gotodlg_g.h"
#include "dialogs_g.h"
}

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
#include "mapview.h"
#include "minimapview.h"
#include "outputpanemanager.h"
#include "messagepane.h"
#include "chatpane.h"
#include "reportpane.h"
#include "playerwidget.h"
#include "cityview.h"
#include "citydialog.h"

using namespace KV;

MainWindow::MainWindow()
  : QMainWindow()
  , m_ui(new Ui::MainWindow)
  , m_currentState(nullptr)
  , m_stateList()
  , m_mapView(nullptr)
{
  m_ui->setupUi(this);
  setWindowTitle(qAppName());

  auto chatPane = new ChatPane;
  m_chatLine = chatPane->chatLine();
  m_panes = new OutputPaneManager({
                                    new MessagePane,
                                    new Top5CitiesPane,
                                    new WondersPane,
                                    new DemographicsPane,
                                    new AchievementsPane,
                                    chatPane}, this);

  m_players = new PlayerWidget(this);
  connect(Application::instance(), &Application::popupPlayers,
          this, &MainWindow::on_actionPlayers_triggered);


  m_cityReport = new CityView(this);
  connect(Application::instance(), &Application::popupCityReport,
          this, &MainWindow::on_actionCities_triggered);

  m_cityManager = new CityDialog(m_cityReport, this);
  connect(Application::instance(), &Application::refreshCityDialog,
          this, [=] (city* c, bool pop) {
    if (pop) {
      m_cityManager->changeCity(c);
      m_cityManager->show();
      m_cityManager->raise();
    } else if (m_cityManager->isVisible()) {
      m_cityManager->refresh(c);
    }
  });
  connect(Application::instance(), &Application::popdownCityDialog,
          this, [=] (city* c) {
    if (c == nullptr || c == m_cityManager->current()) {
      m_cityManager->hide();
    }
  });
  connect(m_cityReport, &CityView::manageCity, this, [=] (city* c) {
    m_cityManager->changeCity(c);
    m_cityManager->show();
    m_cityManager->raise();
  });



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

  m_ui->actionCityOutlines->setChecked(gui_options.draw_city_outlines);
  m_ui->actionCityOutput->setChecked(gui_options.draw_city_output);
  m_ui->actionMapGrid->setChecked(gui_options.draw_map_grid);
  m_ui->actionNationalBorders->setChecked(gui_options.draw_borders);
  m_ui->actionNativeTiles->setChecked(gui_options.draw_native);
  m_ui->actionCityFullBar->setChecked(gui_options.draw_full_citybar);
  m_ui->actionCityNames->setChecked(gui_options.draw_city_names);
  m_ui->actionCityGrowth->setChecked(gui_options.draw_city_growth);
  m_ui->actionCityProductionLevels->setChecked(gui_options.draw_city_productions);
  m_ui->actionCityBuyCost->setChecked(gui_options.draw_city_buycost);
  m_ui->actionCityTradeRoutes->setChecked(gui_options.draw_city_trade_routes);

  m_ui->actionShowMenubar->setChecked(true);
  m_ui->actionShowToolbar->setChecked(true);
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
  m_ui->actionIrrigate->setEnabled(ok);
  m_ui->actionMine->setEnabled(ok);
  m_ui->actionConnectWithRoad->setEnabled(ok);
  m_ui->actionConnectWithRailway->setEnabled(ok);
  m_ui->actionConnectWithIrrigation->setEnabled(ok);
  m_ui->actionTransform->setEnabled(ok);
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
  m_ui->actionOptions->setEnabled(ok);
}

void MainWindow::setMapView(MapView *map) {
  m_mapView = map;
  setCentralWidget(map);
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

void MainWindow::resizeEvent(QResizeEvent *event) {
  auto p = statusBar()->pos();
  m_panes->move(p.x(), p.y() - m_panes->height());
  QMainWindow::resizeEvent(event);
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

void MainWindow::on_actionFullscreen_toggled(bool on) {
  if (on) {
    showFullScreen();
  } else {
    showNormal();
  }
}

void MainWindow::on_actionMinimap_toggled(bool on) {
  auto minis = findChildren<MinimapView*>();
  for (auto mini: minis) {
    mini->setVisible(on);
  }
}

void MainWindow::on_actionCityOutlines_toggled(bool on) {
  if (gui_options.draw_city_outlines != on) {
    key_city_outlines_toggle();
  }
}

void MainWindow::on_actionCityOutput_toggled(bool on) {
  if (gui_options.draw_city_output != on) {
    key_city_output_toggle();
  }
}

void MainWindow::on_actionMapGrid_toggled(bool on) {
  if (gui_options.draw_map_grid != on) {
    key_map_grid_toggle();
  }
}

void MainWindow::on_actionNationalBorders_toggled(bool on) {
  if (gui_options.draw_borders != on) {
    key_map_borders_toggle();
  }
}

void MainWindow::on_actionNativeTiles_toggled(bool on) {
  if (gui_options.draw_native != on) {
    key_map_native_toggle();
  }
}

void MainWindow::on_actionCityFullBar_toggled(bool on) {
  if (gui_options.draw_full_citybar != on) {
    key_city_full_bar_toggle();
  }
}


void MainWindow::on_actionCityNames_toggled(bool on) {
  if (gui_options.draw_city_names != on) {
    key_city_names_toggle();
  }
}

void MainWindow::on_actionCityGrowth_toggled(bool on) {
  if (gui_options.draw_city_growth != on) {
    key_city_growth_toggle();
  }
}

void MainWindow::on_actionCityProductionLevels_toggled(bool on) {
  if (gui_options.draw_city_productions != on) {
    key_city_productions_toggle();
  }
}

void MainWindow::on_actionCityBuyCost_toggled(bool on) {
  if (gui_options.draw_city_buycost != on) {
    key_city_buycost_toggle();
  }
}

void MainWindow::on_actionCityTradeRoutes_toggled(bool on) {
  if (gui_options.draw_city_trade_routes != on) {
    key_city_trade_routes_toggle();
  }
}

void MainWindow::on_actionCenterView_triggered() {
  request_center_focus_unit();
}

void MainWindow::on_actionZoomIn_triggered() {
  m_mapView->zoomIn();
  tilespec_reread(tileset_basename(tileset), true, m_mapView->zoomLevel());
}

void MainWindow::on_actionZoomOut_triggered() {
  m_mapView->zoomOut();
  tilespec_reread(tileset_basename(tileset), true, m_mapView->zoomLevel());
}


void MainWindow::on_actionScaleFonts_toggled(bool on) {}

void MainWindow::on_actionGotoTile_triggered() {
  key_unit_goto();
}

void MainWindow::on_actionGotoNearestCity_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    request_unit_return(punit);
  } unit_list_iterate_end;
}

void MainWindow::on_actionGoAirlifttoCity_triggered() {
  popup_goto_dialog();
}

void MainWindow::on_actionAutoExplore_triggered() {
  key_unit_auto_explore();
}

void MainWindow::on_actionPatrol_triggered() {
  key_unit_patrol();
}

void MainWindow::on_actionSentry_triggered() {
  key_unit_sentry();
}

void MainWindow::on_actionUnsentryAllOnTile_triggered() {
  key_unit_wakeup_others();
}

void MainWindow::on_actionLoad_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    request_transport(punit, unit_tile(punit));
  } unit_list_iterate_end;
}

void MainWindow::on_actionUnload_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    request_unit_unload(punit);
  } unit_list_iterate_end;
}

void MainWindow::on_actionUnloadAllFromTransporter_triggered() {
  key_unit_unload_all();
}

void MainWindow::on_actionSetHomeCity_triggered() {
  key_unit_homecity();
}

void MainWindow::on_actionUpgrade_triggered() {
  popup_upgrade_dialog(get_units_in_focus());
}

void MainWindow::on_actionConvert_triggered() {
  key_unit_convert();
}

void MainWindow::on_actionDisband_triggered() {
  popup_disband_dialog(get_units_in_focus());
}

void MainWindow::on_actionWait_triggered() {
  key_unit_wait();
}

void MainWindow::on_actionDone_triggered() {
  key_unit_done();
}

void MainWindow::on_actionFortifyUnit_triggered() {
  key_unit_fortify();
}

void MainWindow::on_actionBuildFortFortressBuoy_triggered() {
  key_unit_fortress();
}

void MainWindow::on_actionBuildAirstripAirbase_triggered() {
  key_unit_airbase();
}

void MainWindow::on_actionPillage_triggered() {
  key_unit_pillage();
}

void MainWindow::on_actionBuildCity_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    if (unit_can_add_or_build_city(punit)) {
      request_unit_build_city(punit);
    }
  } unit_list_iterate_end;
}

void MainWindow::on_actionAutoWorker_triggered() {
  key_unit_auto_settle();
}

void MainWindow::on_actionBuildRoad_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    auto t = next_extra_for_tile(unit_tile(punit),
                                 EC_ROAD,
                                 unit_owner(punit),
                                 punit);
    if (t != nullptr && can_unit_do_activity_targeted(punit, ACTIVITY_GEN_ROAD, t)) {
      request_new_unit_activity_targeted(punit, ACTIVITY_GEN_ROAD, t);
    }
  } unit_list_iterate_end;
}

void MainWindow::on_actionIrrigate_triggered() {
  key_unit_irrigate();
}

void MainWindow::on_actionMine_triggered() {
  key_unit_mine();
}

void MainWindow::on_actionConnectWithRoad_triggered() {
  auto proad = road_by_compat_special(ROCO_ROAD);
  if (proad != nullptr) {
    key_unit_connect(ACTIVITY_GEN_ROAD, road_extra_get(proad));
  }
}

void MainWindow::on_actionConnectWithRailway_triggered() {
  auto proad = road_by_compat_special(ROCO_RAILROAD);
  if (proad != nullptr) {
    key_unit_connect(ACTIVITY_GEN_ROAD, road_extra_get(proad));
  }
}

void MainWindow::on_actionConnectWithIrrigation_triggered() {
  auto extras = extra_type_list_by_cause(EC_IRRIGATION);
  if (extra_type_list_size(extras) > 0) {
    auto pextra = extra_type_list_get(extra_type_list_by_cause(EC_IRRIGATION), 0);
    key_unit_connect(ACTIVITY_IRRIGATE, pextra);
  }
}

void MainWindow::on_actionTransform_triggered() {
  key_unit_transform();
}

void MainWindow::on_actionCleanPollution_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    auto pextra = prev_extra_in_tile(unit_tile(punit), ERM_CLEANPOLLUTION,
                                     unit_owner(punit), punit);
    if (pextra != nullptr) {
      request_new_unit_activity_targeted(punit, ACTIVITY_POLLUTION, pextra);
    }
  } unit_list_iterate_end;
}

void MainWindow::on_actionCleanNuclearFallout_triggered() {
  key_unit_fallout();
}

void MainWindow::on_actionHelpBuildWonder_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    if (utype_can_do_action(unit_type_get(punit), ACTION_HELP_WONDER)) {
      request_unit_caravan_action(punit, ACTION_HELP_WONDER);
    }
  } unit_list_iterate_end;
}


void MainWindow::on_actionEstablishTraderoute_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    if (unit_can_est_trade_route_here(punit)) {
      request_unit_caravan_action(punit, ACTION_TRADE_ROUTE);
    }
  } unit_list_iterate_end;
}

void MainWindow::on_actionUnits_triggered() {}

void MainWindow::on_actionPlayers_triggered() {
  if (m_players->isVisible()) {
      m_players->hide();
  } else {
    m_players->show();
    m_players->raise();
  }
}

void MainWindow::on_actionCities_triggered() {
  if (m_cityReport->isVisible()) {
      m_cityReport->hide();
  } else {
    m_cityReport->show();
    m_cityReport->raise();
  }
}

void MainWindow::on_actionEconomy_triggered() {}
void MainWindow::on_actionResearch_triggered() {}
void MainWindow::on_actionSpaceship_triggered() {}
void MainWindow::on_actionAbout_triggered() {}
void MainWindow::on_actionHandbook_triggered() {}
void MainWindow::on_actionConfigureShortcuts_triggered() {}
void MainWindow::on_actionConfigureToolbar_triggered() {}
void MainWindow::on_actionOptions_triggered() {}

void MainWindow::on_actionShowMenubar_toggled(bool on) {
  menuBar()->setVisible(on);
}

void MainWindow::on_actionShowToolbar_toggled(bool on) {
  m_ui->toolBar->setVisible(on);
}
