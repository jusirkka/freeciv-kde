#include "fc_config.h"

extern "C" {
#include "gotodlg_g.h"
#include "dialogs_g.h"
}

#include "client_main.h"
#include "clinet.h"
#include "control.h"

#include "mainwindow.h"
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
#include "playerdialog.h"
#include "cityview.h"
#include "citydialog.h"
#include <QCloseEvent>
#include "sciencedialog.h"
#include <KStandardAction>
#include <QApplication>
#include <KActionCollection>
#include <QStatusBar>

using namespace KV;

MainWindow::MainWindow()
  : KXmlGuiWindow()
  , m_currentState(nullptr)
  , m_stateList()
  , m_mapView(nullptr)
{
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

  m_players = new PlayerDialog(this);
  connect(Application::instance(), &Application::popupPlayers, this, [=] () {
    m_players->show();
    m_players->raise();
  });


  m_cityReport = new CityView(this);
  connect(Application::instance(), &Application::popupCityReport, this, [=] () {
    m_cityReport->show();
    m_cityReport->raise();
  });

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

  m_scienceReport = new ScienceDialog(this);
  connect(Application::instance(), &Application::popupScienceReport, this, [=] () {
    m_scienceReport->show();
    m_scienceReport->raise();
  });

  addActions();
  createStateMachine();
  readSettings();
}

void MainWindow::createStateMachine() {

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

  intro->addTransition(action("actionConnectToGame"), &QAction::triggered, nw);
  intro->addTransition(action("actionNewGame"), &QAction::triggered, nw);
  intro->addTransition(intro, &State::Intro::playing, game);
  intro->addTransition(this, &MainWindow::resetStateMachine, final);

  nw->addTransition(nw, &State::Network::accepted, start);
  nw->addTransition(nw, &State::Network::rejected, intro);
  nw->addTransition(this, &MainWindow::resetStateMachine, final);

  start->addTransition(start, &State::Start::accepted, game);
  start->addTransition(start, &State::Start::rejected, intro);
  start->addTransition(this, &MainWindow::resetStateMachine, final);

  game->addTransition(action("actionConnectToGame"), &QAction::triggered, nw);
  game->addTransition(action("actionNewGame"), &QAction::triggered, nw);
  game->addTransition(this, &MainWindow::resetStateMachine, final);

  connect(&m_states, &QStateMachine::finished,
          this, &MainWindow::restartStateMachine);

  connect(Application::instance(), &Application::stateChange,
          this, &MainWindow::stateChange);

  m_states.setInitialState(intro);
  m_states.start();
}

void MainWindow::enableGameMenus(bool ok) {
  action("actionSaveGameAs")->setEnabled(ok);
  action("actionFullscreen")->setEnabled(ok);
  action("actionMinimap")->setEnabled(ok);
  action("actionCityOutlines")->setEnabled(ok);
  action("actionCityOutput")->setEnabled(ok);
  action("actionMapGrid")->setEnabled(ok);
  action("actionNationalBorders")->setEnabled(ok);
  action("actionNativeTiles")->setEnabled(ok);
  action("actionCityFullBar")->setEnabled(ok);
  action("actionCityNames")->setEnabled(ok);
  action("actionCityGrowth")->setEnabled(ok);
  action("actionCityProductionLevels")->setEnabled(ok);
  action("actionCityBuyCost")->setEnabled(ok);
  action("actionCityTradeRoutes")->setEnabled(ok);
  action("actionCenterView")->setEnabled(ok);
  action("actionZoomIn")->setEnabled(ok);
  action("actionZoomOut")->setEnabled(ok);
  action("actionScaleFonts")->setEnabled(ok);
  action("actionGotoTile")->setEnabled(ok);
  action("actionGotoNearestCity")->setEnabled(ok);
  action("actionGoAirlifttoCity")->setEnabled(ok);
  action("actionAutoExplore")->setEnabled(ok);
  action("actionPatrol")->setEnabled(ok);
  action("actionSentry")->setEnabled(ok);
  action("actionUnsentryAllOnTile")->setEnabled(ok);
  action("actionLoad")->setEnabled(ok);
  action("actionUnload")->setEnabled(ok);
  action("actionUnloadAllFromTransporter")->setEnabled(ok);
  action("actionSetHomeCity")->setEnabled(ok);
  action("actionUpgrade")->setEnabled(ok);
  action("actionConvert")->setEnabled(ok);
  action("actionDisband")->setEnabled(ok);
  action("actionWait")->setEnabled(ok);
  action("actionDone")->setEnabled(ok);
  action("actionFortifyUnit")->setEnabled(ok);
  action("actionBuildFortFortressBuoy")->setEnabled(ok);
  action("actionBuildAirstripAirbase")->setEnabled(ok);
  action("actionPillage")->setEnabled(ok);
  action("actionBuildCity")->setEnabled(ok);
  action("actionAutoWorker")->setEnabled(ok);
  action("actionBuildRoad")->setEnabled(ok);
  action("actionIrrigate")->setEnabled(ok);
  action("actionMine")->setEnabled(ok);
  action("actionConnectWithRoad")->setEnabled(ok);
  action("actionConnectWithRailway")->setEnabled(ok);
  action("actionConnectWithIrrigation")->setEnabled(ok);
  action("actionTransform")->setEnabled(ok);
  action("actionCleanPollution")->setEnabled(ok);
  action("actionCleanNuclearFallout")->setEnabled(ok);
  action("actionHelpBuildWonder")->setEnabled(ok);
  action("actionEstablishTraderoute")->setEnabled(ok);
  action("actionUnits")->setEnabled(ok);
  action("actionPlayers")->setEnabled(ok);
  action("actionCities")->setEnabled(ok);
  action("actionEconomy")->setEnabled(ok);
  action("actionResearch")->setEnabled(ok);
  action("actionSpaceship")->setEnabled(ok);
  action("actionOptions")->setEnabled(ok);
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
  writeSettings();
}

void MainWindow::closeEvent(QCloseEvent *event) {
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
    event->accept();
  } else {
    event->ignore();
  }
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
  close();
}

void MainWindow::writeSettings() {}

void MainWindow::readSettings() {
  action("actionCityOutlines")->setChecked(gui_options.draw_city_outlines);
  action("actionCityOutput")->setChecked(gui_options.draw_city_output);
  action("actionMapGrid")->setChecked(gui_options.draw_map_grid);
  action("actionNationalBorders")->setChecked(gui_options.draw_borders);
  action("actionNativeTiles")->setChecked(gui_options.draw_native);
  action("actionCityFullBar")->setChecked(gui_options.draw_full_citybar);
  action("actionCityNames")->setChecked(gui_options.draw_city_names);
  action("actionCityGrowth")->setChecked(gui_options.draw_city_growth);
  action("actionCityProductionLevels")->setChecked(gui_options.draw_city_productions);
  action("actionCityBuyCost")->setChecked(gui_options.draw_city_buycost);
  action("actionCityTradeRoutes")->setChecked(gui_options.draw_city_trade_routes);
}

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

void MainWindow::on_actionResearch_triggered() {
  if (m_scienceReport->isVisible()) {
      m_scienceReport->hide();
  } else {
    m_scienceReport->show();
    m_scienceReport->raise();
  }
}

void MainWindow::on_actionSpaceship_triggered() {}
void MainWindow::on_actionOptions_triggered() {}


void MainWindow::addActions() {
  struct Data {
    QString name;
    QString text;
    QString shortcut;
    QString theme;
    QString tooltip;
    bool enabled;
    bool checkable;
    bool checked;
  };

  QVector<Data> actionData{
    {"actionSaveGameAs", "Save Game As...", "Ctrl+Shift+S", "document-save", "Save game as", false, false, false},
    {"actionNewGame", "New Game...", "Ctrl+N", "document-new", "New game", true, false, false},
    {"actionLoadScenario", "Load Scenario...", "", "", "Load scenario game", true, false, false},
    {"actionLoadGame", "Load Game...", "", "", "Load saved game", true, false, false},
    {"actionConnectToGame", "Connect to Game...", "", "", "Connect to game server", true, false, false},
    {"actionQuit", "Quit", "Ctrl+Q", "application-exit", "Quit", true, false, false},
    {"actionFullscreen", "Fullscreen", "Alt+Return", "", "", false, true, false},
    {"actionMinimap", "Minimap", "Ctrl+M", "", "", false, true, true},
    {"actionCityOutlines", "City Outlines", "Ctrl+Y", "", "", false, true, false},
    {"actionCityOutput", "City Output", "Alt+W", "", "", false, true, false},
    {"actionMapGrid", "Map Grid", "Ctrl+G", "", "", false, true, false},
    {"actionNationalBorders", "National Borders", "Ctrl+B", "", "", false, true, false},
    {"actionNativeTiles", "Native Tiles", "Alt+Shift+N", "", "", false, true, false},
    {"actionCityFullBar", "City Full Bar", "Alt+F", "", "City Full Bar", false, true, false},
    {"actionCityNames", "City Names", "Alt+N", "", "", false, true, false},
    {"actionCityGrowth", "City Growth", "Ctrl+R", "", "City Growth", false, true, false},
    {"actionCityProductionLevels", "City Production Levels", "Ctrl+P", "", "City Production Levels", false, true, false},
    {"actionCityBuyCost", "City Buy Cost", "Alt+B", "", "", false, true, false},
    {"actionCityTradeRoutes", "City Traderoutes", "Alt+D", "", "City Traderoutes", false, true, false},
    {"actionCenterView", "Center View", "C", "", "", false, false, false},
    {"actionZoomIn", "Zoom In", "Ctrl++", "zoom-in", "", false, false, false},
    {"actionZoomOut", "Zoom Out", "Ctrl+-", "zoom-out", "", false, false, false},
    {"actionScaleFonts", "Scale Fonts", "", "", "", false, true, false},
    {"actionGotoTile", "Go to Tile", "G", "", "", false, false, false},
    {"actionGotoNearestCity", "Go to Nearest City", "Shift+G", "", "", false, false, false},
    {"actionGoAirlifttoCity", "Go / Airlift to City...", "T", "", "Go / Airlift to City", false, false, false},
    {"actionAutoExplore", "Auto Explore", "X", "", "Auto Explore", false, false, false},
    {"actionPatrol", "Patrol", "Q", "", "", false, false, false},
    {"actionSentry", "Sentry", "S", "", "", false, false, false},
    {"actionUnsentryAllOnTile", "Unsentry all on Tile", "Shift+S", "", "", false, false, false},
    {"actionLoad", "Load", "L", "", "", false, false, false},
    {"actionUnload", "Unload", "U", "", "", false, false, false},
    {"actionUnloadAllFromTransporter", "Unload all from Transporter", "Shift+U", "", "", false, false, false},
    {"actionSetHomeCity", "Set Home City", "H", "", "", false, false, false},
    {"actionUpgrade", "Upgrade", "Shift+U", "", "", false, false, false},
    {"actionConvert", "Convert", "Ctrl+O", "", "", false, false, false},
    {"actionDisband", "Disband", "Shift+D", "", "", false, false, false},
    {"actionWait", "Wait", "W", "", "", false, false, false},
    {"actionDone", "Done", "Space", "", "", false, false, false},
    {"actionFortifyUnit", "Fortify Unit", "F", "", "", false, false, false},
    {"actionBuildFortFortressBuoy", "Build Fort/Fortress/Buoy", "Shift+F", "", "Build Fort/Fortress/Buoy", false, false, false},
    {"actionBuildAirstripAirbase", "Build Airstrip/Airbase", "Shift+E", "", "", false, false, false},
    {"actionPillage", "Pillage...", "Shift+P", "", "", false, false, false},
    {"actionBuildCity", "Build City", "B", "", "", false, false, false},
    {"actionAutoWorker", "Auto Worker", "A", "", "Auto Worker", false, false, false},
    {"actionBuildRoad", "Build Road", "R", "", "", false, false, false},
    {"actionIrrigate", "Irrigate", "I", "", "Irrigate", false, false, false},
    {"actionMine", "Mine", "M", "", "Mine", false, false, false},
    {"actionConnectWithRoad", "Connect with Road", "Shift+R", "", "", false, false, false},
    {"actionConnectWithRailway", "Connect with Railway", "Shift+L", "", "", false, false, false},
    {"actionConnectWithIrrigation", "Connect with Irrigation", "Shift+L", "", "", false, false, false},
    {"actionTransform", "Transform", "O", "", "Transform", false, false, false},
    {"actionCleanPollution", "Clean Pollution", "P", "", "", false, false, false},
    {"actionCleanNuclearFallout", "Clean Nuclear Fallout", "N", "", "", false, false, false},
    {"actionHelpBuildWonder", "Help Build Wonder", "B", "", "", false, false, false},
    {"actionEstablishTraderoute", "Establish Traderoute", "R", "", "", false, false, false},
    {"actionUnits", "Units", "F2", "", "", false, false, false},
    {"actionPlayers", "Players", "F3", "", "", false, false, false},
    {"actionCities", "Cities", "F4", "", "", false, false, false},
    {"actionEconomy", "Economy", "F5", "", "", false, false, false},
    {"actionResearch", "Research", "F6", "", "", false, false, false},
    {"actionSpaceship", "Spaceship", "F12", "", "", false, false, false},
    {"actionOptions", "Options...", "", "configure", "", true, false, false},
  };

  for (auto d: actionData) {
    auto a = new QAction(this);
    actionCollection()->addAction(d.name, a);
    a->setText(d.text);
    a->setCheckable(d.checkable);
    if (d.checkable) {
      a->setChecked(d.checked);
    }
    if (!d.theme.isEmpty()) {
      a->setIcon(QIcon::fromTheme(d.theme));
    }
    if (!d.shortcut.isEmpty()) {
      actionCollection()->setDefaultShortcut(a, d.shortcut);
    }
    if (!d.tooltip.isEmpty()) {
      a->setToolTip(d.tooltip);
    }
    a->setEnabled(d.enabled);
  }

  setupGUI();
  QMetaObject::connectSlotsByName(this);
}
