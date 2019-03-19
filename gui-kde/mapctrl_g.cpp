extern "C" {
#include "mapctrl_g.h"
}
#include "logging.h"
#include "application.h"
#include "inputbox.h"
#include "mainwindow.h"

void popup_newcity_dialog(unit *punit, const char *suggestname) {
  // qCDebug(FC) << "TODO: popup_newcity_dialog";
  KV::InputBox ask(KV::Application::Mainwin(),
                   _("What should we call our new city?"),
                   _("Build New City"),
                   QString(suggestname));
  auto t = index_to_tile(&(wld.map), tile_index(unit_tile(punit)));
  if (ask.exec() == QDialog::Accepted) {
    finish_city(t, ask.input().toUtf8());
  } else {
    cancel_city(t);
  }
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

