extern "C" {
#include "helpdlg_g.h"
}
#include "logging.h"

void popup_help_dialog_string(const char *item) {
  qCDebug(FC) << "popup_help_dialog_string";
}

void popup_help_dialog_typed(const char *item, enum help_page_type) {
  qCDebug(FC) << "popup_help_dialog_typed";
}

void popdown_help_dialog() {
  qCDebug(FC) << "popdown_help_dialog";
}

