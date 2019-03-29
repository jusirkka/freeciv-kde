extern "C" {
#include "cityrep_g.h"
}
#include "logging.h"
#include "application.h"

void city_report_dialog_popup(bool /*raise*/) {
  qCDebug(FC) << "city_report_dialog_popup";
  KV::Application::PopupCityReport();
}

void real_city_report_dialog_update(void */*unused*/) {
  // qCDebug(FC) << "real_city_report_dialog_update";
  KV::Application::UpdateCityReport();
}

void real_city_report_update_city(struct city *pcity) {
  qCDebug(FC) << "real_city_report_update_city";
  KV::Application::UpdateCity(pcity);
}

void hilite_cities_from_canvas() {
  qCDebug(FC) << "TODO: hilite_cities_from_canvas";
}

void toggle_city_hilite(struct city *pcity, bool on_off) {
  qCDebug(FC) << "TODO: toggle_city_hilite";
}

