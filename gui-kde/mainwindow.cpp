#include "fc_config.h"

#include "client_main.h"
#include "clinet.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "messagebox.h"

using namespace KV;

MainWindow::MainWindow()
  : QMainWindow()
  , m_UI(new Ui::MainWindow)
{
  m_UI->setupUi(this);
}

MainWindow::~MainWindow() {
  delete m_UI;
}

void MainWindow::closeEvent(QCloseEvent *event) {
}

void MainWindow::on_actionSaveGameAs_triggered() {}
void MainWindow::on_actionNewGame_triggered() {}
void MainWindow::on_actionLoadScenario_triggered() {}
void MainWindow::on_actionLoadGame_triggered() {}
void MainWindow::on_actionConnectToGame_triggered() {}

void MainWindow::on_actionQuit_triggered() {
  KV::StandardMessageBox ask(centralWidget(),
                             _("Are you sure you want to quit?"),
                             _("Quit?"));
  if (ask.exec() == QMessageBox::Ok) {
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
void MainWindow::on_actionGotoTile_triggered() {}
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
