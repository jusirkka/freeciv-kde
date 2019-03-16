extern "C" {
#include "messagewin_g.h"
}
#include "logging.h"
#include "application.h"

void meswin_dialog_popup(bool raise) {
  qCDebug(FC) << "TODO: meswin_dialog_popup";
}

bool meswin_dialog_is_open() {
  qCDebug(FC) << "TODO: meswin_dialog_is_open";
  return true;
}

void real_meswin_dialog_update(void */*unused*/) {
  // qCDebug(FC) << "TODO: real_meswin_dialog_update";
  KV::Application::UpdateMessages();
}

