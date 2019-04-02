extern "C" {
#include "messagewin_g.h"
}
#include "logging.h"
#include "application.h"

void meswin_dialog_popup(bool /*raise*/) {
  qCDebug(FC) << "meswin_dialog_popup: dummy";
}

bool meswin_dialog_is_open() {
  qCDebug(FC) << "meswin_dialog_is_open: dummy";
  return true;
}

void real_meswin_dialog_update(void */*unused*/) {
  // qCDebug(FC) << "TODO: real_meswin_dialog_update";
  KV::Application::UpdateMessages();
}

