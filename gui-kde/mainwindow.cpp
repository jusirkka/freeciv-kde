#include "fc_config.h"

extern "C" {
#include "gotodlg_g.h"
#include "dialogs_g.h"
}

#include "client_main.h"
#include "clinet.h"
#include "control.h"
#include "options.h"

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
  action("saveGameAs")->setEnabled(ok);
  action("fullScreen")->setEnabled(ok);
  action("minimap")->setEnabled(ok);
  action("cityOutlines")->setEnabled(ok);
  action("cityOutput")->setEnabled(ok);
  action("mapGrid")->setEnabled(ok);
  action("nationalBorders")->setEnabled(ok);
  action("nativeTiles")->setEnabled(ok);
  action("cityFullBar")->setEnabled(ok);
  action("cityNames")->setEnabled(ok);
  action("cityGrowth")->setEnabled(ok);
  action("cityProductionLevels")->setEnabled(ok);
  action("cityBuyCost")->setEnabled(ok);
  action("cityTradeRoutes")->setEnabled(ok);
  action("centerView")->setEnabled(ok);
  action("zoomIn")->setEnabled(ok);
  action("zoomOut")->setEnabled(ok);
  action("scaleFonts")->setEnabled(ok);
  action("gotoTile")->setEnabled(ok);
  action("gotoNearestCity")->setEnabled(ok);
  action("goAirlifttoCity")->setEnabled(ok);
  action("autoExplore")->setEnabled(ok);
  action("patrol")->setEnabled(ok);
  action("sentry")->setEnabled(ok);
  action("unsentryAllOnTile")->setEnabled(ok);
  action("load")->setEnabled(ok);
  action("unload")->setEnabled(ok);
  action("unloadAllFromTransporter")->setEnabled(ok);
  action("setHomeCity")->setEnabled(ok);
  action("upgrade")->setEnabled(ok);
  action("convert")->setEnabled(ok);
  action("disband")->setEnabled(ok);
  action("wait")->setEnabled(ok);
  action("done")->setEnabled(ok);
  action("fortifyUnit")->setEnabled(ok);
  action("buildFortFortressBuoy")->setEnabled(ok);
  action("buildAirstripAirbase")->setEnabled(ok);
  action("pillage")->setEnabled(ok);
  action("buildCity")->setEnabled(ok);
  action("autoWorker")->setEnabled(ok);
  action("buildRoad")->setEnabled(ok);
  action("irrigate")->setEnabled(ok);
  action("mine")->setEnabled(ok);
  action("connectWithRoad")->setEnabled(ok);
  action("connectWithRailway")->setEnabled(ok);
  action("connectWithIrrigation")->setEnabled(ok);
  action("transform")->setEnabled(ok);
  action("cleanPollution")->setEnabled(ok);
  action("cleanNuclearFallout")->setEnabled(ok);
  action("helpBuildWonder")->setEnabled(ok);
  action("establishTraderoute")->setEnabled(ok);
  action("units")->setEnabled(ok);
  action("players")->setEnabled(ok);
  action("cities")->setEnabled(ok);
  action("economy")->setEnabled(ok);
  action("research")->setEnabled(ok);
  action("spaceship")->setEnabled(ok);
  action("options")->setEnabled(ok);
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
  if (gui_options.draw_city_outlines != on) {
    key_city_outlines_toggle();
  }
}

void MainWindow::on_cityOutput_toggled(bool on) {
  if (gui_options.draw_city_output != on) {
    key_city_output_toggle();
  }
}

void MainWindow::on_mapGrid_toggled(bool on) {
  if (gui_options.draw_map_grid != on) {
    key_map_grid_toggle();
  }
}

void MainWindow::on_nationalBorders_toggled(bool on) {
  if (gui_options.draw_borders != on) {
    key_map_borders_toggle();
  }
}

void MainWindow::on_nativeTiles_toggled(bool on) {
  if (gui_options.draw_native != on) {
    key_map_native_toggle();
  }
}

void MainWindow::on_cityFullBar_toggled(bool on) {
  if (gui_options.draw_full_citybar != on) {
    key_city_full_bar_toggle();
  }
}


void MainWindow::on_cityNames_toggled(bool on) {
  if (gui_options.draw_city_names != on) {
    key_city_names_toggle();
  }
}

void MainWindow::on_cityGrowth_toggled(bool on) {
  if (gui_options.draw_city_growth != on) {
    key_city_growth_toggle();
  }
}

void MainWindow::on_cityProductionLevels_toggled(bool on) {
  if (gui_options.draw_city_productions != on) {
    key_city_productions_toggle();
  }
}

void MainWindow::on_cityBuyCost_toggled(bool on) {
  if (gui_options.draw_city_buycost != on) {
    key_city_buycost_toggle();
  }
}

void MainWindow::on_cityTradeRoutes_toggled(bool on) {
  if (gui_options.draw_city_trade_routes != on) {
    key_city_trade_routes_toggle();
  }
}

void MainWindow::on_centerView_triggered() {
  request_center_focus_unit();
}

void MainWindow::on_zoomIn_triggered() {
  m_mapView->zoomIn();
  tilespec_reread(tileset_basename(tileset), true, m_mapView->zoomLevel());
}

void MainWindow::on_zoomOut_triggered() {
  m_mapView->zoomOut();
  tilespec_reread(tileset_basename(tileset), true, m_mapView->zoomLevel());
}


void MainWindow::on_scaleFonts_toggled(bool on) {}

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

void MainWindow::on_units_triggered() {}

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

void MainWindow::on_economy_triggered() {}

void MainWindow::on_research_triggered() {
  if (m_scienceReport->isVisible()) {
      m_scienceReport->hide();
  } else {
    m_scienceReport->show();
    m_scienceReport->raise();
  }
}

void MainWindow::on_spaceship_triggered() {}
void MainWindow::on_options_triggered() {}


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
  };

#define DRW(k) "draw_" #k

  QVector<Data> actionData{
    {"saveGameAs", "Save Game As...", "Ctrl+Shift+S", "document-save", "Save game as", false, false, false},
    {"newGame", "New Game...", "Ctrl+N", "document-new", "New game", true, false, false},
    {"loadScenario", "Load Scenario...", "", "", "Load scenario game", true, false, false},
    {"loadGame", "Load Game...", "", "", "Load saved game", true, false, false},
    {"connectToGame", "Connect to Game...", "", "", "Connect to game server", true, false, false},
    {"quit", "Quit", "Ctrl+Q", "application-exit", "Quit", true, false, false},
    {"fullScreen", "Fullscreen", "Alt+Return", "", "Play freeciv in full screen", false, true, false},
    {"minimap", "Minimap", "Ctrl+M", "", "Hide/show overview map", false, true, true},
    {"cityOutlines", "City Outlines", "Ctrl+Y", "", DRW(city_outlines), false, true, false},
    {"cityOutput", "City Output", "Alt+W", "", DRW(city_output), false, true, false},
    {"mapGrid", "Map Grid", "Ctrl+G", "", DRW(map_grid), false, true, false},
    {"nationalBorders", "National Borders", "Ctrl+B", "", DRW(borders), false, true, false},
    {"nativeTiles", "Native Tiles", "Alt+Shift+N", "", DRW(native), false, true, false},
    {"cityFullBar", "City Full Bar", "Alt+F", "", DRW(full_citybar), false, true, false},
    {"cityNames", "City Names", "Alt+N", "", DRW(city_names), false, true, false},
    {"cityGrowth", "City Growth", "Ctrl+R", "", DRW(city_growth), false, true, false},
    {"cityProductionLevels", "City Production Levels", "Ctrl+P", "", DRW(city_productions), false, true, false},
    {"cityBuyCost", "City Buy Cost", "Alt+B", "", DRW(city_buycost), false, true, false},
    {"cityTradeRoutes", "City Traderoutes", "Alt+D", "", DRW(city_trade_routes), false, true, false},
    {"centerView", "Center View", "C", "", "", false, false, false},
    {"zoomIn", "Zoom In", "Ctrl++", "zoom-in", "", false, false, false},
    {"zoomOut", "Zoom Out", "Ctrl+-", "zoom-out", "", false, false, false},
    {"scaleFonts", "Scale Fonts", "", "", "", false, true, false},
    {"gotoTile", "Go to Tile", "G", "", "", false, false, false},
    {"gotoNearestCity", "Go to Nearest City", "Shift+G", "", "", false, false, false},
    {"goAirlifttoCity", "Go / Airlift to City...", "T", "", "Go / Airlift to City", false, false, false},
    {"autoExplore", "Auto Explore", "X", "", "Auto Explore", false, false, false},
    {"patrol", "Patrol", "Q", "", "", false, false, false},
    {"sentry", "Sentry", "S", "", "", false, false, false},
    {"unsentryAllOnTile", "Unsentry all on Tile", "Shift+S", "", "", false, false, false},
    {"load", "Load", "L", "", "", false, false, false},
    {"unload", "Unload", "U", "", "", false, false, false},
    {"unloadAllFromTransporter", "Unload all from Transporter", "Shift+U", "", "", false, false, false},
    {"setHomeCity", "Set Home City", "H", "", "", false, false, false},
    {"upgrade", "Upgrade", "Shift+U", "", "", false, false, false},
    {"convert", "Convert", "Ctrl+O", "", "", false, false, false},
    {"disband", "Disband", "Shift+D", "", "", false, false, false},
    {"wait", "Wait", "W", "", "", false, false, false},
    {"done", "Done", "Space", "", "", false, false, false},
    {"fortifyUnit", "Fortify Unit", "F", "", "", false, false, false},
    {"buildFortFortressBuoy", "Build Fort/Fortress/Buoy", "Shift+F", "", "Build Fort/Fortress/Buoy", false, false, false},
    {"buildAirstripAirbase", "Build Airstrip/Airbase", "Shift+E", "", "", false, false, false},
    {"pillage", "Pillage...", "Shift+P", "", "", false, false, false},
    {"buildCity", "Build City", "B", "", "", false, false, false},
    {"autoWorker", "Auto Worker", "A", "", "Auto Worker", false, false, false},
    {"buildRoad", "Build Road", "R", "", "", false, false, false},
    {"irrigate", "Irrigate", "I", "", "Irrigate", false, false, false},
    {"mine", "Mine", "M", "", "Mine", false, false, false},
    {"connectWithRoad", "Connect with Road", "Shift+R", "", "", false, false, false},
    {"connectWithRailway", "Connect with Railway", "Shift+L", "", "", false, false, false},
    {"connectWithIrrigation", "Connect with Irrigation", "Shift+L", "", "", false, false, false},
    {"transform", "Transform", "O", "", "Transform", false, false, false},
    {"cleanPollution", "Clean Pollution", "P", "", "", false, false, false},
    {"cleanNuclearFallout", "Clean Nuclear Fallout", "N", "", "", false, false, false},
    {"helpBuildWonder", "Help Build Wonder", "B", "", "", false, false, false},
    {"establishTraderoute", "Establish Traderoute", "R", "", "", false, false, false},
    {"units", "Units", "F2", "", "", false, false, false},
    {"players", "Players", "F3", "", "", false, false, false},
    {"cities", "Cities", "F4", "", "", false, false, false},
    {"economy", "Economy", "F5", "", "", false, false, false},
    {"research", "Research", "F6", "", "", false, false, false},
    {"spaceship", "Spaceship", "F12", "", "", false, false, false},
    {"options", "Options...", "", "configure", "", true, false, false},
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
  }

  setupGUI();
  QMetaObject::connectSlotsByName(this);
}
