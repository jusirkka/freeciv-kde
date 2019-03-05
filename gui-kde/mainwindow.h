#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStateMachine>

extern "C" {
#include "pages_g.h"
}

namespace Ui {class MainWindow;}

namespace KV {

namespace State {
class Base;
class Network;
}

class MainWindow: public QMainWindow
{

  Q_OBJECT

  friend class State::Network;

public:

  MainWindow();
  ~MainWindow() override;

  client_pages state() const;


protected:

    void closeEvent(QCloseEvent *event) override;


private slots:

  void on_actionSaveGameAs_triggered();
  void on_actionLoadScenario_triggered();
  void on_actionLoadGame_triggered();
  void on_actionQuit_triggered();
  void on_actionFullscreen_toggled(bool on);
  void on_actionMinimap_toggled(bool on);
  void on_actionCityOutlines_toggled(bool on);
  void on_actionCityOutput_toggled(bool on);
  void on_actionMapGrid_toggled(bool on);
  void on_actionNationalBorders_toggled(bool on);
  void on_actionNativeTiles_toggled(bool on);
  void on_actionCityFullBar_toggled(bool on);
  void on_actionCityNames_toggled(bool on);
  void on_actionCityGrowth_toggled(bool on);
  void on_actionCityProductionLevels_toggled(bool on);
  void on_actionCityBuyCost_toggled(bool on);
  void on_actionCityTradeRoutes_toggled(bool on);
  void on_actionCenterView_triggered();
  void on_actionZoomIn_triggered();
  void on_actionZoomOut_triggered();
  void on_actionScaleFonts_toggled(bool on);
  void on_actionGotoTile_triggered();
  void on_actionGotoNearestCity_triggered();
  void on_actionGoAirlifttoCity_triggered();
  void on_actionAutoExplore_triggered();
  void on_actionPatrol_triggered();
  void on_actionSentry_triggered();
  void on_actionUnsentryAllOnTile_triggered();
  void on_actionLoad_triggered();
  void on_actionUnload_triggered();
  void on_actionUnloadAllFromTransporter_triggered();
  void on_actionSetHomeCity_triggered();
  void on_actionUpgrade_triggered();
  void on_actionConvert_triggered();
  void on_actionDisband_triggered();
  void on_actionWait_triggered();
  void on_actionDone_triggered();
  void on_actionFortifyUnit_triggered();
  void on_actionBuildFortFortressBuoy_triggered();
  void on_actionBuildAirstripAirbase_triggered();
  void on_actionPillage_triggered();
  void on_actionBuildCity_triggered();
  void on_actionAutoWorker_triggered();
  void on_actionBuildRoad_triggered();
  void on_actionTransformToPlains_triggered();
  void on_actionTransformToGrassland_triggered();
  void on_actionConnectWithRoad_triggered();
  void on_actionConnectWithRailway_triggered();
  void on_actionConnectWithIrrigation_triggered();
  void on_actionTransformTerrain_triggered();
  void on_actionCleanPollution_triggered();
  void on_actionCleanNuclearFallout_triggered();
  void on_actionHelpBuildWonder_triggered();
  void on_actionEstablishTraderoute_triggered();
  void on_actionUnits_triggered();
  void on_actionPlayers_triggered();
  void on_actionCities_triggered();
  void on_actionEconomy_triggered();
  void on_actionResearch_triggered();
  void on_actionSpaceship_triggered();
  void on_actionAchievements_triggered();
  void on_actionAbout_triggered();
  void on_actionHandbook_triggered();
  void on_actionConfigureShortcuts_triggered();
  void on_actionConfigureToolbar_triggered();
  void on_actionOptions_triggered();
  void on_actionShowMenubar_toggled(bool on);
  void on_actionShowToolbar_toggled(bool on);

  void setCurrentState(bool active);

private:

  void writeSettings();
  void readSettings();

private:

  Ui::MainWindow* m_ui;
  QStateMachine m_states;
  State::Base* m_currentState;

};

}
#endif // MAINWINDOW_H
