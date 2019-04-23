extern "C" {
#include "helpdlg_g.h"
}
#include "helpdata.h"
#include "client_main.h"

#include "logging.h"
#include "application.h"


void popup_help_dialog_string(const char *item) {
  // qCDebug(FC) << "TODO: popup_help_dialog_string";
  popup_help_dialog_typed(item , HELP_ANY);
}

void popup_help_dialog_typed(const char *item, enum help_page_type section) {
  // qCDebug(FC) << "popup_help_dialog_typed";
  KV::Application::PopupHelpDialog(QString(item).trimmed(), section);
}

void popdown_help_dialog() {
  // qCDebug(FC) << "popdown_help_dialog";
  // popdown_help_dialog might be called before we have an event loop
  if (client_state() == C_S_INITIAL) return;
  KV::Application::PopdownHelpDialog();
}

