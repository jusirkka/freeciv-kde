#include "actionselector.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

#include "actions.h"
#include "city.h"
#include "unit.h"
#include "tile.h"
#include "game.h"
#include "control.h"

using namespace KV;

// ActionDialog implementation
ActionDialog::ActionDialog(const QString& title, const QString& header, QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle(title);
  setAttribute(Qt::WA_DeleteOnClose);
  auto mainLayout = new QVBoxLayout;

  m_header = new QLabel(header);
  mainLayout->addWidget(m_header);

  m_buttonLayout = new QVBoxLayout;
  auto buttons = new QWidget;
  buttons->setLayout(m_buttonLayout);
  mainLayout->addWidget(buttons);

  auto box = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal);
  connect(box, &QDialogButtonBox::rejected, this, &ActionDialog::reject);
  mainLayout->addWidget(box);

  setLayout(mainLayout);
}

void ActionDialog::setTitleAndHeader(const QString& title, const QString& header) {
  setWindowTitle(title);
  m_header->setText(header);
}

void ActionDialog::addButton(int id, int actor, int target, const act_prob& prob) {

  QString text = action_prepare_ui_name(id, "&", prob, nullptr);
  auto but = new QPushButton(text);
  but->setToolTip(action_get_tool_tip(id, prob));
  connect(but, &QPushButton::clicked, this, [this, id, actor, target] () {
    request_do_action(id, actor, target, 0, "");
    close();
  });

  if (!m_actions.contains(id)) {
    m_actions[id] = m_buttonLayout->count();
    m_buttonLayout->addWidget(but);
  } else {
    delete m_buttonLayout->takeAt(m_actions[id])->widget();
    m_buttonLayout->insertWidget(m_actions[id], but);
  }
}






// ActionSelector implementation

void ActionSelector::Popup(unit *actor,  city *t_city, unit *t_unit,
                           tile *t_tile, extra_type *t_extra,
                           const act_prob *act_probs) {
  instance()->createDialog(actor, t_city, t_unit, t_tile, t_extra,
                           act_probs);
}

void ActionSelector::Refresh(unit *actor, city *t_city, unit *t_unit,
                             tile *t_tile, extra_type *t_extra,
                             const act_prob *act_probs) {
  instance()->refreshDialog(actor, t_city, t_unit, t_tile, t_extra,
                            act_probs);
}

void ActionSelector::Close() {
  if (instance()->m_dialog == nullptr) return;
  instance()->m_dialog->done(QDialog::Accepted);
}

int ActionSelector::ActorId() {
  return instance()->m_actor;
}

int ActionSelector::TargetCityId() {
  return instance()->m_targetCity;
}

int ActionSelector::TargetUnitId() {
  return instance()->m_targetUnit;
}

int ActionSelector::TargetTileId() {
  return instance()->m_targetTile;
}

int ActionSelector::TargetExtraId() {
  return instance()->m_targetExtra;
}

ActionSelector* ActionSelector::instance() {
  static ActionSelector* self = new ActionSelector;
  return self;
}

ActionSelector::ActionSelector(QObject *parent)
  : QObject(parent)
  , m_actor(IDENTITY_NUMBER_ZERO)
  , m_targetCity(IDENTITY_NUMBER_ZERO)
  , m_targetUnit(IDENTITY_NUMBER_ZERO)
  , m_targetTile(TILE_INDEX_NONE)
  , m_targetExtra(EXTRA_NONE)
{}


void ActionSelector::createDialog(unit *actor,
                                  city *t_city,
                                  unit *t_unit,
                                  tile *t_tile,
                                  extra_type *t_extra,
                                  const act_prob *act_probs) {

  m_actor = actor != nullptr ? actor->id : IDENTITY_NUMBER_ZERO;
  m_targetCity = t_city != nullptr ? t_city->id : IDENTITY_NUMBER_ZERO;
  m_targetUnit = t_unit != nullptr ? t_unit->id : IDENTITY_NUMBER_ZERO;
  m_targetTile = t_tile != nullptr ? tile_index(t_tile) : TILE_INDEX_NONE;
  m_targetExtra = t_extra != nullptr ? extra_number(t_extra) : EXTRA_NONE;

  if (m_actor == IDENTITY_NUMBER_ZERO) return;

  QString name = unit_name_translation(actor);
  auto title = QString(_("Choose Your %1's Strategy"))
      .arg(name);

  QString header;
  auto home = game_city_by_number(actor->homecity);
  if (home != nullptr) {
    name = QString("%1 from %2").arg(name, city_name_get(home));
  }
  if (t_city != nullptr) {
    header = QString(_("Your %1 reaches the city of %2.\nWhat now?"))
        .arg(name)
        .arg(city_name_get(t_city));
  } else if (t_unit != nullptr) {
    header = QString(_("Your %1 is ready to act against %2 %3."))
        .arg(name)
        .arg(nation_adjective_for_player(unit_owner(t_unit)))
        .arg(unit_name_translation(t_unit));
  } else if (t_tile != nullptr) {
    header = QString(_("Your %1 is waiting for your command."))
        .arg(name);
  }

  if (m_dialog) {
    m_dialog->done(QDialog::Rejected);
  }
  m_dialog = new ActionDialog(title, header);
  connect(m_dialog, &ActionDialog::finished, this, &ActionSelector::reset);

  action_iterate(aid) {
    if (action_id_get_actor_kind(aid) != AAK_UNIT) continue;
    if (!action_prob_possible(act_probs[aid])) continue;

    if (action_id_get_target_kind(aid) == ATK_CITY) {
      m_dialog->addButton(aid, m_actor, m_targetCity, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNIT) {
      m_dialog->addButton(aid, m_actor, m_targetUnit, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNITS) {
      m_dialog->addButton(aid, m_actor, m_targetTile, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_TILE) {
      m_dialog->addButton(aid, m_actor, m_targetTile, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_SELF) {
      m_dialog->addButton(aid, m_actor, m_actor, act_probs[aid]);
    }
  } action_iterate_end;

  m_dialog->show();
}

void ActionSelector::refreshDialog(unit *actor,
                                   city *t_city,
                                   unit *t_unit,
                                   tile *t_tile,
                                   extra_type *t_extra,
                                   const act_prob *act_probs) {

  if (m_dialog == nullptr) return;
  if (m_actor == IDENTITY_NUMBER_ZERO) return;
  if (m_actor != actor->id) return;

  m_targetCity = t_city != nullptr ? t_city->id : IDENTITY_NUMBER_ZERO;
  m_targetUnit = t_unit != nullptr ? t_unit->id : IDENTITY_NUMBER_ZERO;
  m_targetTile = t_tile != nullptr ? tile_index(t_tile) : TILE_INDEX_NONE;
  m_targetExtra = t_extra != nullptr ? extra_number(t_extra) : EXTRA_NONE;


  QString name = unit_name_translation(actor);
  auto title = QString(_("Choose Your %1's Strategy"))
      .arg(name);

  QString header;
  auto home = game_city_by_number(actor->homecity);
  if (home != nullptr) {
    name = QString("%1 from %2").arg(name, city_name_get(home));
  }
  if (t_city != nullptr) {
    header = QString(_("Your %1 reaches the city of %2.\nWhat now?"))
        .arg(name)
        .arg(city_name_get(t_city));
  } else if (t_unit != nullptr) {
    header = QString(_("Your %1 is ready to act against %2 %3."))
        .arg(name)
        .arg(nation_adjective_for_player(unit_owner(t_unit)))
        .arg(unit_name_translation(t_unit));
  } else if (t_tile != nullptr) {
    header = QString(_("Your %1 is waiting for your command."))
        .arg(name);
  }

  m_dialog->setTitleAndHeader(title, header);

  action_iterate(aid) {
    if (action_id_get_actor_kind(aid) != AAK_UNIT) continue;
    if (!action_prob_possible(act_probs[aid])) continue;

    if (action_id_get_target_kind(aid) == ATK_CITY) {
      m_dialog->addButton(aid, m_actor, m_targetCity, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNIT) {
      m_dialog->addButton(aid, m_actor, m_targetUnit, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNITS) {
      m_dialog->addButton(aid, m_actor, m_targetTile, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_TILE) {
      m_dialog->addButton(aid, m_actor, m_targetTile, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_SELF) {
      m_dialog->addButton(aid, m_actor, m_actor, act_probs[aid]);
    }
  } action_iterate_end;

  m_dialog->raise();

}

void ActionSelector::reset() {

  auto old = m_actor;

  m_dialog = nullptr;
  m_actor = IDENTITY_NUMBER_ZERO;
  m_targetCity = IDENTITY_NUMBER_ZERO;
  m_targetUnit = IDENTITY_NUMBER_ZERO;
  m_targetTile = TILE_INDEX_NONE;
  m_targetExtra = EXTRA_NONE;

  action_selection_no_longer_in_progress(old);
  action_decision_clear_want(old);
  action_selection_next_in_focus(old);

}
