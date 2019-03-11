extern "C" {
#include "mapctrl_g.h"
}
#include "logging.h"
#include "application.h"

void popup_newcity_dialog(struct unit *punit, const char *suggestname) {
  qCDebug(FC) << "TODO: popup_newcity_dialog";
}

void set_turn_done_button_state(bool state) {
  // qCDebug(FC) << "set_turn_done_button_state";
  KV::Application::ToggleTurnDone(state);
}

void create_line_at_mouse_pos() {
  qCDebug(FC) << "create_line_at_mouse_pos";
  KV::Application::CreateLineAtMousePos();
}

void update_rect_at_mouse_pos() {
  qCDebug(FC) << "TODO: update_rect_at_mouse_pos";
}

