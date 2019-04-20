#pragma once

#include <KXmlGuiWindow>
#include <QStateMachine>

extern "C" {
#include "pages_g.h"
}


namespace KV {

namespace State {
class Base;
class Network;
class Game;
class Start;
}

class ChatLineEdit;
class OutputPaneManager;
class MapView;
class PlayerDialog;
class CityView;
class CityDialog;
class ScienceDialog;
class UnitActionChecker;
class UnitReport;
class EconomyReport;
class HelpDialog;
class LocalOptionsDialog;
class ServerOptionsDialog;

class MainWindow: public KXmlGuiWindow
{

  Q_OBJECT

  friend class State::Network;
  friend class State::Game;
  friend class State::Start;
  friend class OutputPaneManager;

public:

  MainWindow();
  ~MainWindow() override;

  client_pages state() const;
  void enableGameMenus(bool ok);
  void setMapView(MapView* map);

  ChatLineEdit* chatLine() {return m_chatLine;}

protected:

  void resizeEvent(QResizeEvent *event) override;
  void closeEvent(QCloseEvent *event) override;


private slots:

  void on_saveGameAs_triggered();
  void on_newGame_triggered();
  void on_loadScenario_triggered();
  void on_loadGame_triggered();
  void on_quit_triggered();
  void on_fullScreen_toggled(bool on);
  void on_minimap_toggled(bool on);
  void on_cityOutlines_toggled(bool on);
  void on_cityOutput_toggled(bool on);
  void on_mapGrid_toggled(bool on);
  void on_nationalBorders_toggled(bool on);
  void on_nativeTiles_toggled(bool on);
  void on_cityFullBar_toggled(bool on);
  void on_cityNames_toggled(bool on);
  void on_cityGrowth_toggled(bool on);
  void on_cityProductionLevels_toggled(bool on);
  void on_cityBuyCost_toggled(bool on);
  void on_cityTradeRoutes_toggled(bool on);
  void on_centerView_triggered();
  void on_zoomIn_triggered();
  void on_zoomOut_triggered();
  void on_scaleFonts_toggled(bool on);
  void on_gotoTile_triggered();
  void on_gotoNearestCity_triggered();
  void on_goAirlifttoCity_triggered();
  void on_autoExplore_triggered();
  void on_patrol_triggered();
  void on_sentry_triggered();
  void on_unsentryAllOnTile_triggered();
  void on_load_triggered();
  void on_unload_triggered();
  void on_unloadAllFromTransporter_triggered();
  void on_setHomeCity_triggered();
  void on_upgrade_triggered();
  void on_convert_triggered();
  void on_disband_triggered();
  void on_wait_triggered();
  void on_done_triggered();
  void on_moveNorth_triggered();
  void on_moveEast_triggered();
  void on_moveSouth_triggered();
  void on_moveWest_triggered();
  void on_moveNortheast_triggered();
  void on_moveSoutheast_triggered();
  void on_moveSouthwest_triggered();
  void on_moveNorthwest_triggered();
  void on_panNorth_triggered();
  void on_panEast_triggered();
  void on_panSouth_triggered();
  void on_panWest_triggered();
  void on_previousFocus_triggered();
  void on_cancel_triggered();
  void on_endTurn_triggered();
  void on_showUnits_triggered();
  void on_combatInfo_triggered();
  void on_fortifyUnit_triggered();
  void on_buildFortFortressBuoy_triggered();
  void on_buildAirstripAirbase_triggered();
  void on_pillage_triggered();
  void on_buildCity_triggered();
  void on_autoWorker_triggered();
  void on_buildRoad_triggered();
  void on_irrigate_triggered();
  void on_mine_triggered();
  void on_connectWithRoad_triggered();
  void on_connectWithRailway_triggered();
  void on_connectWithIrrigation_triggered();
  void on_transform_triggered();
  void on_cleanPollution_triggered();
  void on_cleanNuclearFallout_triggered();
  void on_helpBuildWonder_triggered();
  void on_establishTraderoute_triggered();
  void on_units_triggered();
  void on_players_triggered();
  void on_cities_triggered();
  void on_economy_triggered();
  void on_research_triggered();
  void on_spaceship_triggered();
  void on_localOptions_triggered();
  void on_serverOptions_triggered();

  void setCurrentState(bool active);
  void stateChange(client_pages page);
  void restartStateMachine();
  void checkActions();
  void updateOption(const void* d); // really an option pointer
  void popupManual();

signals:

  void resetStateMachine();
  void startNewGame(const QString& fromFile);

private:

  void writeSettings();
  void readSettings();
  void addActions();
  void createStateMachine();
  void registerPaneAction(QAction* a, int idx, const QKeySequence& sc);
  void loadGame(const QStringList& dirs);

private:

  using CheckerMap = QMap<QString, UnitActionChecker*>;
  using CheckerMapIterator = QMapIterator<QString, UnitActionChecker*>;
  using OptionNameMap = QMap<QString, QString>;

  QStateMachine m_states;
  State::Base* m_currentState;
  QVector<State::Base*> m_stateList;
  MapView* m_mapView;
  OutputPaneManager* m_panes;
  ChatLineEdit* m_chatLine = nullptr;
  PlayerDialog* m_players;
  CityView* m_cityReport;
  CityDialog* m_cityManager;
  ScienceDialog* m_scienceReport;
  QRect m_fallbackGeom;
  CheckerMap m_checkers;
  QStringList m_staticGameActions;
  OptionNameMap m_optionActions;
  UnitReport* m_unitReport;
  EconomyReport* m_economyReport;
  HelpDialog* m_help;
  LocalOptionsDialog* m_localOptions;
  ServerOptionsDialog* m_serverOptions;
};

}
