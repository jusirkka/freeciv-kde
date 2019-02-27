extern "C" {
#include "dialogs_g.h"
}

void popup_notify_goto_dialog(const char *headline, const char *lines, const struct text_tag_list *tags, struct tile *ptile) {
}

void popup_notify_dialog(const char *caption, const char *headline, const char *lines) {
}

void popup_connect_msg(const char *headline, const char *message) {
}

void popup_races_dialog(struct player *pplayer) {
}

void popdown_races_dialog() {
}

void unit_select_dialog_popup(struct tile *ptile) {
}

void unit_select_dialog_update_real(void *unused) {
}

void races_toggles_set_sensitive() {
}

void races_update_pickable(bool nationset_change) {
}

void show_img_play_snd(const char *img_path, const char *snd_path, const char *desc, bool fullsize) {
}

void popup_action_selection(struct unit *actor_unit, struct city *target_city, struct unit *target_unit, struct tile *target_tile, struct extra_type *target_extra, const struct act_prob *act_probs) {
}

int action_selection_actor_unit() {
}

int action_selection_target_city() {
}

int action_selection_target_unit() {
}

int action_selection_target_tile() {
}

int action_selection_target_extra() {
}

void action_selection_close() {
}

void action_selection_refresh(struct unit *actor_unit, struct city *target_city, struct unit *target_unit, struct tile *target_tile, struct extra_type *target_extra, const struct act_prob *act_probs) {
}

void action_selection_no_longer_in_progress_gui_specific(int actor_unit_id) {
}

void popup_incite_dialog(struct unit *actor, struct city *pcity, int cost, const struct action *paction) {
}

void popup_bribe_dialog(struct unit *actor, struct unit *punit, int cost, const struct action *paction) {
}

void popup_sabotage_dialog(struct unit *actor, struct city *pcity, const struct action *paction) {
}

void popup_pillage_dialog(struct unit *punit, bv_extras extras) {
}

void popup_upgrade_dialog(struct unit_list *punits) {
}

void popup_disband_dialog(struct unit_list *punits) {
}

void popup_tileset_suggestion_dialog() {
}

void popup_soundset_suggestion_dialog() {
}

void popup_musicset_suggestion_dialog() {
}

bool popup_theme_suggestion_dialog(const char *theme_name) {
}

void show_tech_gained_dialog(Tech_type_id tech) {
}

void show_tileset_error(const char *msg) {
}

void popdown_all_game_dialogs() {
}

