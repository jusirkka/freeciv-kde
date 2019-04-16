#include "fc_config.h"

extern "C" {
#include "gotodlg_g.h"
#include "dialogs_g.h"
}

#include "client_main.h"
#include "clinet.h"
#include "control.h"
#include "options.h"
#include "mapview_common.h"
#include "mapctrl_common.h"

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
#include "conf_mainwindow.h"
#include "unitactionchecker.h"
#include "unitreport.h"
#include "economyreport.h"
#include <KHelpMenu>
#include <KAboutData>
#include "helpdialog.h"
#include "localoptionsdialog.h"

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
  });

  m_scienceReport = new ScienceDialog(this);
  connect(Application::instance(), &Application::popupScienceReport, this, [=] () {
    m_scienceReport->show();
  });

  m_unitReport = new UnitReport(this);
  connect(Application::instance(), &Application::popupUnitReport, this, [=] () {
    m_unitReport->showAsTable();
  });

  m_economyReport = new EconomyReport(this);
  connect(Application::instance(), &Application::popupEconomyReport, this, [=] () {
    m_economyReport->showAsTable();
  });

  m_help = new HelpDialog(this);
  connect(Application::instance(), &Application::popdownHelpDialog, this, [=] () {
    m_help->hide();
  });

  m_localOptions = new LocalOptionsDialog(this);

  connect(Application::instance(), &Application::updateActions,
          this, &MainWindow::checkActions);

  connect(Application::instance(), &Application::updateOption,
          this, &MainWindow::updateOption);

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

  intro->addTransition(action("connectToGame"), &QAction::triggered, nw);
  intro->addTransition(action("newGame"), &QAction::triggered, nw);
  intro->addTransition(intro, &State::Intro::playing, game);
  intro->addTransition(this, &MainWindow::resetStateMachine, final);

  nw->addTransition(nw, &State::Network::accepted, start);
  nw->addTransition(nw, &State::Network::rejected, intro);
  nw->addTransition(this, &MainWindow::resetStateMachine, final);

  start->addTransition(start, &State::Start::accepted, game);
  start->addTransition(start, &State::Start::rejected, intro);
  start->addTransition(this, &MainWindow::resetStateMachine, final);

  game->addTransition(action("connectToGame"), &QAction::triggered, nw);
  game->addTransition(action("newGame"), &QAction::triggered, nw);
  game->addTransition(this, &MainWindow::resetStateMachine, final);

  connect(&m_states, &QStateMachine::finished,
          this, &MainWindow::restartStateMachine);

  connect(Application::instance(), &Application::stateChange,
          this, &MainWindow::stateChange);

  m_states.setInitialState(intro);
  m_states.start();
}

void MainWindow::enableGameMenus(bool ok) {
  for (auto name: m_staticGameActions) {
    actionCollection()->action(name)->setEnabled(ok);
  }
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
  qDeleteAll(m_checkers);
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

void MainWindow::on_saveGameAs_triggered() {}
void MainWindow::on_loadScenario_triggered() {}
void MainWindow::on_loadGame_triggered() {}

void MainWindow::on_quit_triggered() {
  close();
}

void MainWindow::writeSettings() {
  Conf::MainWindow::setMinimap(action("minimap")->isChecked());
  Conf::MainWindow::setScaleFonts(action("scaleFonts")->isChecked());
  Conf::MainWindow::setFullScreen(action("fullScreen")->isChecked());
  if (m_fallbackGeom.isValid()) {
    Conf::MainWindow::setLastGeom(m_fallbackGeom);
  }
  Conf::MainWindow::self()->save();
}

void MainWindow::readSettings() {
  action("minimap")->setChecked(Conf::MainWindow::minimap());
  action("scaleFonts")->setChecked(Conf::MainWindow::scaleFonts());
  m_fallbackGeom = QRect();
  if (Conf::MainWindow::fullScreen()) {
    m_fallbackGeom = Conf::MainWindow::lastGeom();
    // go fullscreen only when game commences
    if (m_fallbackGeom.isValid()) {
      setGeometry(m_fallbackGeom);
    }
  }
}

void MainWindow::on_fullScreen_toggled(bool on) {
  if (on) {
    m_fallbackGeom = geometry();
    showFullScreen();
  } else {
    showNormal();
  }
}

void MainWindow::on_minimap_toggled(bool on) {
  auto minis = findChildren<MinimapView*>();
  for (auto mini: minis) {
    mini->setVisible(on);
  }
}

void MainWindow::on_cityOutlines_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_city_outlines");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_cityOutput_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_city_output");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_mapGrid_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_map_grid");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_nationalBorders_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_borders");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_nativeTiles_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_native");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_cityFullBar_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_full_citybar");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}


void MainWindow::on_cityNames_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_city_names");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_cityGrowth_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_city_growth");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_cityProductionLevels_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_city_productions");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_cityBuyCost_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_city_buycost");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_cityTradeRoutes_toggled(bool on) {
  auto opt = optset_option_by_name(client_optset, "draw_city_trade_routes");
  bool v = option_bool_get(opt);
  if (v != on) {
    option_bool_set(opt, on);
  }
}

void MainWindow::on_centerView_triggered() {
  request_center_focus_unit();
}

void MainWindow::on_zoomIn_triggered() {
  m_mapView->zoomIn();
  tilespec_reread(tileset_basename(get_tileset()), true, m_mapView->zoomLevel());
}

void MainWindow::on_zoomOut_triggered() {
  m_mapView->zoomOut();
  tilespec_reread(tileset_basename(get_tileset()), true, m_mapView->zoomLevel());
}


void MainWindow::on_scaleFonts_toggled(bool on) {}


void MainWindow::on_moveNorth_triggered() {
  key_unit_move(DIR8_NORTH);
}

void MainWindow::on_moveEast_triggered() {
  key_unit_move(DIR8_EAST);
}

void MainWindow::on_moveSouth_triggered() {
  key_unit_move(DIR8_SOUTH);
}

void MainWindow::on_moveWest_triggered() {
  key_unit_move(DIR8_WEST);
}

void MainWindow::on_moveNortheast_triggered() {
  key_unit_move(DIR8_NORTHEAST);
}

void MainWindow::on_moveSoutheast_triggered() {
  key_unit_move(DIR8_SOUTHEAST);
}

void MainWindow::on_moveSouthwest_triggered() {
  key_unit_move(DIR8_SOUTHWEST);
}

void MainWindow::on_moveNorthwest_triggered() {
  key_unit_move(DIR8_NORTHWEST);
}

void MainWindow::on_panNorth_triggered() {
  auto s = m_mapView->size();
  recenter_button_pressed(s.width() / 2, 0);
}

void MainWindow::on_panEast_triggered() {
  auto s = m_mapView->size();
  recenter_button_pressed(s.width(), s.height() / 2);
}

void MainWindow::on_panSouth_triggered() {
  auto s = m_mapView->size();
  recenter_button_pressed(s.width() / 2, s.height());
}

void MainWindow::on_panWest_triggered() {
  auto s = m_mapView->size();
  recenter_button_pressed(0, s.height() / 2);
}

void MainWindow::on_previousFocus_triggered() {
  key_recall_previous_focus_unit();
}

void MainWindow::on_cancel_triggered() {
  if (m_panes->isVisible()) {
    m_panes->hidePane();
  } else {
    key_cancel_action();
  }
}

void MainWindow::on_endTurn_triggered() {
  key_end_turn();
}

void MainWindow::on_showUnits_triggered() {
  auto pos = m_mapView->mapFromGlobal(QCursor::pos());
  auto t = canvas_pos_to_tile(pos.x(), pos.y());
  if (t != nullptr && unit_list_size(t->units) > 0) {
    m_mapView->popupUnitSelector(t);
  }
}

void MainWindow::on_combatInfo_triggered() {
  qCDebug(FC) << "TODO: on_combatInfo_triggered()";
}

void MainWindow::on_gotoTile_triggered() {
  key_unit_goto();
}

void MainWindow::on_gotoNearestCity_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    request_unit_return(punit);
  } unit_list_iterate_end;
}

void MainWindow::on_goAirlifttoCity_triggered() {
  popup_goto_dialog();
}

void MainWindow::on_autoExplore_triggered() {
  key_unit_auto_explore();
}

void MainWindow::on_patrol_triggered() {
  key_unit_patrol();
}

void MainWindow::on_sentry_triggered() {
  key_unit_sentry();
}

void MainWindow::on_unsentryAllOnTile_triggered() {
  key_unit_wakeup_others();
}

void MainWindow::on_load_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    request_transport(punit, unit_tile(punit));
  } unit_list_iterate_end;
}

void MainWindow::on_unload_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    request_unit_unload(punit);
  } unit_list_iterate_end;
}

void MainWindow::on_unloadAllFromTransporter_triggered() {
  key_unit_unload_all();
}

void MainWindow::on_setHomeCity_triggered() {
  key_unit_homecity();
}

void MainWindow::on_upgrade_triggered() {
  popup_upgrade_dialog(get_units_in_focus());
}

void MainWindow::on_convert_triggered() {
  key_unit_convert();
}

void MainWindow::on_disband_triggered() {
  popup_disband_dialog(get_units_in_focus());
}

void MainWindow::on_wait_triggered() {
  key_unit_wait();
}

void MainWindow::on_done_triggered() {
  key_unit_done();
}

void MainWindow::on_fortifyUnit_triggered() {
  key_unit_fortify();
}

void MainWindow::on_buildFortFortressBuoy_triggered() {
  key_unit_fortress();
}

void MainWindow::on_buildAirstripAirbase_triggered() {
  key_unit_airbase();
}

void MainWindow::on_pillage_triggered() {
  key_unit_pillage();
}

void MainWindow::on_buildCity_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    if (unit_can_add_or_build_city(punit)) {
      request_unit_build_city(punit);
    }
  } unit_list_iterate_end;
}

void MainWindow::on_autoWorker_triggered() {
  key_unit_auto_settle();
}

void MainWindow::on_buildRoad_triggered() {
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

void MainWindow::on_irrigate_triggered() {
  key_unit_irrigate();
}

void MainWindow::on_mine_triggered() {
  key_unit_mine();
}

void MainWindow::on_connectWithRoad_triggered() {
  auto proad = road_by_compat_special(ROCO_ROAD);
  if (proad != nullptr) {
    key_unit_connect(ACTIVITY_GEN_ROAD, road_extra_get(proad));
  }
}

void MainWindow::on_connectWithRailway_triggered() {
  auto proad = road_by_compat_special(ROCO_RAILROAD);
  if (proad != nullptr) {
    key_unit_connect(ACTIVITY_GEN_ROAD, road_extra_get(proad));
  }
}

void MainWindow::on_connectWithIrrigation_triggered() {
  auto extras = extra_type_list_by_cause(EC_IRRIGATION);
  if (extra_type_list_size(extras) > 0) {
    auto pextra = extra_type_list_get(extra_type_list_by_cause(EC_IRRIGATION), 0);
    key_unit_connect(ACTIVITY_IRRIGATE, pextra);
  }
}

void MainWindow::on_transform_triggered() {
  key_unit_transform();
}

void MainWindow::on_cleanPollution_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    auto pextra = prev_extra_in_tile(unit_tile(punit), ERM_CLEANPOLLUTION,
                                     unit_owner(punit), punit);
    if (pextra != nullptr) {
      request_new_unit_activity_targeted(punit, ACTIVITY_POLLUTION, pextra);
    }
  } unit_list_iterate_end;
}

void MainWindow::on_cleanNuclearFallout_triggered() {
  key_unit_fallout();
}

void MainWindow::on_helpBuildWonder_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    if (utype_can_do_action(unit_type_get(punit), ACTION_HELP_WONDER)) {
      request_unit_caravan_action(punit, ACTION_HELP_WONDER);
    }
  } unit_list_iterate_end;
}


void MainWindow::on_establishTraderoute_triggered() {
  unit_list_iterate(get_units_in_focus(), punit) {
    if (unit_can_est_trade_route_here(punit)) {
      request_unit_caravan_action(punit, ACTION_TRADE_ROUTE);
    }
  } unit_list_iterate_end;
}

void MainWindow::on_units_triggered() {
  if (m_unitReport->isVisible()) {
      m_unitReport->hide();
  } else {
    m_unitReport->showAsTable();
    m_unitReport->raise();
  }
}

void MainWindow::on_players_triggered() {
  if (m_players->isVisible()) {
      m_players->hide();
  } else {
    m_players->show();
    m_players->raise();
  }
}

void MainWindow::on_cities_triggered() {
  if (m_cityReport->isVisible()) {
      m_cityReport->hide();
  } else {
    m_cityReport->show();
    m_cityReport->raise();
  }
}

void MainWindow::on_economy_triggered() {
  if (m_economyReport->isVisible()) {
      m_economyReport->hide();
  } else {
    m_economyReport->showAsTable();
    m_economyReport->raise();
  }
}

void MainWindow::on_research_triggered() {
  if (m_scienceReport->isVisible()) {
      m_scienceReport->hide();
  } else {
    m_scienceReport->show();
    m_scienceReport->raise();
  }
}

void MainWindow::on_spaceship_triggered() {}

void MainWindow::on_localOptions_triggered() {
  m_localOptions->checkAndShow();
}

void MainWindow::on_serverOptions_triggered() {
  qCDebug(FC) << "server options";
}

void MainWindow::popupManual() {
  if (m_help->isVisible()) {
    m_help->hide();
  } else {
    m_help->checkAndShow();
    m_help->raise();
  }
}


void MainWindow::addActions() {
  struct Data {
    QString name;
    QString text;
    QString shortcut;
    QString theme;
    QString tooltipOrKey;
    bool enabled;
    bool checkable;
    bool checked;
    UnitActionChecker* checker;
  };

#define DRW(k) "draw_" #k

  auto anyCities = [] (unit_list*) {
    players_iterate(pplayer) {
      if (city_list_size(pplayer->cities) > 0) {
        return true;
      }
    } players_iterate_end;
    return false;
  };

  QVector<Data> actionData{
    {"saveGameAs", "Save Game As...", "Ctrl+Shift+S", "document-save", "Save game as", false, false, false,
      nullptr},
    {"newGame", "New Game...", "Ctrl+N", "document-new", "New game", true, false, false,
      nullptr},
    {"loadScenario", "Load Scenario...", "", "", "Load scenario game", true, false, false,
      nullptr},
    {"loadGame", "Load Game...", "", "", "Load saved game", true, false, false,
      nullptr},
    {"connectToGame", "Connect to Game...", "", "", "Connect to game server", true, false, false,
      nullptr},
    {"quit", "Quit", "Ctrl+Q", "application-exit", "Quit", true, false, false,
      nullptr},
    {"fullScreen", "Fullscreen", "Alt+Return", "", "Play freeciv in full screen", false, true, false,
      nullptr},
    {"minimap", "Minimap", "Ctrl+M", "", "Hide/show overview map", false, true, true,
      nullptr},
    {"cityOutlines", "City Outlines", "Ctrl+Y", "", DRW(city_outlines), false, true, false,
      nullptr},
    {"cityOutput", "City Output", "Alt+W", "", DRW(city_output), false, true, false,
      nullptr},
    {"mapGrid", "Map Grid", "Ctrl+G", "", DRW(map_grid), false, true, false,
      nullptr},
    {"nationalBorders", "National Borders", "Ctrl+B", "", DRW(borders), false, true, false,
      nullptr},
    {"nativeTiles", "Native Tiles", "Alt+Shift+N", "", DRW(native), false, true, false,
      nullptr},
    {"cityFullBar", "City Full Bar", "Alt+F", "", DRW(full_citybar), false, true, false,
      nullptr},
    {"cityNames", "City Names", "Alt+N", "", DRW(city_names), false, true, false,
      nullptr},
    {"cityGrowth", "City Growth", "Ctrl+R", "", DRW(city_growth), false, true, false,
      nullptr},
    {"cityProductionLevels", "City Production Levels", "Ctrl+P", "", DRW(city_productions), false, true, false,
      nullptr},
    {"cityBuyCost", "City Buy Cost", "Alt+B", "", DRW(city_buycost), false, true, false,
      nullptr},
    {"cityTradeRoutes", "City Traderoutes", "Alt+D", "", DRW(city_trade_routes), false, true, false,
      nullptr},
    {"centerView", "Center View", "C", "quickview", "", false, false, false,
      nullptr},
    {"zoomIn", "Zoom In", "Ctrl++", "zoom-in", "", false, false, false,
      nullptr},
    {"zoomOut", "Zoom Out", "Ctrl+-", "zoom-out", "", false, false, false,
      nullptr},
    {"scaleFonts", "Scale Fonts", "", "", "", false, true, false,
      nullptr},
    {"wait", "Wait", "W", "", "", false, false, false,
      nullptr},
    {"done", "Done", "Space", "", "", false, false, false,
      nullptr},
    {"moveNorth", "Move North", "Shift+Up", "", "", false, false, false,
      nullptr},
    {"moveEast", "Move East", "Shift+Right", "", "", false, false, false,
      nullptr},
    {"moveSouth", "Move South", "Shift+Down", "", "", false, false, false,
      nullptr},
    {"moveWest", "Move West", "Shift+Left", "", "", false, false, false,
      nullptr},
    {"moveNortheast", "Move Northeast", "Shift+Ctrl+Up", "", "", false, false, false,
      nullptr},
    {"moveSoutheast", "Move Southeast", "Shift+Ctrl+Right", "", "", false, false, false,
      nullptr},
    {"moveSouthwest", "Move Southwest", "Shift+Ctrl+Down", "", "", false, false, false,
      nullptr},
    {"moveNorthwest", "Move Northwest", "Shift+Ctrl+Left", "", "", false, false, false,
      nullptr},
    {"panNorth", "Pan North", "Alt+Up", "", "", false, false, false,
      nullptr},
    {"panEast", "Pan East", "Alt+Right", "", "", false, false, false,
      nullptr},
    {"panSouth", "Pan South", "Alt+Down", "", "", false, false, false,
      nullptr},
    {"panWest", "Pan West", "Alt+Left", "", "", false, false, false,
      nullptr},
    {"previousFocus", "Previous focus", "Alt+PgDown", "", "", false, false, false,
      nullptr},
    {"cancel", "Cancel action", "Esc", "", "", false, false, false,
      nullptr},
    {"endTurn", "End turn", "Shift+Return", "games-endturn", "", false, false, false,
      nullptr},
    {"showUnits", "Show Units", "Ctrl+Space", "", "", false, false, false,
      nullptr},
    {"combatInfo", "Combat info", "Shift+F2", "", "", false, false, false,
      nullptr},
    {"gotoTile", "Go to Tile", "G", "", "", false, false, false,
      new BoolChecker([] (unit_list*) {return true;})},
    {"gotoNearestCity", "Go to Nearest City", "Shift+G", "", "", false, false, false,
      new BoolChecker(anyCities)},
    {"goAirlifttoCity", "Go / Airlift to City...", "T", "", "Go / Airlift to City", false, false, false,
      new BoolChecker(anyCities)},
    {"autoExplore", "Auto Explore", "X", "", "Auto Explore", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_activity(u, ACTIVITY_EXPLORE);})},
    {"patrol", "Patrol", "Q", "", "", false, false, false,
      new BoolChecker([] (unit_list*) {return true;})},
    {"sentry", "Sentry", "S", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_activity(u, ACTIVITY_SENTRY);})},
    {"unsentryAllOnTile", "Unsentry all on Tile", "Shift+S", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return units_have_activity_on_tile(u, ACTIVITY_SENTRY);})},
    {"load", "Load", "L", "", "", false, false, false,
      new BoolChecker(units_can_load)},
    {"unload", "Unload", "U", "", "", false, false, false,
      new BoolChecker(units_can_unload)},
    {"unloadAllFromTransporter", "Unload all from Transporter", "Shift+U", "", "", false, false, false,
      new BoolChecker(units_are_occupied)},
    {"setHomeCity", "Set Home City", "H", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do(u, can_unit_change_homecity);})},
    {"upgrade", "Upgrade", "Shift+U", "", "", false, false, false,
      new BoolChecker(units_can_upgrade)},
    {"convert", "Convert", "Ctrl+O", "", "", false, false, false,
      new BoolChecker(units_can_convert)},
    {"disband", "Disband", "Shift+D", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return units_can_do_action(u, ACTION_DISBAND_UNIT, true);})},
    {"fortifyUnit", "Fortify Unit", "F", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_activity(u, ACTIVITY_FORTIFYING);})},
    {"buildFortFortressBuoy", "Build Fort/Fortress/Buoy", "Shift+F", "", "Build Fort/Fortress/Buoy", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_base_gui(u, BASE_GUI_FORTRESS);})},
    {"buildAirstripAirbase", "Build Airstrip/Airbase", "Shift+E", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_base_gui(u, BASE_GUI_AIRBASE);})},
    {"pillage", "Pillage...", "Shift+P", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_activity(u, ACTIVITY_PILLAGE);})},
    {"buildCity", "Build City", "B", "", "", false, false, false,
      new BuildCityChecker},
    {"autoWorker", "Auto Worker", "A", "", "Auto Worker", false, false, false,
      new AutoWorkerChecker},
    {"buildRoad", "Build Road", "R", "", "", false, false, false,
      new RoadChecker},
    {"irrigate", "Irrigate", "I", "", "Irrigate", false, false, false,
      new IrrigateChecker},
    {"mine", "Mine", "M", "", "Mine", false, false, false,
      new MineChecker},
    {"connectWithRoad", "Connect with Road", "Shift+R", "", "", false, false, false,
      new ConnectRoadChecker},
    {"connectWithRailway", "Connect with Railway", "Shift+L", "", "", false, false, false,
      new ConnectRailChecker},
    {"connectWithIrrigation", "Connect with Irrigation", "Shift+I", "", "", false, false, false,
      new ConnectIrrigationChecker},
    {"transform", "Transform", "O", "", "Transform", false, false, false,
      new TransformChecker},
    {"cleanPollution", "Clean Pollution", "P", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_activity(u, ACTIVITY_POLLUTION);})},
    {"cleanNuclearFallout", "Clean Nuclear Fallout", "N", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do_activity(u, ACTIVITY_FALLOUT);})},
    {"helpBuildWonder", "Help Build Wonder", "B", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do(u, unit_can_help_build_wonder_here);})},
    {"establishTraderoute", "Establish Traderoute", "R", "", "", false, false, false,
      new BoolChecker([] (unit_list* u) {return can_units_do(u, unit_can_est_trade_route_here);})},
    {"units", "Units", "F2", "", "", false, false, false,
      nullptr},
    {"players", "Players", "F3", "", "", false, false, false,
      nullptr},
    {"cities", "Cities", "F4", "", "", false, false, false,
      nullptr},
    {"economy", "Economy", "F5", "", "", false, false, false,
      nullptr},
    {"research", "Research", "F6", "", "", false, false, false,
      nullptr},
    {"spaceship", "Spaceship", "F12", "", "", false, false, false,
      nullptr},
    {"localOptions", "Local Options...", "", "configure", "", true, false, false,
      nullptr},
    {"serverOptions", "Server Options...", "", "configure", "", false, false, false,
      nullptr},
  };


#undef DRW

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
    if (!d.tooltipOrKey.isEmpty()) {
      auto opt = optset_option_by_name(client_optset, d.tooltipOrKey.toUtf8());
      if (opt != nullptr) {
        a->setToolTip(option_description(opt));
        a->setWhatsThis(option_help_text(opt));
        m_optionActions[d.tooltipOrKey] = d.name;
        if (d.checkable) { // boolean option
          a->setChecked(option_bool_get(opt));
        }
      } else {
        a->setToolTip(d.tooltipOrKey);
      }
    }
    if (!d.shortcut.isEmpty()) {
      actionCollection()->setDefaultShortcut(a, d.shortcut);
    }
    a->setEnabled(d.enabled);

    if (d.checker != nullptr) {
      m_checkers[d.name] = d.checker;
    } else if (!d.enabled) {
      m_staticGameActions << d.name;
    }
  }

  // Customized help menu

  auto help = new KHelpMenu(this, KAboutData::applicationData(), true);

  QAction *whatsThisAction = help->action(KHelpMenu::menuWhatsThis);
  actionCollection()->addAction(whatsThisAction->objectName(), whatsThisAction);

  QAction *reportBugAction = help->action(KHelpMenu::menuReportBug);
  actionCollection()->addAction(reportBugAction->objectName(), reportBugAction);

  QAction *aboutAppAction = help->action(KHelpMenu::menuAboutApp);
  actionCollection()->addAction(aboutAppAction->objectName(), aboutAppAction);

  QAction *aboutKdeAction = help->action(KHelpMenu::menuAboutKDE);
  actionCollection()->addAction(aboutKdeAction->objectName(), aboutKdeAction);

  QAction* manual = KStandardAction::helpContents(this, &MainWindow::popupManual, this);
  actionCollection()->addAction(manual->objectName(), manual);

  setHelpMenuEnabled(false);

  setupGUI();

  QMetaObject::connectSlotsByName(this);
}

void MainWindow::checkActions() {
  CheckerMapIterator it(m_checkers);
  auto units = get_units_in_focus();
  if (unit_list_size(units) == 0) {
    while (it.hasNext()) {
      it.next();
      actionCollection()->action(it.key())->setEnabled(false);
    }
  } else {
    while (it.hasNext()) {
      it.next();
      it.value()->check(units, actionCollection()->action(it.key()));
    }
  }
}

void MainWindow::updateOption(const void *d) {
  auto opt = static_cast<const option*>(d);
  if (option_optset(opt) != client_optset) return;
  if (option_type(opt) != OT_BOOLEAN) return; // only booleans supported atm
  QString optName = option_name(opt);
  if (!m_optionActions.contains(optName)) return;
  actionCollection()->action(m_optionActions[optName])->setChecked(option_bool_get(opt));
}

void MainWindow::registerPaneAction(QAction *a, int idx, const QKeySequence& sc) {
  actionCollection()->addAction(QString("pane_%1").arg(idx), a);
  actionCollection()->setDefaultShortcut(a, sc);
}
