extern "C" {
#include "dialogs_g.h"
}
#include "text.h"
#include "control.h"

#include "logging.h"
#include "application.h"
#include "actionselector.h"
#include "messagebox.h"
#include <QApplication>


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
  // qCDebug(FC) << "popdown_races_dialog";
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
  // qCDebug(FC) << "races_toggles_set_sensitive";
  KV::Application::RefreshNationDialog(false);
}

void races_update_pickable(bool nationset_change) {
  // qCDebug(FC) << "races_update_pickable";
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
  // qCDebug(FC) << "popup_action_selection";
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
  // qCDebug(FC) << "action_selection_target_city";
  return KV::ActionSelector::TargetCityId();
}

int action_selection_target_unit() {
  // qCDebug(FC) << "action_selection_target_unit";
  return KV::ActionSelector::TargetUnitId();
}

int action_selection_target_tile() {
  // qCDebug(FC) << "action_selection_target_tile";
  return KV::ActionSelector::TargetTileId();
}

int action_selection_target_extra() {
  // qCDebug(FC) << "action_selection_target_extra";
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
  KV::ActionSelector::Finalize(actor_unit_id);
}

void popup_incite_dialog(unit *actor, city *c, int cost, const action *act) {
  KV::ActionSelector::InciteDialog(actor, c, cost, act);
}

void popup_bribe_dialog(unit *actor, unit *u, int cost, const action *act) {
  KV::ActionSelector::BribeDialog(actor, u, cost, act);
}

void popup_sabotage_dialog(unit *actor, city *c, const action *act) {
  KV::ActionSelector::SabotageDialog(actor, c, act);
}

void popup_pillage_dialog(unit *u, bv_extras e) {
  KV::ActionSelector::PillageDialog(u, e);
}

void popup_upgrade_dialog(struct unit_list *punits) {
  if (punits == nullptr || unit_list_size(punits) == 0) return;

  char buf[1024];
  bool canDo = get_units_upgrade_info(buf, sizeof(buf), punits);
  QString title = canDo ? _("Upgrade Obsolete Units") : _("Upgrade Unit!");
  KV::StandardMessageBox ask(qApp->topLevelAt(QCursor::pos()), buf, title);
  if (!canDo) ask.setStandardButtons(QMessageBox::Ok);
  if (ask.exec() == QMessageBox::Ok) {
    unit_list_iterate(punits, punit) {
      request_unit_upgrade(punit);
    }
    unit_list_iterate_end;
  }
}

void popup_disband_dialog(struct unit_list *punits) {
  if (punits == nullptr || unit_list_size(punits) == 0) return;

  QString title = _("Disband Units");
  QString text = QString(PL_("Are you sure you want to disband %1 unit?",
                             "Are you sure you want to disband %1 units?",
                             unit_list_size(punits))).arg(unit_list_size(punits));
  KV::StandardMessageBox ask(qApp->topLevelAt(QCursor::pos()), text, title);
  if (ask.exec() == QMessageBox::Ok) {
    unit_list_iterate(punits, punit) {
      if (unit_can_do_action(punit, ACTION_DISBAND_UNIT)) {
        request_unit_disband(punit);
      }
    }
    unit_list_iterate_end;
  }
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
  qCDebug(FC) << "TODO: popup_theme_suggestion_dialog" << theme_name;
  return false;
}

void show_tech_gained_dialog(Tech_type_id /*tech*/) {
  // qCDebug(FC) << "TODO: show_tech_gained_dialog";
}

void show_tileset_error(const char *msg) {
  qCDebug(FC) << "TODO: show_tileset_error" << msg;
}

void popdown_all_game_dialogs() {
  // qCDebug(FC) << "popdown_all_game_dialogs: dummy";
}

