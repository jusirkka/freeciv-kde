extern "C" {
#include "luaconsole_g.h"
}
#include "logging.h"

void luaconsole_dialog_popup(bool raise) {
  qCDebug(FC) << "luaconsole_dialog_popup";
}

bool luaconsole_dialog_is_open() {
  qCDebug(FC) << "luaconsole_dialog_is_open";
  return false;
}

void real_luaconsole_dialog_update() {
  qCDebug(FC) << "real_luaconsole_dialog_update";
}

void real_luaconsole_append(const char *astring, const struct text_tag_list *tags) {
  qCDebug(FC) << "real_luaconsole_append";
}

