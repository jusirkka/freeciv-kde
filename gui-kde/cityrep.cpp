extern "C" {
#include "cityrep_g.h"
}
#include "logging.h"

void city_report_dialog_popup(bool raise) {
  qCDebug(FC) << "city_report_dialog_popup";
}

void real_city_report_dialog_update(void *unused) {
  qCDebug(FC) << "real_city_report_dialog_update";
}

void real_city_report_update_city(struct city *pcity) {
  qCDebug(FC) << "real_city_report_update_city";
}

void hilite_cities_from_canvas() {
  qCDebug(FC) << "hilite_cities_from_canvas";
}

void toggle_city_hilite(struct city *pcity, bool on_off) {
  qCDebug(FC) << "toggle_city_hilite";
}

