extern "C" {
#include "repodlgs_g.h"
}
#include "logging.h"

void science_report_dialog_popup(bool raise) {
  qCDebug(FC) << "science_report_dialog_popup";
}

void science_report_dialog_redraw() {
  qCDebug(FC) << "science_report_dialog_redraw";
}

void economy_report_dialog_popup(bool raise) {
  qCDebug(FC) << "economy_report_dialog_popup";
}

void units_report_dialog_popup(bool raise) {
  qCDebug(FC) << "units_report_dialog_popup";
}

void endgame_report_dialog_start(const struct packet_endgame_report *packet) {
  qCDebug(FC) << "endgame_report_dialog_start";
}

void endgame_report_dialog_player(const struct packet_endgame_player *packet) {
  qCDebug(FC) << "endgame_report_dialog_player";
}

void real_science_report_dialog_update(void *unused) {
  qCDebug(FC) << "real_science_report_dialog_update";
}

void real_economy_report_dialog_update(void *unused) {
  qCDebug(FC) << "real_economy_report_dialog_update";
}

void real_units_report_dialog_update(void *unused) {
  qCDebug(FC) << "real_units_report_dialog_update";
}

