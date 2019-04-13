#include "economyreport.h"
#include "ui_economyreport.h"
#include <KConfigGroup>
#include <KWindowConfig>
#include <KSharedConfig>
#include <QWindow>
#include "sprite.h"
#include "application.h"
#include <QPainter>
#include "logging.h"
#include <QPalette>
#include "messagebox.h"

#include "unit.h"
#include "city.h"
#include "game.h"
#include "client_main.h"
#include "tilespec.h"
#include "mapview_common.h"
#include "control.h"
#include "climisc.h"
#include "citydlg_common.h"

using namespace KV;

using CityOrUnitId = EconomyTreeModel::CityOrUnitId;
using CityOrUnitMap = EconomyTreeModel::CityOrUnitMap;
using CityOrUnitIterator = EconomyTreeModel::CityOrUnitIterator;
using CityOrUnitVector = EconomyTreeModel::CityOrUnitVector;
using UniversalId = EconomyTreeModel::UniversalId;

EconomyReport::EconomyReport(QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::EconomyReport)
  , m_model(new EconomyTreeModel(this))
{
  m_ui->setupUi(this);

  m_ui->sellButton->setEnabled(m_model->hasSelection());
  connect(m_model, &EconomyTreeModel::dataChanged, this, [=] () {
    m_ui->sellButton->setEnabled(m_model->hasSelection());
  });

  m_ui->obsoleteButton->setEnabled(m_model->hasObsoletes());

  m_ui->buildingView->setModel(m_model);
  connect(Application::instance(), &Application::updateEconomyReport,
          m_model, &EconomyTreeModel::reset);

  connect(m_model, &EconomyTreeModel::modelReset, this, [=] () {
    m_ui->buildingView->expand(m_model->index(0, 0));
    m_ui->obsoleteButton->setEnabled(m_model->hasObsoletes());
    m_ui->sellButton->setEnabled(m_model->hasSelection());
    int gold = 0;
    city_list_iterate(client_player()->cities, pcity) {
      gold += pcity->prod[O_GOLD];
      if (city_production_has_flag(pcity, IF_GOLD)) {
        gold += MAX(0, pcity->surplus[O_SHIELD]);
      }
    } city_list_iterate_end;
    m_ui->incomeLabel->setText(QString("Income: %1").arg(gold));
  });

  auto header = m_ui->buildingView->header();
  header->setDefaultSectionSize(fontMetrics().width("Redundant") + 6);
  header->setSectionResizeMode(0, QHeaderView::Stretch);
  for (int i = 1; i < header->count(); i++) {
    header->setSectionResizeMode(i, QHeaderView::Fixed);
  }

  m_ui->buildingView->setItemDelegate(new EcoDelegate(this));


  setWindowTitle(QString("%1: Economy").arg(qAppName()));
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
  setWindowFlag(Qt::Dialog, false);
  setWindowFlag(Qt::Window, true);

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "EconomyReport");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());
}

EconomyReport::~EconomyReport()
{
  KConfigGroup cnf(KSharedConfig::openConfig(), "EconomyReport");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
  delete m_ui;
}

void EconomyReport::showAsTable() {
  // expand root item
  m_ui->buildingView->expand(m_model->index(0, 0));
  show();
}

void EconomyReport::on_sellButton_clicked() {
  CityOrUnitMap sel = m_model->selected();
  CityOrUnitIterator it(sel);
  while(it.hasNext()) {
    it.next();
    UniversalId cid = it.key();
    auto u = cid_decode(cid);
    if (u.kind == VUT_IMPROVEMENT) {
      for (auto id: it.value()) {
        city* c = game_city_by_number(id);
        auto res = test_player_sell_building_now(client_player(), c, u.value.building);
        if (res == TR_SUCCESS) {
          city_sell_improvement(c, improvement_number(u.value.building));
        } else {
          QString text;
          if (res == TR_ALREADY_SOLD) {
            text = QString("Already sold %1 from %2")
                .arg(improvement_name_translation(u.value.building))
                .arg(city_name_get(c));
          } else {
            text = QString("Cannot sell %1 in %2")
                .arg(improvement_name_translation(u.value.building))
                .arg(city_name_get(c));
          }
          MessageBox ask(this, text, "Cannot sell");
          ask.setStandardButtons(QMessageBox::Ok);
          ask.exec();
        }
      }
    } else {
      for (auto id: it.value()) {
        unit* p = game_unit_by_number(id);
        if (unit_can_do_action(p, ACTION_DISBAND_UNIT)) {
          request_unit_disband(p);
        } else {
          QString text = QString("Cannot disband %1")
              .arg(unit_name_translation(p));
          MessageBox ask(this, text, "Cannot disband");
          ask.setStandardButtons(QMessageBox::Ok);
          ask.exec();
        }
      }
    }
  }
}

void EconomyReport::on_obsoleteButton_clicked() {
  CityOrUnitMap obs = m_model->obsoletes();
  CityOrUnitIterator it(obs);
  while(it.hasNext()) {
    it.next();
    UniversalId cid = it.key();
    auto u = cid_decode(cid);
    if (u.kind != VUT_IMPROVEMENT) continue;
    for (auto id: it.value()) {
      city* c = game_city_by_number(id);
      if (test_player_sell_building_now(client_player(), c, u.value.building)) {
        city_sell_improvement(c, improvement_number(u.value.building));
      }
    }
  }
}

// EconomyTreeModel Implementation

EconomyTreeModel::EconomyTreeModel(QObject* parent)
  : QAbstractItemModel(parent)
  , m_headers(NumColumns)
{
  m_headers[0] = "Type";
  m_headers[RedundantColumn] = "Redundant";
  m_headers[CountColumn] = "Count";
  m_headers[CostColumn] = "Cost";
  m_headers[UpkeepColumn] = "Upkeep";

  reset();
}

void EconomyTreeModel::reset() {

  beginResetModel();

  m_universalMap.clear();
  m_selected.clear();
  m_stats.clear();
  m_universals.clear();

  m_stats[rootId] = QVector<int>(NumColumns - 1, 0);

  if (!client_has_player()) return;

  improvement_iterate(pimp) {
    if (!is_improvement(pimp)) continue;
    auto cid = static_cast<UniversalId>(cid_encode_building(pimp));

    city_list_iterate(client_player()->cities, pcity) {
      if (!city_has_building(pcity, pimp)) continue;
      if (!m_universalMap.contains(cid)) {
        initUniversal(cid);
      }

      CityData d;
      d.unit = 0;
      d.id = pcity->id;
      d.redundant = is_improvement_redundant(pcity, pimp);
      m_universalMap[cid].cities.append(d);

      if (d.redundant) {
        m_stats[cid][RedundantColumn - 1] += 1;
      }
      m_stats[cid][CountColumn - 1] += 1;
      m_stats[cid][UpkeepColumn - 1] += city_improvement_upkeep(pcity, pimp);
    }
    city_list_iterate_end;

  }
  improvement_iterate_end;

  unit_list_iterate(client_player()->units, punit) {
    if (punit->homecity == 0) continue;
    auto utype = unit_type_get(punit);
    int cost = utype_upkeep_cost(utype, client_player(), O_GOLD);
    if (cost == 0) continue;
    auto cid = static_cast<UniversalId>(cid_encode_unit(utype));
    if (!m_universalMap.contains(cid)) {
      initUniversal(cid);
    }
    CityData d;
    d.id = punit->homecity;
    d.unit = punit->id;
    d.redundant = false;
    m_universalMap[cid].cities.append(d);

    m_stats[cid][CountColumn - 1] += 1;
    m_stats[cid][UpkeepColumn - 1] += cost;
  }
  unit_list_iterate_end;

  for (auto cid: m_universalMap.keys()) {
    m_universals << cid;
  }

  for (auto cid: m_universals) {
    m_stats[cid][CostColumn - 1] = m_stats[cid][UpkeepColumn - 1] / m_stats[cid][CountColumn - 1];
    for (int i = 0; i < NumColumns - 1; i++) {
      m_stats[rootId][i] += m_stats[cid][i];
    }
  }

  endResetModel();
}

void EconomyTreeModel::initUniversal(UniversalId id) {

  auto u = cid_decode(id);

  int h = QApplication::fontMetrics().height() + 6;
  QPixmap pix;
  if (u.kind == VUT_IMPROVEMENT) {
    pix = get_building_sprite(get_tileset(), u.value.building)->pm;
  } else {
    pix = get_unittype_sprite(get_tileset(), u.value.utype, direction8_invalid())->pm;
  }
  m_universalMap[id].decoration = pix.scaledToHeight(h);

  m_stats[id] = QVector<int>(NumColumns - 1, 0);

}

bool EconomyTreeModel::hasSelection() const {
  for (auto cities: m_selected.values()) {
    if (!cities.isEmpty()) return true;
  }
  return false;
}

CityOrUnitMap EconomyTreeModel::selected() const {
  CityOrUnitMap items;
  UniversalIterator it(m_selected);
  while (it.hasNext()) {
    it.next();
    UniversalId cid = it.key();
    auto u = cid_decode(cid);
    for (auto idx: it.value()) {
      CityData d = m_universalMap[cid].cities[idx];
      if (u.kind == VUT_IMPROVEMENT) {
        items[cid] << d.id;
      } else {
        items[cid] << d.unit;
      }
    }
  }
  return items;
}

bool EconomyTreeModel::hasObsoletes() const {
  DataIterator it(m_universalMap);
  while (it.hasNext()) {
    it.next();
    for (auto d: it.value().cities) {
      if (d.redundant) return true;
    }
  }
  return false;
}

CityOrUnitMap EconomyTreeModel::obsoletes() const {
  CityOrUnitMap items;
  DataIterator it(m_universalMap);
  while (it.hasNext()) {
    it.next();
    UniversalId cid = it.key();
    for (auto d: it.value().cities) {
      if (d.redundant) {
        items[cid] << d.id;
      }
    }
  }
  return items;
}

static int internalToUniversalId(quintptr internal) {
  return internal / 10000 - 1;
}

static int internalToCityIndex(quintptr internal) {
  return internal % 10000;
}

static quintptr cityIndexToInternal(int cid, int cityIndex) {
  return (cid + 1) * 10000 + cityIndex;
}

QModelIndex EconomyTreeModel::index(int row, int column, const QModelIndex &parent) const {
  if (!parent.isValid()) {
    return createIndex(row, column, rootId);
  }

  if (parent.internalId() == rootId) {
    return createIndex(row, column, m_universals[row]);
  }

  UniversalId cid = internalToUniversalId(parent.internalId());
  if (cid < 0) {
    cid = parent.internalId();
    auto id = cityIndexToInternal(cid, row);
    return createIndex(row, column, id);
  }
  return QModelIndex();
}

QModelIndex EconomyTreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid() || index.internalId() == rootId) return QModelIndex();

  UniversalId cid = internalToUniversalId(index.internalId());

  if (cid < 0) {
    return createIndex(0, 0, rootId);
  }

  return createIndex(m_universals.indexOf(cid), 0, cid);
}

int EconomyTreeModel::columnCount(const QModelIndex &/*parent*/) const {
  return m_headers.size();
}

int EconomyTreeModel::rowCount(const QModelIndex &parent) const {
  if (!parent.isValid()) return 1; // root item

  if (parent.internalId() == rootId) return m_universals.size();

  UniversalId cid = internalToUniversalId(parent.internalId());
  if (cid < 0) {
    cid = parent.internalId();
    return m_universalMap[cid].cities.size();
  }

  return 0;
}

QVariant EconomyTreeModel::headerData(int section, Qt::Orientation, int role) const {
  if (role != Qt::DisplayRole) return QVariant();
  return m_headers[section];
}

QVariant EconomyTreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  // root item
  if (!index.parent().isValid()) {
    if (role == Qt::DisplayRole) {
      if (index.column() == 0) {
        return "Totals";
      }
      if (index.column() == CostColumn) return QVariant();
      return m_stats[index.internalId()][index.column() - 1];
    }

    if (role == Qt::TextAlignmentRole && index.column() > 0) {
      return Qt::AlignRight;
    }

    return QVariant();
  }

  // unit type item
  if (!index.parent().parent().isValid()) {
    UniversalId cid = index.internalId();
    auto u = cid_decode(cid);

    if (role == Qt::DisplayRole) {
      if (index.column() == 0) {
        if (u.kind == VUT_IMPROVEMENT) {
          return improvement_name_translation(u.value.building);
        } else {
          return utype_name_translation(u.value.utype);
        }
      }
      return m_stats[cid][index.column() - 1];
    }

    if (role == Qt::TextAlignmentRole && index.column() > 0) {
      return Qt::AlignRight;
    }

    if (role == Qt::DecorationRole && index.column() == 0) {
      return m_universalMap[cid].decoration;
    }

    if (role == Qt::CheckStateRole && index.column() == 0) {
      auto cityIndices = m_selected[cid];
      if (cityIndices.isEmpty()) return Qt::Unchecked;
      if (cityIndices.size() == rowCount(index)) return Qt::Checked;
      return Qt::PartiallyChecked;
    }

    return QVariant();
  }

  // unit item
  UniversalId cid = index.parent().internalId();
  CityData d = m_universalMap[cid].cities[index.row()];
  if (role == Qt::DisplayRole && index.column() == 0) {
    auto u = cid_decode(cid);
    QString cityName = city_name_get(game_city_by_number(d.id));
    if (u.kind == VUT_IMPROVEMENT) {
      return QString("in %1").arg(cityName);
    }
    return QString("from %1").arg(cityName);
  }

  if (role == Qt::DecorationRole && index.column() == RedundantColumn) {
    if (d.redundant) {
      return QIcon::fromTheme("list-remove");
    }
    return QVariant();
  }


  if (role == Qt::CheckStateRole && index.column() == 0) {
    if (m_selected[cid].contains(index.row())) {
      return Qt::Checked;
    }
    return Qt::Unchecked;
  }

  if (role == Qt::UserRole && index.column() == 0) {
    auto u = cid_decode(cid);
    if (u.kind == VUT_IMPROVEMENT) {
      return d.id;
    } else {
      return d.unit;
    }
  }

  return QVariant();
}


bool EconomyTreeModel::setData(const QModelIndex &idx, const QVariant &value, int role) {
  if (role != Qt::CheckStateRole) return false;
  if (!idx.isValid() || !idx.parent().isValid()) return false;
  if (idx.column() > 0) return false;

  auto state = static_cast<Qt::CheckState>(value.toInt());
  QVector<int> roles{Qt::CheckStateRole};

  // unit type item
  if (!idx.parent().parent().isValid()) {
    UniversalId cid = idx.internalId();

    QModelIndex top = index(0, 0, idx);
    QModelIndex bot = index(rowCount(idx) - 1, 0, idx);

    IndexVector& sel = m_selected[cid];
    if (state == Qt::Checked) {
      if (sel.size() == rowCount(idx)) return false;
      sel.clear();
      for (int i = 0; i < m_universalMap[cid].cities.size(); i++) {
        sel << i;
      }
      emit dataChanged(idx, idx, roles);
      emit dataChanged(top, bot, roles);
      return true;
    }
    if (state == Qt::Unchecked) {
      if (sel.isEmpty()) return false;
      sel.clear();
      emit dataChanged(idx, idx, roles);
      emit dataChanged(top, bot, roles);
      return true;
    }

    return false;
  }

  // unit item
  UniversalId cid = idx.parent().internalId();
  IndexVector& sel = m_selected[cid];
  int cityIndex = idx.row();
  if (state == Qt::Checked) {
    if (sel.contains(cityIndex)) return false;
    sel.append(cityIndex);
    emit dataChanged(idx, idx, roles);
    emit dataChanged(idx.parent(), idx.parent(), roles);
    return true;
  }
  if (state == Qt::Unchecked) {
    if (!sel.contains(cityIndex)) return false;
    sel.removeAll(cityIndex);
    emit dataChanged(idx, idx, roles);
    emit dataChanged(idx.parent(), idx.parent(), roles);
    return true;
  }

  return false;
}

Qt::ItemFlags EconomyTreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags baseflags = Qt::ItemIsEnabled;

  if (index.column() > 0) return baseflags;

  if (index.internalId() == rootId) return baseflags;

  UniversalId cid = internalToUniversalId(index.internalId());
  if (cid < 0) {
    return baseflags | Qt::ItemIsAutoTristate | Qt::ItemIsUserCheckable;
  }

  return baseflags | Qt::ItemIsUserCheckable;
}


// EcoDelegate implementation


EcoDelegate::EcoDelegate(QObject *parent)
  : QStyledItemDelegate(parent) {}


void EcoDelegate::paint(QPainter *p,
                         const QStyleOptionViewItem &opt,
                         const QModelIndex &index) const {


  if (!index.isValid() ||
      !index.parent().isValid() ||
      !index.parent().parent().isValid() ||
      index.column() > 0) {
    return QStyledItemDelegate::paint(p, opt, index);
  }

  auto options = opt;
  QColor c = QApplication::palette().color(QPalette::Active, QPalette::Link);
  if (options.state & QStyle::State_MouseOver) {
    c = c.lighter();
  }
  options.palette.setColor(QPalette::Active, QPalette::Text, c);
  options.font.setUnderline(true);
  QStyledItemDelegate::paint(p, options, index);
}

bool EcoDelegate::editorEvent(QEvent *ev,
                               QAbstractItemModel *model,
                               const QStyleOptionViewItem &opt,
                               const QModelIndex &index) {


  bool clicked = QStyledItemDelegate::editorEvent(ev, model, opt, index);

  if (!index.isValid() ||
      !index.parent().isValid() ||
      !index.parent().parent().isValid() ||
      index.column() > 0 ||
      clicked) {
    return clicked;
  }

  if (ev->type() != QEvent::MouseButtonRelease) return false;

  auto e = static_cast<QMouseEvent*>(ev);

  if (!opt.rect.contains(e->pos())) return false;

  auto d = model->data(index, Qt::UserRole);
  if (!d.isValid()) return false;
  int id = d.toInt();

  UniversalId cid = index.parent().internalId();
  auto u = cid_decode(cid);

  if (u.kind == VUT_IMPROVEMENT) {
    city* pcity = game_city_by_number(id);
    center_tile_mapcanvas(city_tile(pcity));
    link_marks_clear_all();
    link_mark_restore(TLT_CITY, id);
  } else {
    unit* punit = game_unit_by_number(id);
    if (punit == nullptr) return false;
    center_tile_mapcanvas(unit_tile(punit));
    link_marks_clear_all();
    link_mark_restore(TLT_UNIT, id);
  }
  return true;
}

