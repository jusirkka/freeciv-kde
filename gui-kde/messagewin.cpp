extern "C" {
#include "messagewin_g.h"
}
#include "logging.h"

void meswin_dialog_popup(bool raise) {
  qCDebug(FC) << "meswin_dialog_popup";
}

bool meswin_dialog_is_open() {
  qCDebug(FC) << "meswin_dialog_is_open";
  return false;
}

void real_meswin_dialog_update(void *unused) {
  qCDebug(FC) << "real_meswin_dialog_update";
}

