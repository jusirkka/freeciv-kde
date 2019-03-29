extern "C" {
#include "repodlgs_g.h"
}
#include "logging.h"
#include "application.h"

void science_report_dialog_popup(bool /*raise*/) {
  // qCDebug(FC) << "TODO: science_report_dialog_popup";
  KV::Application::PopupScienceReport();
}

void science_report_dialog_redraw() {
  // qCDebug(FC) << "TODO: science_report_dialog_redraw";
  KV::Application::UpdateScienceReport();
}

void economy_report_dialog_popup(bool raise) {
  qCDebug(FC) << "TODO: economy_report_dialog_popup";
}

void units_report_dialog_popup(bool raise) {
  qCDebug(FC) << "TODO: units_report_dialog_popup";
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

void real_economy_report_dialog_update(void *unused) {
  qCDebug(FC) << "TODO: real_economy_report_dialog_update";
}

void real_units_report_dialog_update(void *unused) {
  qCDebug(FC) << "TODO: real_units_report_dialog_update";
}

