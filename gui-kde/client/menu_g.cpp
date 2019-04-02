extern "C" {
#include "menu_g.h"
}
#include "logging.h"
#include "application.h"
#include "client_main.h"

void real_menus_init() {
  qCDebug(FC) << "real_menus_init: dummy";
}

void real_menus_update() {
  qCDebug(FC) << "real_menus_update";
  if (client_state() != C_S_RUNNING) return;
  KV::Application::UpdateActions();
}

