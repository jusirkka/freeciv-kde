#include "unitactionchecker.h"

#include "unitlist.h"
#include "unit.h"
#include "tile.h"
#include "actions.h"
#include "climisc.h"

using namespace KV;

void BuildCityChecker::check(unit_list *us, QAction *a) {

  a->setEnabled(can_units_do(us, unit_can_add_or_build_city));

  if (!a->isEnabled()) {
    a->setText(_("Build City"));
    return;
  }

  bool city_on_tile = false;
  unit_list_iterate(us, u) {
    if (tile_city(unit_tile(u))) {
      city_on_tile = true;
      break;
    }
  } unit_list_iterate_end;

  if (city_on_tile
      && units_can_do_action(us, ACTION_JOIN_CITY, true)) {
    a->setText(action_id_name_translation(ACTION_JOIN_CITY));
  } else {
    a->setText(action_id_name_translation(ACTION_FOUND_CITY));
  }
}


void AutoWorkerChecker::check(unit_list *us, QAction *a) {
  a->setEnabled(can_units_do(us, can_unit_do_autosettlers));
  if (units_contain_cityfounder(us)) {
    a->setText(_("Auto Settler"));
  } else {
    a->setText(_("Auto Worker"));
  }
}


void RoadChecker::check(unit_list *us, QAction *a) {
  a->setEnabled(can_units_do_any_road(us));

  a->setText(_("Build Road"));
  if (!a->isEnabled()) return;

  extra_type* e = nullptr;
  unit_list_iterate(us, u) {
    e = next_extra_for_tile(unit_tile(u), EC_ROAD, unit_owner(u), u);
    if (e != nullptr) break;
  } unit_list_iterate_end;

  if (e != nullptr) {
    a->setText(QString("Build %1").arg(extra_name_translation(e)));
  }
}

/**********************************************************************//**
  Return the text for the tile, changed by the activity.
  Should only be called for irrigation, mining, or transformation, and
  only when the activity changes the base terrain type.
**************************************************************************/
static const char *get_tile_change_menu_text(tile *ptile,
                                             unit_activity activity)
{
  tile *newtile = tile_virtual_new(ptile);
  tile_apply_activity(newtile, activity, nullptr);

  const char* text = tile_get_info_text(newtile, false, 0);
  tile_virtual_destroy(newtile);
  return text;
}

void IrrigateChecker::check(unit_list *us, QAction *a) {
  a->setEnabled(can_units_do_activity(us, ACTIVITY_IRRIGATE));

  a->setText(_("Irrigate"));
  if (!a->isEnabled()) return;

  bool units_all_same_tile = true;
  tile* prev_tile = nullptr;
  unit_list_iterate(us, u) {
    if (unit_tile(u) != prev_tile && prev_tile != nullptr) {
      units_all_same_tile = false;
      break;
    }
    prev_tile = unit_tile(u);
  } unit_list_iterate_end;

  if (!units_all_same_tile) return;

  tile* tl = unit_tile(unit_list_get(us, 0));
  terrain* tr = tile_terrain(tl);
  if (tr->irrigation_result != T_NONE && tr->irrigation_result != tr) {
    /* TRANS: Transform terrain to specific type */
    a->setText(QString(_("Transform to %1"))
               .arg(get_tile_change_menu_text(tl, ACTIVITY_IRRIGATE)));
  } else if (units_have_type_flag(us, UTYF_SETTLERS, true)){
    extra_type* e = nullptr;
    // FIXME: this overloading doesn't work well with multiple focus units.
    unit_list_iterate(us, u) {
      e = next_extra_for_tile(unit_tile(u), EC_IRRIGATION, unit_owner(u), u);
      if (e != nullptr) break;
    } unit_list_iterate_end;

    if (e != nullptr) {
      /* TRANS: Build irrigation of specific type */
      a->setText(QString(_("Build %1")).arg(extra_name_translation(e)));
    }
  }
}

void MineChecker::check(unit_list *us, QAction *a) {
  a->setEnabled(can_units_do_activity(us, ACTIVITY_MINE));

  a->setText(_("Mine"));
  if (!a->isEnabled()) return;

  bool units_all_same_tile = true;
  tile* prev_tile = nullptr;
  unit_list_iterate(us, u) {
    if (unit_tile(u) != prev_tile && prev_tile != nullptr) {
      units_all_same_tile = false;
      break;
    }
    prev_tile = unit_tile(u);
  } unit_list_iterate_end;

  if (!units_all_same_tile) return;

  tile* tl = unit_tile(unit_list_get(us, 0));
  terrain* tr = tile_terrain(tl);

  if (tr->mining_result != T_NONE && tr->mining_result != tr) {
    /* TRANS: Transfrom terrain to specific type */
    a->setText(QString(_("Transform to %1"))
               .arg(get_tile_change_menu_text(tl, ACTIVITY_MINE)));
  } else if (units_have_type_flag(us, UTYF_SETTLERS, true)){
    extra_type* e = nullptr;
    // FIXME: this overloading doesn't work well with multiple focus units.
    unit_list_iterate(us, u) {
      e = next_extra_for_tile(unit_tile(u), EC_MINE, unit_owner(u), u);
      if (e != nullptr) break;
    } unit_list_iterate_end;

    if (e != nullptr) {
      /* TRANS: Build mine of specific type */
      a->setText(QString(_("Build %1")).arg(extra_name_translation(e)));
    }
  }
}

void TransformChecker::check(unit_list *us, QAction *a) {
  a->setEnabled(can_units_do_activity(us, ACTIVITY_TRANSFORM));

  a->setText(_("Transform"));
  if (!a->isEnabled()) return;

  bool units_all_same_tile = true;
  tile* prev_tile = nullptr;
  unit_list_iterate(us, u) {
    if (unit_tile(u) != prev_tile && prev_tile != nullptr) {
      units_all_same_tile = false;
      break;
    }
    prev_tile = unit_tile(u);
  } unit_list_iterate_end;

  if (!units_all_same_tile) return;

  tile* tl = unit_tile(unit_list_get(us, 0));
  terrain* tr = tile_terrain(tl);

  if (tr->transform_result != T_NONE && tr->transform_result != tr) {
    /* TRANS: Transfrom terrain to specific type */
    a->setText(QString(_("Transform to %1"))
               .arg(get_tile_change_menu_text(tl, ACTIVITY_TRANSFORM)));
  }
}



void ConnectRoadChecker::check(unit_list *us, QAction *a) {
  auto e = road_extra_get(road_by_compat_special(ROCO_ROAD));
  a->setEnabled(can_units_do_connect(us, ACTIVITY_GEN_ROAD, e));
}

void ConnectRailChecker::check(unit_list *us, QAction *a) {
  auto e = road_extra_get(road_by_compat_special(ROCO_RAILROAD));
  a->setEnabled(can_units_do_connect(us, ACTIVITY_GEN_ROAD, e));
}

void ConnectIrrigationChecker::check(unit_list *us, QAction *a) {
  bool cando = false;
  extra_type_by_cause_iterate(EC_IRRIGATION, e) {
    cando = can_units_do_connect(us, ACTIVITY_IRRIGATE, e);
    if (cando) break;
  } extra_type_by_cause_iterate_end;

  a->setEnabled(cando);
}


