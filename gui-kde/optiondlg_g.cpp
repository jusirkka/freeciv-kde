extern "C" {
#include "optiondlg_g.h"
}
#include "options.h"
#include "logging.h"

void option_dialog_popup(const char *name, const struct option_set *poptset) {
  qCDebug(FC) << "TODO: option_dialog_popup" << name;
//  options_iterate(poptset, p) {
//    qCDebug(FC) << option_name(p) << option_description(p);
//  } options_iterate_end;
}

void option_dialog_popdown(const struct option_set *poptset) {
  qCDebug(FC) << "TODO: option_dialog_popdown";
//  options_iterate(poptset, p) {
//    qCDebug(FC) << option_name(p) << option_description(p);
//  } options_iterate_end;
}

void option_gui_update(struct option *poption) {
  qCDebug(FC) << "TODO: option_gui_update"
              << option_name(poption) << option_description(poption);
}

void option_gui_add(struct option *poption) {
  qCDebug(FC) << "TODO: option_gui_add"
              << option_name(poption) << option_description(poption);
}

void option_gui_remove(struct option *poption) {
  qCDebug(FC) << "TODO: option_gui_remove"
              << option_name(poption) << option_description(poption);
}

