extern "C" {
#include "dialogs_g.h"
}
#include "logging.h"
#include "application.h"
#include "actionselector.h"

void popup_notify_goto_dialog(const char *headline, const char *lines, const struct text_tag_list *tags, struct tile *ptile) {
  qCDebug(FC) << "TODO: popup_notify_goto_dialog"
              << headline << KV::Application::ApplyTags(lines, tags);
}

void popup_notify_dialog(const char *caption, const char *headline, const char *lines) {
  // qCDebug(FC) << "TODO: popup_notify_dialog";
  QStringList s{caption, headline, lines};
  KV::Application::UpdateReport(s);
}

void popup_connect_msg(const char *headline, const char *message) {
  qCDebug(FC) << "TODO: popup_connect_msg";
}

void popup_races_dialog(struct player *pplayer) {
  qCDebug(FC) << "TODO: popup_races_dialog";
}

void popdown_races_dialog() {
  qCDebug(FC) << "popdown_races_dialog";
  KV::Application::PopdownNationDialog();
}

void unit_select_dialog_popup(tile *ptile) {
  // qCDebug(FC) << "unit_select_dialog_popup";
  KV::Application::UnitSelectDialog(ptile);
}

void unit_select_dialog_update_real(void */*unused*/) {
  // qCDebug(FC) << "unit_select_dialog_update_real";
  KV::Application::UpdateUnitSelector();
}

void races_toggles_set_sensitive() {
  qCDebug(FC) << "races_toggles_set_sensitive";
  KV::Application::RefreshNationDialog(false);
}

void races_update_pickable(bool nationset_change) {
  qCDebug(FC) << "races_update_pickable";
  KV::Application::RefreshNationDialog(nationset_change);
}

void show_img_play_snd(const char *img_path, const char *snd_path, const char *desc, bool fullsize) {
  qCDebug(FC) << "TODO: show_img_play_snd";
}

void popup_action_selection(unit *actor,
                            city *target_city,
                            unit *target_unit,
                            tile *target_tile,
                            extra_type *target_extra,
                            const act_prob *act_probs) {
  qCDebug(FC) << "popup_action_selection";
  KV::ActionSelector::Popup(actor,
                            target_city,
                            target_unit,
                            target_tile,
                            target_extra,
                            act_probs);
}

int action_selection_actor_unit() {
  // qCDebug(FC) << "action_selection_actor_unit";
  return KV::ActionSelector::ActorId();
}

int action_selection_target_city() {
  qCDebug(FC) << "action_selection_target_city";
  return KV::ActionSelector::TargetCityId();
}

int action_selection_target_unit() {
  qCDebug(FC) << "action_selection_target_unit";
  return KV::ActionSelector::TargetUnitId();
}

int action_selection_target_tile() {
  qCDebug(FC) << "action_selection_target_tile";
  return KV::ActionSelector::TargetTileId();
}

int action_selection_target_extra() {
  qCDebug(FC) << "action_selection_target_extra";
  return KV::ActionSelector::TargetExtraId();
}

void action_selection_close() {
  qCDebug(FC) << "action_selection_close";
  KV::ActionSelector::Close();
}

void action_selection_refresh(unit *actor,
                              city *target_city,
                              unit *target_unit,
                              tile *target_tile,
                              extra_type *target_extra,
                              const act_prob *act_probs) {
  qCDebug(FC) << "action_selection_refresh";
  KV::ActionSelector::Refresh(actor,
                              target_city,
                              target_unit,
                              target_tile,
                              target_extra,
                              act_probs);
}

void action_selection_no_longer_in_progress_gui_specific(int actor_unit_id) {
  qCDebug(FC) << "TODO: action_selection_no_longer_in_progress_gui_specific";
}

void popup_incite_dialog(struct unit *actor, struct city *pcity, int cost, const struct action *paction) {
  qCDebug(FC) << "TODO: popup_incite_dialog";
}

void popup_bribe_dialog(struct unit *actor, struct unit *punit, int cost, const struct action *paction) {
  qCDebug(FC) << "TODO: popup_bribe_dialog";
}

void popup_sabotage_dialog(struct unit *actor, struct city *pcity, const struct action *paction) {
  qCDebug(FC) << "TODO: popup_sabotage_dialog";
}

void popup_pillage_dialog(struct unit *punit, bv_extras extras) {
  qCDebug(FC) << "TODO: popup_pillage_dialog";
}

void popup_upgrade_dialog(struct unit_list *punits) {
  qCDebug(FC) << "TODO: popup_upgrade_dialog";
}

void popup_disband_dialog(struct unit_list *punits) {
  qCDebug(FC) << "TODO: popup_disband_dialog";
}

void popup_tileset_suggestion_dialog() {
  qCDebug(FC) << "TODO: popup_tileset_suggestion_dialog";
}

void popup_soundset_suggestion_dialog() {
  qCDebug(FC) << "TODO: popup_soundset_suggestion_dialog";
}

void popup_musicset_suggestion_dialog() {
  qCDebug(FC) << "TODO: popup_musicset_suggestion_dialog";
}

bool popup_theme_suggestion_dialog(const char *theme_name) {
  qCDebug(FC) << "TODO: popup_theme_suggestion_dialog";
  return false;
}

void show_tech_gained_dialog(Tech_type_id tech) {
  qCDebug(FC) << "TODO: show_tech_gained_dialog";
}

void show_tileset_error(const char *msg) {
  qCDebug(FC) << "TODO: show_tileset_error";
}

void popdown_all_game_dialogs() {
  qCDebug(FC) << "popdown_all_game_dialogs: dummy";
}

