#include "nationdialog.h"
#include "ui_nationdialog.h"
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include <QFontDatabase>
#include "sprite.h"

#include "options.h"
#include "helpdata.h"

#include "nation.h"
#include "game.h"
#include "style.h"
#include "tilespec.h"
#include "client_main.h"
#include "chatline_common.h"

using namespace KV;

NationDialog::NationDialog(QWidget *parent) :
  QDialog(parent),
  m_ui(new Ui::NationDialog)
{
  m_ui->setupUi(this);

  // Nation set combo
  connect(m_ui->setCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
          this, &NationDialog::nationsetChanged);

  // Nation group list
  connect(m_ui->groupList, &QListWidget::itemSelectionChanged,
          this, &NationDialog::nationGroupSelected);

  // Nation list filter
  connect(m_ui->filterEdit, &QLineEdit::textChanged,
          this, &NationDialog::filterChanged);

  // Nation list
  auto model = new NationModel(this);
  auto proxy = new QSortFilterProxyModel(this);
  proxy->setSourceModel(model);
  proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ui->nationList->setModel(proxy);
  connect(m_ui->nationList->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &NationDialog::nationSelected);

  // Leader combo
  connect(m_ui->leaderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &NationDialog::leaderChanged);

  // Random/Ok/Cancel
  connect(m_ui->randomButton, &QPushButton::clicked,
          this, &NationDialog::randomNation);
  connect(this, &NationDialog::accepted,
          this, &NationDialog::setNation);

}

NationDialog::~NationDialog()
{
  delete m_ui;
}


void NationDialog::init(const player *p) {
  QString title;
  if (client_state() == C_S_RUNNING) {
    title = _("Edit Nation");
  } else if (p != nullptr && p == client.conn.playing) {
    title = _("What Nation Will You Be?");
  } else {
    title = _("Pick Nation");
  }
  setWindowTitle(title);

  m_player = p;


  // Nation set combo
  m_ui->setCombo->blockSignals(true);
  m_ui->setCombo->clear();
  nation_sets_iterate(pset) {
    m_ui->setCombo->addItem(nation_set_name_translation(pset),
                            nation_set_rule_name(pset));
  } nation_sets_iterate_end;

  auto popt = optset_option_by_name(server_optset, "nationset");
  if (popt) {
    auto s = nation_set_by_setting_value(option_str_get(popt));
    m_ui->setCombo->setCurrentText(nation_set_name_translation(s));
    m_ui->setCombo->setToolTip(nation_set_description(s));
  }
  m_ui->setCombo->blockSignals(false);

  // Nation group list
  initNationGroups();

  // Nation filter
  m_ui->filterEdit->clear();

  // Style List
  m_ui->styleList->clear();
  styles_iterate(pstyle) {
    int i = basic_city_style_for_style(pstyle);

    if (i >= 0) {
      auto item = new QListWidgetItem;
      auto pix = get_sample_city_sprite(tileset, i)->pm;
      item->setData(Qt::DecorationRole, pix);
      item->setData(Qt::UserRole, style_number(pstyle));
      item->setSizeHint(QSize(pix.width(), pix.height()));
      item->setText(style_name_translation(pstyle));

      m_ui->styleList->addItem(item);
    }
  } styles_iterate_end;

  // Leader combo
  m_ui->leaderCombo->clear();

  // Sex group
  m_ui->maleButton->setChecked(true);
  m_ui->femaleButton->setChecked(false);

  // Info browser
  m_ui->infoBrowser->setText(_("Choose nation"));

}

void NationDialog::nationsetChanged(const QString &) {
  auto poption = optset_option_by_name(server_optset, "nationset");
  auto name = m_ui->setCombo->currentData().toString().toLocal8Bit().data();
  if (nation_set_by_setting_value(option_str_get(poption))
      != nation_set_by_rule_name(name)) {
    option_str_set(poption, name);
    m_ui->setCombo->setToolTip(nation_set_description(nation_set_by_rule_name(name)));
  }
}

void NationDialog::initNationGroups() {

  m_ui->groupList->clear();
  m_ui->groupList->addItem(_("All nations"));

  for (int i = 0; i < nation_group_count(); i++) {
    auto group = nation_group_by_number(i);
    if (is_nation_group_hidden(group)) continue;

    // checking if group is empty
    int count = 0;
    nations_iterate(pnation) {
      if (!is_nation_playable(pnation)
          || !is_nation_pickable(pnation)
          || !nation_is_in_group(pnation, group)) {
        continue;
      }
      count ++;
      if (count > 1) break;
    } nations_iterate_end;
    if (count == 0) continue;

    auto item = new QListWidgetItem(nation_group_name_translation(group));
    item->setData(Qt::UserRole, i);
    m_ui->groupList->addItem(item);
  }
  m_ui->groupList->setItemSelected(m_ui->groupList->item(0), true);
}

void NationDialog::nationGroupSelected() {

  auto proxy = qobject_cast<QSortFilterProxyModel*>(m_ui->nationList->model());
  auto nations = qobject_cast<NationModel*>(proxy->sourceModel());

  nations->removeRows(0, nations->rowCount());

  QListWidgetItem* item = nullptr;
  auto items = m_ui->groupList->selectedItems();
  auto indices = m_ui->groupList->selectionModel()->selectedIndexes();
  if (!items.isEmpty() && !indices.isEmpty() && indices[0].row() != 0) {
    item = items[0];
  }
  bool all = item == nullptr;
  int index = all ? -1 : item->data(Qt::UserRole).toInt();
  auto group = all ? nullptr : nation_group_by_number(index);

  nations_iterate(pnation) {
    if (!is_nation_playable(pnation)
        || !is_nation_pickable(pnation)) {
      continue;
    }
    if (!all  && !nation_is_in_group(pnation, group)) continue;

    auto newrow = nations->rowCount();
    nations->insertRow(newrow);
    QModelIndex idx = nations->index(newrow);
    if (pnation->player && pnation->player != m_player) {
      QVariant v = nations->data(idx, Qt::FontRole);
      QFont f;
      if (v.isValid()) {
        f = v.value<QFont>();
      } else {
        f = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
      }
      f.setStrikeOut(true);
      nations->setData(idx, f, Qt::FontRole);
    }
    auto pix = get_nation_flag_sprite(tileset, pnation)->pm;
    nations->setData(idx, pix, Qt::DecorationRole);
    nations->setData(idx, nation_number(pnation), Qt::UserRole);
    nations->setData(idx, nation_adjective_translation(pnation), Qt::DisplayRole);
  } nations_iterate_end;

  filterChanged(QString("%1 ").arg(m_ui->filterEdit->text()));
  filterChanged(m_ui->filterEdit->text());
}



void NationDialog::filterChanged(const QString &s) {
  auto proxy = qobject_cast<QSortFilterProxyModel*>(m_ui->nationList->model());
  proxy->sort(0);
  proxy->setFilterFixedString(s);
}

void NationDialog::nationSelected(const QItemSelection& c, const QItemSelection&) {

  auto indexes = c.indexes();
  if (indexes.isEmpty()) return;

  auto index = indexes.at(0);

  auto nation_id = index.data(Qt::UserRole).toInt();

  char buf[2000];
  helptext_nation(buf, sizeof(buf), nation_by_number(nation_id), nullptr);
  m_ui->infoBrowser->setText(buf);
  m_ui->leaderCombo->clear();
  if (client.conn.playing == m_player) {
    m_ui->leaderCombo->addItem(m_player->username, true);
  }
  nation_leader_list_iterate(nation_leaders(nation_by_number(nation_id)), pleader) {
    m_ui->leaderCombo->addItem(QString::fromUtf8(nation_leader_name(pleader)), nation_leader_is_male(pleader));
  } nation_leader_list_iterate_end;

  // select style for nation
  int style_id = style_number(style_of_nation(nation_by_number(nation_id)));

  for (int i = 0; i < m_ui->styleList->count(); i++) {
    auto item = m_ui->styleList->item(i);
    int id = item->data(Qt::UserRole).toInt();
    if (id == style_id) {
      m_ui->styleList->setItemSelected(item, true);
      break;
    }
  }
}

void NationDialog::leaderChanged(int idx) {
  auto male = m_ui->leaderCombo->itemData(idx).toBool();
  m_ui->maleButton->setChecked(male);
  m_ui->femaleButton->setChecked(!male);
}

void NationDialog::randomNation() {
  dsend_packet_nation_select_req(&client.conn,
                                 player_number(m_player),
                                 -1,
                                 false,
                                 "",
                                 0);
  QString s("%1 selects nation in random.");
  output_window_append(ftc_client, s.arg(m_player->username).toUtf8());
}


void NationDialog::setNation() {

  auto indices = m_ui->nationList->selectionModel()->selectedIndexes();
  if (indices.isEmpty()) {
    output_window_append(ftc_client, _("You must select nation."));
    return;
  }

  auto index = indices.first();
  auto nation_id = index.data(Qt::UserRole).toInt();

  indices = m_ui->styleList->selectionModel()->selectedIndexes();
  if (indices.isEmpty()) {
    output_window_append(ftc_client, _("You must select your style."));
    return;
  }

  index = indices.first();
  auto style_id = index.data(Qt::UserRole).toInt();

  if (m_ui->leaderCombo->currentText() == "") {
    output_window_append(ftc_client, _("You must type a legal name."));
    return;
  }

  auto nplayer = nation_by_number(nation_id)->player;
  if (nplayer != nullptr && nplayer != m_player) {
    output_window_append(ftc_client,
                         _("Nation has been chosen by other player"));
    return;
  }

  dsend_packet_nation_select_req(&client.conn,
                                 player_number(m_player),
                                 nation_id,
                                 m_ui->maleButton->isChecked(),
                                 m_ui->leaderCombo->currentText().toUtf8().data(),
                                 style_id);
}



//
// NationModel
//

NationModel::NationModel(QObject* parent)
  : QAbstractListModel(parent) {}

int NationModel::rowCount(const QModelIndex &/*parent*/) const {
  return m_nations.size();
}

QVariant NationModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  Nation d = m_nations[index.row()];
  if (role == Qt::DisplayRole || role == Qt::EditRole) return d.name;
  if (role == Qt::DecorationRole) return d.flag;
  if (role == Qt::UserRole) return d.id;
  if (role == Qt::FontRole && m_fonts.contains(d.id)) return m_fonts[d.id];
  return QVariant();
}

bool NationModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (!index.isValid()) return false;

  Nation& d = m_nations[index.row()];
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    d.name = value.toString();
    return true;
  }
  if (role == Qt::DecorationRole) {
    d.flag = value.value<QPixmap>();
    return true;
  }
  if (role == Qt::UserRole) {
    d.id = value.toInt();
    return true;
  }
  if (role == Qt::FontRole) {
    m_fonts[d.id] = value.value<QFont>();
    return true;
  }
  return false;
}


bool NationModel::insertRows(int row, int count, const QModelIndex& parent) {
  if (row < 0 || row > m_nations.size()) return false;
  if (count <= 0) return false;
  Nation d{};
  beginInsertRows(parent, row, row + count - 1);
  m_nations.insert(row, count, d);
  endInsertRows();
  return true;
}

bool NationModel::removeRows(int row, int count, const QModelIndex& parent) {
  if (row < 0 || row >= m_nations.size()) return false;
  if (count <= 0 || count > m_nations.size()) return false;
  beginRemoveRows(parent, row, row + count - 1);
  for (int i = 0; i < count; i++) {
    m_fonts.remove(m_nations[row + i].id);
  }
  m_nations.remove(row, count);
  endRemoveRows();
  return true;
}

