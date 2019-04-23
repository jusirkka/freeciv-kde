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
#include "client_main.h"
#include "research.h"

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

void ActionDialog::addButton(int id, int actor, int target, int extra, const act_prob& prob) {

  QString text = action_prepare_ui_name(id, "&", prob, nullptr);
  auto but = new QPushButton(text);
  but->setToolTip(action_get_tool_tip(id, prob));
  but->setEnabled(action_prob_possible(prob));
  auto exe = ActionExec::Create(id, this);
  connect(exe, &ActionExec::inProgress, this, &ActionDialog::inProgress);
  connect(exe, &ActionExec::stealTech, this, &ActionDialog::stealTech);
  connect(but, &QPushButton::clicked, this, [this, exe, actor, target, extra] () {
    exe->func(actor, target, extra);
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

void ActionSelector::Finalize(int actor) {
  instance()->m_inProgressActors.removeAll(actor);
}

QMap<int, int> ActionSelector::m_targetedActionMap{
  {ACTION_SPY_SABOTAGE_CITY, ACTION_SPY_TARGETED_SABOTAGE_CITY},
  {ACTION_SPY_SABOTAGE_CITY_ESC, ACTION_SPY_TARGETED_SABOTAGE_CITY_ESC},
  {ACTION_SPY_STEAL_TECH, ACTION_SPY_TARGETED_STEAL_TECH},
  {ACTION_SPY_STEAL_TECH_ESC, ACTION_SPY_TARGETED_STEAL_TECH_ESC}
};

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
  connect(m_dialog, &ActionDialog::inProgress, this, [this] (int actor) {
    if (!m_inProgressActors.contains(actor)) {
      m_inProgressActors.append(actor);
    }
  });
  action_iterate(aid) {
    if (action_id_get_actor_kind(aid) != AAK_UNIT) continue;
    if (!action_prob_possible(act_probs[aid])) continue;

    if (m_targetedActionMap.contains(aid)) {
      if (action_prob_possible(act_probs[m_targetedActionMap[aid]])) {
        // untargeted version handled in the targeted popup ("at spy's discretion")
        continue;
      }
    }

    if (action_id_get_target_kind(aid) == ATK_CITY) {
      m_dialog->addButton(aid, m_actor, m_targetCity, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNIT) {
      m_dialog->addButton(aid, m_actor, m_targetUnit, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNITS) {
      m_dialog->addButton(aid, m_actor, m_targetTile, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_TILE) {
      m_dialog->addButton(aid, m_actor, m_targetTile, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_SELF) {
      m_dialog->addButton(aid, m_actor, m_actor, m_targetExtra, act_probs[aid]);
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

    if (action_id_get_target_kind(aid) == ATK_CITY) {
      m_dialog->addButton(aid, m_actor, m_targetCity, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNIT) {
      m_dialog->addButton(aid, m_actor, m_targetUnit, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_UNITS) {
      m_dialog->addButton(aid, m_actor, m_targetTile, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_TILE) {
      m_dialog->addButton(aid, m_actor, m_targetTile, m_targetExtra, act_probs[aid]);
    } else if (action_id_get_target_kind(aid) == ATK_SELF) {
      m_dialog->addButton(aid, m_actor, m_actor, m_targetExtra, act_probs[aid]);
    }
  } action_iterate_end;

  m_dialog->raise();
}

void ActionSelector::buildStealTechDialog(int id, int actor, int target) {
  auto c = game_city_by_number(target);
  player* victim = nullptr;
  if (c != nullptr)  {
    victim = city_owner(c);
  }
  if (victim == nullptr) {
    finalAct(actor);
    return;
  }
  auto d = new QDialog;
  d->setAttribute(Qt::WA_DeleteOnClose);
  d->setWindowTitle(_("Steal"));
  auto lay = new QVBoxLayout;
  d->setLayout(lay);
  lay->addWidget(new QLabel(_("Steal Technology")));

  auto pr = research_get(client_player());
  auto vr = research_get(victim);

  advance_index_iterate(A_FIRST, i) {
    if (research_invention_gettable(pr, i, game.info.tech_steal_allow_holes)
        && research_invention_state(vr, i) == TECH_KNOWN
        && research_invention_state(pr, i) != TECH_KNOWN) {
      auto but = new QPushButton(research_advance_name_translation(pr, i));
      connect(but, &QPushButton::clicked, this, [d, id, actor, target, i] () {
        request_do_action(id, actor, target, i, "");
        d->close();
      });
      lay->addWidget(but);
    }
  } advance_index_iterate_end;

  int n = m_targetedActionMap.key(id, -1);
  if (n != -1) {
    auto u = game_unit_by_number(actor);
    if (action_prob_possible(u->client.act_prob_cache[n])) {
      auto but = new QPushButton(QString(_("At %1's Discretion")).arg(unit_name_translation(u)));
      connect(but, &QPushButton::clicked, this, [d, n, actor, target] () {
        request_do_action(n, actor, target, 0, "");
        d->close();
      });
      lay->addWidget(but);
    }
  }

  auto box = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal);
  connect(box, &QDialogButtonBox::rejected, d, &QDialog::reject);

  connect(d, &QDialog::finished, this, [this, actor] () {
    finalAct(actor);
  });

  d->show();
}


void ActionSelector::finalAct(int actor) {
  m_inProgressActors.removeAll(actor);
  if (actor == m_actor) {
    reset();
  } else {
    action_selection_no_longer_in_progress(actor);
    action_decision_clear_want(actor);
  }
}

void ActionSelector::reset() {

  auto old = m_actor;

  // in progress - do not change state
  if (m_inProgressActors.contains(old)) return;

  m_dialog = nullptr;
  m_actor = IDENTITY_NUMBER_ZERO;
  m_targetCity = IDENTITY_NUMBER_ZERO;
  m_targetUnit = IDENTITY_NUMBER_ZERO;
  m_targetTile = TILE_INDEX_NONE;
  m_targetExtra = EXTRA_NONE;


  auto u = game_unit_by_number(old);
  if (u != nullptr) {
    // The case of action selection dialog wasn't closed
    // because the actor unit was lost.
    if (u->client.act_prob_cache != nullptr) {
      FC_FREE(u->client.act_prob_cache);
    }
  }
  action_selection_no_longer_in_progress(old);
  action_decision_clear_want(old);
  action_selection_next_in_focus(old);
}


// ActionExec implementation

ActionExec* ActionExec::Create(int id, QObject *parent) {
  switch (id) {
  case ACTION_SPY_INCITE_CITY:
  case ACTION_SPY_INCITE_CITY_ESC:
  case ACTION_SPY_TARGETED_SABOTAGE_CITY:
  case ACTION_SPY_TARGETED_SABOTAGE_CITY_ESC:
  case ACTION_SPY_BRIBE_UNIT:
    return new RequestActionDetails(id, parent);
  case ACTION_SPY_TARGETED_STEAL_TECH:
  case ACTION_SPY_TARGETED_STEAL_TECH_ESC:
    return new BuildStealTechDialog(id, parent);
  case ACTION_FOUND_CITY:
    return new SuggestCityName(id, parent);
  case ACTION_PILLAGE:
  case ACTION_ROAD:
  case ACTION_BASE:
  case ACTION_MINE:
  case ACTION_IRRIGATE:
    return new RequestDoActionWithExtra(id, parent);
  default:
    return new RequestDoAction(id, parent);
  }
}

ActionExec::ActionExec(int id, QObject *parent)
  : QObject(parent)
  , m_actionId(id) {}

RequestDoAction::RequestDoAction(int id, QObject *parent)
  : ActionExec(id, parent) {}

void RequestDoAction::func(int actor, int target, int /*extra*/) const {
  request_do_action(m_actionId, actor, target, 0, "");
}

RequestDoActionWithExtra::RequestDoActionWithExtra(int id, QObject *parent)
  : ActionExec(id, parent) {}

void RequestDoActionWithExtra::func(int actor, int target, int extra) const {
  request_do_action(m_actionId, actor, target, extra, "");
}

RequestActionDetails::RequestActionDetails(int id, QObject *parent)
  : ActionExec(id, parent) {}

void RequestActionDetails::func(int actor, int target, int /*extra*/) const {
  request_action_details(m_actionId, actor, target);
  emit inProgress(actor);
}

SuggestCityName::SuggestCityName(int id, QObject *parent)
  : ActionExec(id, parent) {}

void SuggestCityName::func(int actor, int /*target*/, int /*extra*/) const {
  dsend_packet_city_name_suggestion_req(&client.conn, actor);
}

BuildStealTechDialog::BuildStealTechDialog(int id, QObject *parent)
  : ActionExec(id, parent) {}

void BuildStealTechDialog::func(int actor, int target, int /*extra*/) const {
  emit inProgress(actor);
  emit stealTech(m_actionId, actor, target);
}


