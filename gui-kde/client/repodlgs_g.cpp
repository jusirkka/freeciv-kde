extern "C" {
#include "repodlgs_g.h"
}
#include "logging.h"
#include "application.h"

void science_report_dialog_popup(bool raise) {
  // qCDebug(FC) << "TODO: science_report_dialog_popup";
  if (raise) {
    KV::Application::PopupScienceReport();
  } else {
    KV::Application::UpdateScienceReport();
  }
}

void science_report_dialog_redraw() {
  // qCDebug(FC) << "TODO: science_report_dialog_redraw";
  KV::Application::UpdateScienceReport();
}

void economy_report_dialog_popup(bool /*raise*/) {
  // qCDebug(FC) << "economy_report_dialog_popup";
  KV::Application::PopupEconomyReport();
}

void units_report_dialog_popup(bool /*raise*/) {
  // qCDebug(FC) << "units_report_dialog_popup";
  KV::Application::PopupUnitReport();
}

void endgame_report_dialog_start(const struct packet_endgame_report *packet) {
  qCDebug(FC) << "TODO: endgame_report_dialog_start";
}

void endgame_report_dialog_player(const struct packet_endgame_player *packet) {
  qCDebug(FC) << "TODO: endgame_report_dialog_player";
}

void real_science_report_dialog_update(void */*unused*/) {
  // qCDebug(FC) << "TODO: real_science_report_dialog_update";
  KV::Application::UpdateScienceReport();
}

void real_economy_report_dialog_update(void */*unused*/) {
  // qCDebug(FC) << "real_economy_report_dialog_update";
  KV::Application::UpdateEconomyReport();
}

void real_units_report_dialog_update(void */*unused*/) {
  // qCDebug(FC) << "real_units_report_dialog_update";
  KV::Application::UpdateUnitReport();
}

