#include "treatydialog.h"
#include "ui_treatydialog.h"
#include "sprite.h"
#include <QListWidget>
#include <QAction>
#include <QMenu>
#include <QListWidgetItem>

#include "government.h"
#include "player.h"
#include "client_main.h"
#include "colors.h"
#include "tilespec.h"
#include "game.h"
#include "research.h"
#include "climisc.h"

using namespace KV;

TreatyDialog::TreatyDialog(player *away, QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::TreatyDialog)
  , m_away(away)
{
  m_ui->setupUi(this);

  auto me = client_player();

  auto sprite = get_nation_flag_sprite(get_tileset(), nation_of_player(me));
  if (sprite) {
    m_ui->homeFlagLabel->setPixmap(sprite->pm);
  } else {
    m_ui->homeFlagLabel->setText("FLAG MISSING");
  }

  sprite = get_nation_flag_sprite(get_tileset(), nation_of_player(m_away));
  if (sprite) {
    m_ui->awayFlagLabel->setPixmap(sprite->pm);
  } else {
    m_ui->awayFlagLabel->setText("FLAG MISSING");
  }

  color* textcolors[2] = {
    get_color(get_tileset(), COLOR_MAPVIEW_CITYTEXT),
    get_color(get_tileset(), COLOR_MAPVIEW_CITYTEXT_DARK)
  };

  auto c = get_player_color(get_tileset(), me);
  auto nation = QString("<style>h3{background-color: %1; color: %2}</style><b><h3>%3</h3></b>")
      .arg(c->qcolor.name())
      .arg(color_best_contrast(c, textcolors, ARRAY_SIZE(textcolors))->qcolor.name())
      .arg(nation_plural_for_player(me));
  m_ui->homeNationLabel->setText(nation);

  c = get_player_color(get_tileset(), m_away);
  nation = QString("<style>h3{background-color: %1; color: %2}</style><b><h3>%3</h3></b>")
      .arg(c->qcolor.name())
      .arg(color_best_contrast(c, textcolors, ARRAY_SIZE(textcolors))->qcolor.name())
      .arg(nation_plural_for_player(m_away));
  m_ui->awayNationLabel->setText(nation);

  auto state = player_diplstate_get(me, m_away);
  if (state->turns_left > 0) {
    auto tip = QString("%1 (%2)")
        .arg(diplstate_type_translated_name(state->type))
        .arg(QString(PL_("%1 turn left", "%1 turns left", state->turns_left)).arg(state->turns_left));
    m_ui->awayNationLabel->setToolTip(tip);
    m_ui->awayLeaderLabel->setToolTip(tip);
  }

  char buf[1024];
  auto leader = QString("<b><h3>%1</h3></b>").arg(ruler_title_for_player(me, buf, sizeof(buf)));
  m_ui->homeLeaderLabel->setText(leader);

  leader = QString("<b><h3>%1</h3></b>").arg(ruler_title_for_player(m_away, buf, sizeof(buf)));
  m_ui->awayLeaderLabel->setText(leader);

  if (game.info.trading_gold) {
    m_ui->homeGoldLine->setValidator(new QIntValidator(0, me->economic.gold));
    m_ui->awayGoldLine->setValidator(new QIntValidator(0, m_away->economic.gold));
  } else {
    m_ui->homeGoldLine->setEnabled(false);
    m_ui->awayGoldLine->setEnabled(false);
  }

  connect(m_ui->clauseList, &QListWidget::itemDoubleClicked,
          this, &TreatyDialog::removeItem);

  updateTreaty();
}

void TreatyDialog::removeItem(QListWidgetItem *item)
{
  int row = item->data(Qt::UserRole).toInt();
  const Clause& d = m_clauses.at(row);
  dsend_packet_diplomacy_remove_clause_req(&client.conn,
                                           away(),
                                           player_number(d.from),
                                           d.type, d.value);
}


void TreatyDialog::on_awayGoldLine_returnPressed()
{
  bool ok;
  int gold = m_ui->awayGoldLine->text().toInt(&ok);
  if (!ok) return;
  dsend_packet_diplomacy_create_clause_req(&client.conn, away(), away(), CLAUSE_GOLD, gold);
}

void TreatyDialog::on_homeGoldLine_returnPressed()
{
  bool ok;
  int gold = m_ui->homeGoldLine->text().toInt(&ok);
  if (!ok) return;
  int hid = player_number(client_player());
  dsend_packet_diplomacy_create_clause_req(&client.conn, away(), hid, CLAUSE_GOLD, gold);
}

void TreatyDialog::on_homeClauseButton_clicked() {
  popupClauses(client_player(), m_away);
}

void TreatyDialog::on_awayClauseButton_clicked() {
  popupClauses(m_away, client_player());
}

void TreatyDialog::popupClauses(player* giver, player* taker)
{
  auto gid = player_number(giver);

  QMenu menu;
  /* Maps */
  auto maps = menu.addMenu(_("Maps"));
  auto a = new QAction(_("Worldmap"));
  connect(a, &QAction::triggered, this, [=] () {
    dsend_packet_diplomacy_create_clause_req(&client.conn,
                                             away(),
                                             gid, CLAUSE_MAP, 0);
  });
  maps->addAction(a);
  a = new QAction(_("Seamap"));
  connect(a, &QAction::triggered, this, [=] () {
    dsend_packet_diplomacy_create_clause_req(&client.conn,
                                             away(),
                                             gid, CLAUSE_SEAMAP, 0);
  });
  maps->addAction(a);

  /* Trading: advances */
  if (game.info.trading_tech) {
    auto advances = menu.addMenu(_("Advances"));

    /* All advances */
    a = new QAction(_("All advances"));
    connect(a, &QAction::triggered, this, [=] () {
      allAdvances(giver, taker);
    });
    advances->addAction(a);
    advances->addSeparator();

    auto gres = research_get(giver);
    auto tres = research_get(taker);
    advance_iterate(A_FIRST, p) {
      Tech_type_id id = advance_number(p);

      if (research_invention_state(gres, id) == TECH_KNOWN
          && research_invention_gettable(tres, id, game.info.tech_trade_allow_holes)
          && (research_invention_state(tres, id) == TECH_UNKNOWN
              || research_invention_state(tres, id) == TECH_PREREQS_KNOWN)) {
        a = new QAction(advance_name_translation(p));
        connect(a, &QAction::triggered, this, [=] () {
          dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_ADVANCE, id);
        });
        advances->addAction(a);
      }
    } advance_iterate_end;
  }

  /* Trading: cities. */
  if (game.info.trading_city) {
    auto cities = menu.addMenu(_("Cities"));

    city_list_iterate(giver->cities, pcity) {
      if (is_capital(pcity)) continue;
      a = new QAction(pcity->name);
      connect(a, &QAction::triggered, this, [=] () {
        dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_CITY, pcity->id);
      });
      cities->addAction(a);
    } city_list_iterate_end;
  }

  a = new QAction(_("Give shared vision"));
  connect(a, &QAction::triggered, this, [=] () {
    dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_VISION, 0);
  });
  menu.addAction(a);

  if (gives_shared_vision(giver, taker)) {
    a->setDisabled(true);
  }

  a = new QAction(_("Give embassy"));
  connect(a, &QAction::triggered, this, [=] () {
    dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_EMBASSY, 0);
  });
  menu.addAction(a);
  a->setDisabled(player_has_real_embassy(taker, giver));

  /* Pacts */
  auto pacts = menu.addMenu(_("Pacts"));
  auto state = player_diplstate_get(giver, taker)->type;
  a = new QAction(Q_("?diplomatic_state:Cease-fire"));
  connect(a, &QAction::triggered, this, [=] () {
    dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_CEASEFIRE, 0);
  });
  pacts->addAction(a);
  a->setDisabled(state == DS_CEASEFIRE || state == DS_TEAM);

  a = new QAction(Q_("?diplomatic_state:Peace"));
  connect(a, &QAction::triggered, this, [=] () {
    dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_PEACE, 0);
  });
  pacts->addAction(a);
  a->setDisabled(state == DS_PEACE || state == DS_TEAM);

  a = new QAction(Q_("?diplomatic_state:Alliance"));
  connect(a, &QAction::triggered, this, [=] () {
    dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_ALLIANCE, 0);
  });
  pacts->addAction(a);
  a->setDisabled(state == DS_ALLIANCE || state == DS_TEAM);

  menu.exec(QCursor::pos());
}

void TreatyDialog::allAdvances(player* giver, player* taker) {

  auto gres = research_get(giver);
  auto tres = research_get(taker);

  auto gid = player_number(giver);

  advance_iterate(A_FIRST, p) {
    Tech_type_id id = advance_number(p);

    if (research_invention_state(gres, id) == TECH_KNOWN
        && research_invention_gettable(tres, id, game.info.tech_trade_allow_holes)
        && (research_invention_state(tres, id) == TECH_UNKNOWN
            || research_invention_state(tres, id) == TECH_PREREQS_KNOWN)) {
      dsend_packet_diplomacy_create_clause_req(&client.conn, away(), gid, CLAUSE_ADVANCE, id);
    }
  } advance_iterate_end;
}

static bool clausesEqual(const Clause& c1, const Clause& c2) {
  return c1.type == c2.type && c1.from == c2.from && c1.value == c2.value;
}

void TreatyDialog::removeClause(const Clause& clause) {
  int found = -1;
  for (int i = 0; i < m_clauses.count(); i++) {
    if (clausesEqual(m_clauses[i], clause)) {
      found = i;
      break;
    }
  }
  if (found < 0) return;
  m_clauses.remove(found);
  updateTreaty();
}

void TreatyDialog::createClause(const Clause& clause) {
  int found = -1;
  for (int i = 0; i < m_clauses.count(); i++) {
    if (clausesEqual(m_clauses[i], clause)) {
      found = i;
      break;
    }
  }
  if (found > 0) return;
  m_clauses.append(clause);
  updateTreaty();
}

void TreatyDialog::awayResolution(bool accept) {
  m_awayAccepts = accept;
  updateTreaty();
}

void TreatyDialog::updateTreaty() {
  m_ui->clauseList->clear();
  int idx = 0;
  for (auto c: m_clauses) {
    char buf[128];
    client_diplomacy_clause_string(buf, sizeof(buf), &c);
    auto item = new QListWidgetItem;
    item->setText(buf);
    item->setData(Qt::UserRole, idx);
    m_ui->clauseList->addItem(item);
    idx++;
  }

  if (m_clauses.isEmpty()) {
    m_ui->clauseList->addItem(_("--- This treaty is blank. "
                                "Please add some clauses. ---"));
  }

  auto pix = get_treaty_thumb_sprite(get_tileset(), m_awayAccepts);
  if (pix) {
    m_ui->acceptLabel->setPixmap(pix->pm);
  } else {
    m_ui->acceptLabel->setText("PIXMAP MISSING");
  }
}

void TreatyDialog::on_acceptButton_clicked() {
  dsend_packet_diplomacy_accept_treaty_req(&client.conn, away());
}

void TreatyDialog::on_cancelButton_clicked() {
  dsend_packet_diplomacy_cancel_meeting_req(&client.conn, away());
}

int TreatyDialog::away() const {
  return player_number(m_away);
}

TreatyDialog::~TreatyDialog() {
  delete m_ui;
}
