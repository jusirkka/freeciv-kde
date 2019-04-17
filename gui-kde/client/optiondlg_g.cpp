extern "C" {
#include "optiondlg_g.h"
}
#include "options.h"
#include "client_main.h"

#include "logging.h"
#include "application.h"

void option_dialog_popup(const char *name, const struct option_set */*poptset*/) {
  qCDebug(FC) << "option_dialog_popup: dummy" << name;
}

void option_dialog_popdown(const struct option_set */*poptset*/) {
  qCDebug(FC) << "option_dialog_popdown: dummy";
}

void option_gui_update(option *opt) {
  if (client_state() < C_S_PREPARING) return;
  KV::Application::UpdateOption(opt);
}

void option_gui_add(option *opt) {
  if (client_state() < C_S_PREPARING) return;
  KV::Application::AddOption(opt);
}

void option_gui_remove(option *opt) {
  if (client_state() < C_S_PREPARING) return;
  KV::Application::DelOption(opt);
}

