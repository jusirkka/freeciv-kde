extern "C" {
#include "plrdlg_g.h"
}
#include "logging.h"
#include "application.h"

void popup_players_dialog(bool /*raise*/) {
  // qCDebug(FC) << "popup_players_dialog";
  KV::Application::PopupPlayers();
}

void real_players_dialog_update(void*) {
  // qCDebug(FC) << "real_players_dialog_update";
  KV::Application::UpdatePlayers();
}

