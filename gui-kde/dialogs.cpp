extern "C" {
#include "dialogs_g.h"
}
#include "logging.h"

void popup_notify_goto_dialog(const char *headline, const char *lines, const struct text_tag_list *tags, struct tile *ptile) {
  qCDebug(FC) << "popup_notify_goto_dialog";
}

void popup_notify_dialog(const char *caption, const char *headline, const char *lines) {
  qCDebug(FC) << "popup_notify_dialog";
}

void popup_connect_msg(const char *headline, const char *message) {
  qCDebug(FC) << "popup_connect_msg";
}

void popup_races_dialog(struct player *pplayer) {
  qCDebug(FC) << "popup_races_dialog";
}

void popdown_races_dialog() {
  qCDebug(FC) << "popdown_races_dialog";
}

void unit_select_dialog_popup(struct tile *ptile) {
  qCDebug(FC) << "unit_select_dialog_popup";
}

void unit_select_dialog_update_real(void *unused) {
  qCDebug(FC) << "unit_select_dialog_update_real";
}

void races_toggles_set_sensitive() {
  qCDebug(FC) << "races_toggles_set_sensitive";
}

void races_update_pickable(bool nationset_change) {
  qCDebug(FC) << "races_update_pickable";
}

void show_img_play_snd(const char *img_path, const char *snd_path, const char *desc, bool fullsize) {
  qCDebug(FC) << "show_img_play_snd";
}

void popup_action_selection(struct unit *actor_unit, struct city *target_city, struct unit *target_unit, struct tile *target_tile, struct extra_type *target_extra, const struct act_prob *act_probs) {
  qCDebug(FC) << "popup_action_selection";
}

int action_selection_actor_unit() {
  qCDebug(FC) << "action_selection_actor_unit";
  return 0;
}

int action_selection_target_city() {
  qCDebug(FC) << "action_selection_target_city";
  return 0;
}

int action_selection_target_unit() {
  qCDebug(FC) << "action_selection_target_unit";
  return 0;
}

int action_selection_target_tile() {
  qCDebug(FC) << "action_selection_target_tile";
  return 0;
}

int action_selection_target_extra() {
  qCDebug(FC) << "action_selection_target_extra";
  return 0;
}

void action_selection_close() {
  qCDebug(FC) << "action_selection_close";
}

void action_selection_refresh(struct unit *actor_unit, struct city *target_city, struct unit *target_unit, struct tile *target_tile, struct extra_type *target_extra, const struct act_prob *act_probs) {
  qCDebug(FC) << "action_selection_refresh";
}

void action_selection_no_longer_in_progress_gui_specific(int actor_unit_id) {
  qCDebug(FC) << "action_selection_no_longer_in_progress_gui_specific";
}

void popup_incite_dialog(struct unit *actor, struct city *pcity, int cost, const struct action *paction) {
  qCDebug(FC) << "popup_incite_dialog";
}

void popup_bribe_dialog(struct unit *actor, struct unit *punit, int cost, const struct action *paction) {
  qCDebug(FC) << "popup_bribe_dialog";
}

void popup_sabotage_dialog(struct unit *actor, struct city *pcity, const struct action *paction) {
  qCDebug(FC) << "popup_sabotage_dialog";
}

void popup_pillage_dialog(struct unit *punit, bv_extras extras) {
  qCDebug(FC) << "popup_pillage_dialog";
}

void popup_upgrade_dialog(struct unit_list *punits) {
  qCDebug(FC) << "popup_upgrade_dialog";
}

void popup_disband_dialog(struct unit_list *punits) {
  qCDebug(FC) << "popup_disband_dialog";
}

void popup_tileset_suggestion_dialog() {
  qCDebug(FC) << "popup_tileset_suggestion_dialog";
}

void popup_soundset_suggestion_dialog() {
  qCDebug(FC) << "popup_soundset_suggestion_dialog";
}

void popup_musicset_suggestion_dialog() {
  qCDebug(FC) << "popup_musicset_suggestion_dialog";
}

bool popup_theme_suggestion_dialog(const char *theme_name) {
  qCDebug(FC) << "popup_theme_suggestion_dialog";
  return false;
}

void show_tech_gained_dialog(Tech_type_id tech) {
  qCDebug(FC) << "show_tech_gained_dialog";
}

void show_tileset_error(const char *msg) {
  qCDebug(FC) << "show_tileset_error";
}

void popdown_all_game_dialogs() {
  qCDebug(FC) << "popdown_all_game_dialogs";
}

