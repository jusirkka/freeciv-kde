#include "playerdialog.h"
#include "ui_playerdialog.h"
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QStringList>
#include "application.h"
#include "logging.h"

#include "plrdlg_common.h"
#include "research.h"
#include "client_main.h"
#include "tilespec.h"
#include "sprite.h"
#include "colors_common.h"
#include "colors.h"
#include "government.h"

using namespace KV;

PlayerDialog::PlayerDialog(QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::PlayerDialog)
  , m_players(new PlayerModel(this))
  , m_filter(new QSortFilterProxyModel(this))
{
  m_ui->setupUi(this);

  m_filter->setDynamicSortFilter(true);
  m_filter->setSourceModel(m_players);
  m_filter->setFilterRole(Qt::DisplayRole);
  m_ui->playerView->setModel(m_filter);
  m_ui->playerView->setAllColumnsShowFocus(true);
  m_ui->playerView->header()->setContextMenuPolicy(Qt::CustomContextMenu);


  connect(m_ui->playerView->header(), &QWidget::customContextMenuRequested,
          this, &PlayerDialog::popupHeaderMenu);
  connect(m_ui->playerView->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &PlayerDialog::playerSelected);

  updatePlayers();

  connect(Application::instance(), &Application::updatePlayers,
          this, &PlayerDialog::updatePlayers);

}


void PlayerDialog::updatePlayers() {
  auto curr = m_ui->playerView->selectionModel()->selection();
  player* s = nullptr;
  if (!curr.isEmpty()) {
    auto i = curr.first().indexes().first();
    s = reinterpret_cast<player*>(i.data(Qt::UserRole).value<void*>());
  }

  m_players->reset();

  if (s) {
    int cnt = m_filter->rowCount();
    for (int row = 0; row < cnt; row++) {
      auto d = m_filter->index(row, 0).data(Qt::UserRole);
      if (d.isNull()) continue;
      auto p = reinterpret_cast<player*>(d.value<void*>());
      if (s == p) {
        m_ui->playerView->selectionModel()->select(
              m_filter->index(row, 0),
              QItemSelectionModel::Rows|QItemSelectionModel::Select);
        break;
      }
    }
  } else {
    playerSelected(QItemSelection(), QItemSelection());
  }

}

void PlayerDialog::popupHeaderMenu(const QPoint &p)
{
  QMenu menu(this);
  menu.setTitle(_("Column visibility"));
  int n = m_ui->playerView->header()->count();
  for (int i = 0; i < n; ++i) {
    QAction *a = menu.addAction(m_players->headerData(i, Qt::Horizontal).toString());
    a->setCheckable(true);
    a->setChecked(!m_ui->playerView->isColumnHidden(i));
    connect(a, &QAction::toggled, this, [this, i] (bool visible) {
      m_ui->playerView->setColumnHidden(i, !visible);
    });
  }
  menu.exec(p);
}

class PlayerData {
public:
  PlayerData(player* p);
  QString table() const;
  QString tax;
  QString luxury;
  QString science;
  QString gold;
  QString gov;
  QString researching;
  QString nation;
  QString title;
  QString capital;
};

PlayerData::PlayerData(player *p) {
  auto me = client_player();
  auto res = research_get(p);
  if (res->researching == A_UNKNOWN) {
    researching =  _("(Unknown)");
  } else if (res->researching == A_UNSET) {
    if (player_has_embassy(me, p)) {
      researching = _("(None)");
    } else {
      researching = _("(Unknown)");
    }
  } else {
    researching = QString("%1 (%2/%3)")
        .arg(research_advance_name_translation(res, res->researching))
        .arg(res->bulbs_researched)
        .arg(res->client.researching_cost);
  }

  tax = _("(Unknown)");
  science = _("(Unknown)");
  luxury = _("(Unknown)");
  if (player_has_embassy(me, p)) {
    tax = QString("%1%").arg(p->economic.tax);
    science = QString("%1%").arg(p->economic.science);
    luxury = QString("%1%").arg(p->economic.luxury);
  }

  gold = _("(Unknown)");
  gov = _("(Unknown)");
  if (could_intel_with_player(me, p)) {
    gold = QString("%1").arg(p->economic.gold);
    gov = government_name_for_player(p);
  }

  nation = nation_adjective_for_player(p);
  char buf[1024];
  title = ruler_title_for_player(p, buf, sizeof(buf));

  capital = _("Unknown");
  if (player_capital(p)) {
    capital = city_name_get(player_capital(p));
  }

}

QString PlayerData::table() const {
  QString r = "<table>";
  QString line = "<tr><td><b>%1</b></td><td>%2</td></tr>";

  r += line.arg(_("Nation")).arg(nation);
  r += line.arg(_("Ruler")).arg(title);
  r += line.arg(_("Government")).arg(gov);
  r += line.arg(_("Capital")).arg(capital);
  r += line.arg(_("Gold")).arg(gold);
  r += line.arg(_("Tax")).arg(tax);
  r += line.arg(_("Science")).arg(science);
  r += line.arg(_("Luxury")).arg(luxury);
  r += line.arg(_("Researching")).arg(researching);
  r += "</table>";

  return r;
}

void PlayerDialog::playerSelected(const QItemSelection &s, const QItemSelection &)
{
  m_ui->playerLabel->clear();
  m_ui->allyLabel->clear();
  m_ui->techLabel->clear();

  m_ui->meetButton->setDisabled(true);
  m_ui->withdrawVisionButton->setDisabled(true);
  m_ui->cancelTreatyButton->setDisabled(true);

  if (s.indexes().isEmpty()) {
    return;
  }

  QModelIndex index = s.indexes().at(0);
  auto pplayer = reinterpret_cast<player*>(index.data(Qt::UserRole).value<void*>());

  if (!pplayer->is_alive) {
    return;
  }

  auto me = client_player();
  m_ui->meetButton->setEnabled(can_meet_with_player(pplayer));
  m_ui->withdrawVisionButton->setEnabled(gives_shared_vision(me, pplayer)
                                         && !players_on_same_team(me, pplayer));
  m_ui->cancelTreatyButton->setEnabled(pplayer_can_cancel_treaty(me, pplayer) != DIPL_ERROR);

  auto d = PlayerData(pplayer);
  m_ui->playerLabel->setText(d.table());


  if (could_intel_with_player(me, pplayer)) {
    QMap<QString, QStringList> diplo;
    players_iterate_alive(other) {
      if (other == pplayer || is_barbarian(other)) continue;
      auto state = player_diplstate_get(pplayer, other);
      QString stateName = diplstate_type_translated_name(state->type);
      QString nation = nation_plural_for_player(other);
      if (gives_shared_vision(pplayer, other)) {
        nation += "(◐‿◑)";
      }
      diplo[stateName] << nation;
    } players_iterate_alive_end;
    QString allies = "";
    QMapIterator<QString, QStringList> it(diplo);
    while (it.hasNext()) {
      it.next();
      allies += "<b>" + it.key() + "</b><br>";
      allies += it.value().join(", ") + ".<br>";
    }
    m_ui->allyLabel->setText(allies);
  }

  if (client_is_global_observer()) {
    auto s = QString(_("<b>Techs known by %1: </b>")).arg(nation_plural_for_player(pplayer));
    QStringList techs;
    auto res = research_get(pplayer);
    advance_iterate(A_FIRST, padvance) {
      auto id = advance_number(padvance);
      if (research_invention_state(res, id) == TECH_KNOWN) {
        techs << research_advance_name_translation(res, id);
      }
    } advance_iterate_end;
    techs.sort(Qt::CaseInsensitive);
    s += "<i>" + techs.join(", ") + ".</i>" ;
    m_ui->techLabel->setText(s);

  } else {
    if (player_has_embassy(me, pplayer) && me != pplayer) {
      auto known = QString(_("<b>Techs unknown by %1: </b>")).
          arg(nation_plural_for_player(pplayer));
      auto unknown = QString(_("<b>Techs unknown by you: </b>"));

      auto r1 = research_get(me);
      auto r2 = research_get(pplayer);
      QStringList knownTechs;
      QStringList unknownTechs;
      advance_iterate(A_FIRST, padvance) {
        auto id = advance_number(padvance);
        if (research_invention_state(r1, id) == TECH_KNOWN
            && (research_invention_state(r2, id) != TECH_KNOWN)) {
          knownTechs << research_advance_name_translation(r1, id);
        }
        if (research_invention_state(r1, id) != TECH_KNOWN
            && (research_invention_state(r2, id) == TECH_KNOWN)) {
          unknownTechs << research_advance_name_translation(r1, id);
        }
      } advance_iterate_end;
      knownTechs.sort(Qt::CaseInsensitive);
      unknownTechs.sort(Qt::CaseInsensitive);
      if (knownTechs.isEmpty()) knownTechs << "None";
      if (unknownTechs.isEmpty()) unknownTechs << "None";
      known += "<i>" + knownTechs.join(", ") + ".</i>";
      unknown += "<i>" + unknownTechs.join(", ") + ".</i>";
      m_ui->techLabel->setText(known + "<br>" + unknown);
    }
  }

}


void PlayerDialog::on_cancelTreatyButton_clicked() {
  auto rows = m_ui->playerView->selectionModel()->selectedRows();
  if (rows.isEmpty()) return;
  auto p = reinterpret_cast<player*>(m_ui->playerView->model()->data(rows[0], Qt::UserRole).value<void*>());
  dsend_packet_diplomacy_cancel_pact(&client.conn, player_number(p), CLAUSE_CEASEFIRE);
}

void PlayerDialog::on_meetButton_clicked() {
  auto rows = m_ui->playerView->selectionModel()->selectedRows();
  if (rows.isEmpty()) return;
  auto p = reinterpret_cast<player*>(m_ui->playerView->model()->data(rows[0], Qt::UserRole).value<void*>());
  dsend_packet_diplomacy_init_meeting_req(&client.conn, player_number(p));
}

void PlayerDialog::on_withdrawVisionButton_clicked()
{
  auto rows = m_ui->playerView->selectionModel()->selectedRows();
  if (rows.isEmpty()) return;
  auto p = reinterpret_cast<player*>(m_ui->playerView->model()->data(rows[0], Qt::UserRole).value<void*>());
  dsend_packet_diplomacy_cancel_pact(&client.conn, player_number(p), CLAUSE_VISION);
}

void PlayerDialog::on_closeButton_clicked() {
  emit closeRequest();
}

PlayerDialog::~PlayerDialog()
{
  delete m_ui;
}


PlayerModel::PlayerModel(QObject *parent)
  : QAbstractListModel(parent)
{}

int PlayerModel::rowCount(const QModelIndex &/*parent*/) const {
  return m_players.size();
}

int PlayerModel::columnCount(const QModelIndex &/*parent*/) const {
  return num_player_dlg_columns;
}

QVariant PlayerModel::data(const QModelIndex &index, int role) const {

  if (!index.isValid()) return QVariant();

  int row = index.row();
  auto pplayer = m_players[row];

  if (role == Qt::UserRole) {
    return QVariant::fromValue(reinterpret_cast<void*>(pplayer));
  }

  int col = index.column();
  auto pcol = &player_dlg_columns[col];

  if (role == Qt::DecorationRole) {
    if (pcol->type == COL_FLAG) {
      return get_nation_flag_sprite(tileset, nation_of_player(pplayer))->pm;
    }
    if (pcol->type == COL_COLOR) {
      return get_player_color(tileset, pplayer)->qcolor;
    }
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    if (pcol->type == COL_BOOLEAN) {
      return pcol->bool_func(pplayer);
    }
    if (pcol->type == COL_TEXT) {
      return pcol->func(pplayer);
    }
    if (pcol->type == COL_RIGHT_TEXT) {
      QString r = pcol->func(pplayer);
      bool ok;
      int v = r.toInt(&ok);
      if (ok) return v;
      if (r == "?") return -1;
      return r;
    }
    return QVariant();
  }
  return QVariant();
}

QVariant PlayerModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
  struct player_dlg_column *pcol;
  if (orientation == Qt::Horizontal && section < num_player_dlg_columns) {
    if (role == Qt::DisplayRole) {
      pcol = &player_dlg_columns[section];
      return pcol->title;
    }
  }
  return QVariant();
}


void PlayerModel::reset() {
  beginResetModel();
  m_players.clear();
  players_iterate(pplayer) {
    if (is_barbarian(pplayer)) continue;
    m_players.append(pplayer);
  } players_iterate_end;
  endResetModel();
}


