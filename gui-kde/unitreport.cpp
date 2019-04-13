#include "unitreport.h"
#include "ui_unitreport.h"
#include <KConfigGroup>
#include <KWindowConfig>
#include <KSharedConfig>
#include <QWindow>
#include "sprite.h"
#include "application.h"
#include <QPainter>
#include "logging.h"

#include "unit.h"
#include "featured_text.h"
#include "city.h"
#include "game.h"
#include "client_main.h"
#include "tilespec.h"
#include "mapview_common.h"
#include "control.h"

using namespace KV;

UnitReport::UnitReport(QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::UnitReport)
  , m_model(new UnitTreeModel(this))
{
  m_ui->setupUi(this);

  m_ui->upgradeButton->setEnabled(m_model->canUpgrade());
  connect(m_model, &UnitTreeModel::dataChanged, this, [=] () {
    m_ui->upgradeButton->setEnabled(m_model->canUpgrade());
  });

  m_ui->unitView->setModel(m_model);
  connect(Application::instance(), &Application::updateUnitReport,
          m_model, &UnitTreeModel::reset);

  connect(m_model, &UnitTreeModel::modelReset, this, [=] () {
    m_ui->unitView->expand(m_model->index(0, 0));
  });

  auto header = m_ui->unitView->header();
  header->setDefaultSectionSize(fontMetrics().width("In-Prog") + 6);
  header->setSectionResizeMode(0, QHeaderView::Stretch);
  for (int i = 1; i < header->count(); i++) {
    header->setSectionResizeMode(i, QHeaderView::Fixed);
  }

  m_ui->unitView->setItemDelegate(new UnitDelegate(this));

  setWindowTitle(QString("%1: Units").arg(qAppName()));
  setWindowFlag(Qt::WindowStaysOnTopHint, false);
  setWindowFlag(Qt::Dialog, false);
  setWindowFlag(Qt::Window, true);

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "UnitReport");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());
}

UnitReport::~UnitReport()
{
  KConfigGroup cnf(KSharedConfig::openConfig(), "UnitReport");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
  delete m_ui;
}

void UnitReport::showAsTable() {
  // expand root item
  m_ui->unitView->expand(m_model->index(0, 0));
  show();
}

void UnitReport::on_upgradeButton_clicked() {
  UnitTreeModel::UnitVector units = m_model->upgradables();
  for (auto id: units) {
    request_unit_upgrade(game_unit_by_number(id));
  }
}

// UnitTreeModelImplementation

UnitTreeModel::UnitTreeModel(QObject* parent)
  : QAbstractItemModel(parent)
  , m_headers(NumColumns)
{
  m_headers[0] = "Type";
  m_headers[InProgColumn] = "In-Prog";
  m_headers[ActiveColumn] = "Active";
  m_headers[ShieldColumn] = "Shield";
  m_headers[FoodColumn] = "Food";
  m_headers[GoldColumn] = "Gold";

  reset();
}

void UnitTreeModel::reset() {

  beginResetModel();

  m_units.clear();
  m_upgradables.clear();
  m_stats.clear();
  m_types.clear();

  m_stats[rootId] = QVector<int>(NumColumns - 1, 0);

  if (!client_has_player()) return;

  CityId capital = 0;
  city_list_iterate(client_player()->cities, pcity) {
    if (is_capital(pcity)) capital = pcity->id;

    if (pcity->production.kind != VUT_UTYPE) continue;

    TypeId tid = static_cast<TypeId>(utype_index(pcity->production.value.utype));
    if (!m_units.contains(tid)) {
      initUnit(pcity->production.value.utype);
    }

    // Account for build slots in city
    int inprog;
    city_production_build_units(pcity, true, &inprog);
    // Unit is in progress even if it won't be done this turn */
    inprog = qMax(inprog, 1);
    m_stats[tid][InProgColumn - 1] += inprog;

  } city_list_iterate_end;

  unit_list_iterate(client_player()->units, punit) {
    TypeId tid = static_cast<TypeId>(utype_index(unit_type_get(punit)));
    if (!m_units.contains(tid)) {
      initUnit(unit_type_get(punit));
    }

    UnitData d;
    d.city = punit->homecity != 0 ? punit->homecity : capital;
    d.unit = punit->id;
    d.stepsFromCity = d.city ?
          real_map_distance(punit->tile, game_city_by_number(d.city)->tile) : -1;
    m_units[tid].units.append(d);

    m_stats[tid][ActiveColumn - 1] += 1;
    if (punit->homecity != 0) {
      m_stats[tid][FoodColumn - 1] += punit->upkeep[O_FOOD];
      m_stats[tid][ShieldColumn - 1] += punit->upkeep[O_SHIELD];
      m_stats[tid][GoldColumn - 1] += punit->upkeep[O_GOLD];
    }
  } unit_list_iterate_end;

  for (auto tid: m_units.keys()) {
    m_types << tid;
  }

  for (auto tid: m_types) {
    for (int i = 0; i < NumColumns - 1; i++) {
      m_stats[rootId][i] += m_stats[tid][i];
    }
  }

  endResetModel();
}

void UnitTreeModel::initUnit(unit_type* utype) {

  auto pix = get_unittype_sprite(get_tileset(), utype, direction8_invalid())->pm;
  int h = QApplication::fontMetrics().height() + 6;
  TypeId tid = static_cast<TypeId>(utype_index(utype));
  m_units[tid].decoration = pix.scaledToHeight(h);


  m_stats[tid] = QVector<int>(NumColumns - 1, 0);

  if (can_upgrade_unittype(client_player(), utype) != nullptr) {
    m_upgradables[tid] = UnitVector(0);
  }
}

bool UnitTreeModel::canUpgrade() const {
  for (auto units: m_upgradables.values()) {
    if (!units.isEmpty()) return true;
  }
  return false;
}

UnitTreeModel::UnitVector UnitTreeModel::upgradables() const {
  UnitVector ups;
  UpgradableIterator it(m_upgradables);
  while (it.hasNext()) {
    it.next();
    ups << it.value();
  }
  return ups;
}

static int internalToTypeId(quintptr internal) {
  return internal / 10000 - 1;
}

static int internalToUnitId(quintptr internal) {
  return internal % 10000;
}

static quintptr unitToInternal(int typeId, int unitId) {
  return (typeId + 1) * 10000 + unitId;
}

QModelIndex UnitTreeModel::index(int row, int column, const QModelIndex &parent) const {
  if (!parent.isValid()) {
    return createIndex(row, column, rootId);
  }

  if (parent.internalId() == rootId) {
    return createIndex(row, column, m_types[row]);
  }

  int typeId = internalToTypeId(parent.internalId());
  if (typeId < 0) {
    typeId = parent.internalId();
    auto id = unitToInternal(typeId, m_units[typeId].units[row].unit);
    return createIndex(row, column, id);
  }
  return QModelIndex();
}

QModelIndex UnitTreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid() || index.internalId() == rootId) return QModelIndex();

  int typeId = internalToTypeId(index.internalId());

  if (typeId < 0) {
    return createIndex(0, 0, rootId);
  }

  return createIndex(m_types.indexOf(typeId), 0, typeId);
}

int UnitTreeModel::columnCount(const QModelIndex &/*parent*/) const {
  return m_headers.size();
}

int UnitTreeModel::rowCount(const QModelIndex &parent) const {
  if (!parent.isValid()) return 1; // root item

  if (parent.internalId() == rootId) return m_types.size();

  int typeId = internalToTypeId(parent.internalId());
  if (typeId < 0) {
    typeId = parent.internalId();
    return m_units[typeId].units.size();
  }

  return 0;
}

QVariant UnitTreeModel::headerData(int section, Qt::Orientation, int role) const {
  if (role != Qt::DisplayRole) return QVariant();
  return m_headers[section];
}

QVariant UnitTreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  // root item
  if (!index.parent().isValid()) {
    if (role == Qt::DisplayRole) {
      if (index.column() == 0) {
        return "Totals";
      }
      return m_stats[index.internalId()][index.column() - 1];
    }

    if (role == Qt::TextAlignmentRole && index.column() > 0) {
      return Qt::AlignRight;
    }

    return QVariant();
  }

  // unit type item
  if (!index.parent().parent().isValid()) {
    if (role == Qt::DisplayRole) {
      if (index.column() == 0) {
        return utype_name_translation(utype_by_number(index.internalId()));
      }
      return m_stats[index.internalId()][index.column() - 1];
    }

    if (role == Qt::TextAlignmentRole && index.column() > 0) {
      return Qt::AlignRight;
    }

    if (role == Qt::DecorationRole && index.column() == 0) {
      return m_units[index.internalId()].decoration;
    }

    if (role == Qt::CheckStateRole && index.column() == 0) {
      if (m_upgradables.contains(index.internalId())) {
        auto ups = m_upgradables[index.internalId()];
        if (ups.isEmpty()) return Qt::Unchecked;
        if (ups.size() == rowCount(index)) return Qt::Checked;
        return Qt::PartiallyChecked;
      }
      return QVariant();
    }

    return QVariant();
  }

  // unit item
  TypeId tid = index.parent().internalId();
  UnitData d = m_units[tid].units[index.row()];

  if (role == Qt::DisplayRole && index.column() == 0) {
    QString citytxt;
    if (d.city == 0) {
      citytxt = "Homeless";
    } else {
      QString cityname = city_name_get(game_city_by_number(d.city));
      if (d.stepsFromCity == 0) {
        citytxt = QString("in %1").arg(cityname);
      } else if (d.stepsFromCity == 1) {
        citytxt = QString("1 step from %1").arg(cityname);
      } else {
        citytxt = QString("%1 steps from %2").arg(d.stepsFromCity).arg(cityname);
      }
    }
    return citytxt;
  }

  if (role == Qt::CheckStateRole && index.column() == 0) {
    if (m_upgradables.contains(tid)) {
      if (m_upgradables[tid].contains(d.unit)) {
        return Qt::Checked;
      }
      return Qt::Unchecked;
    }
    return QVariant();
  }

  if (role == Qt::UserRole && index.column() == 0) {
    return d.unit;
  }

  return QVariant();
}


bool UnitTreeModel::setData(const QModelIndex &idx, const QVariant &value, int role) {
  if (role != Qt::CheckStateRole) return false;
  if (!idx.isValid() || !idx.parent().isValid()) return false;
  if (idx.column() > 0) return false;

  auto state = static_cast<Qt::CheckState>(value.toInt());
  QVector<int> roles{Qt::CheckStateRole};

  // unit type item
  if (!idx.parent().parent().isValid()) {
    TypeId tid = idx.internalId();
    if (!m_upgradables.contains(tid)) return false;

    QModelIndex top = index(0, 0, idx);
    QModelIndex bot = index(rowCount(idx) - 1, 0, idx);

    UnitVector& ups = m_upgradables[tid];
    if (state == Qt::Checked) {
      if (ups.size() == rowCount(idx)) return false;
      ups.clear();
      for (UnitData d: m_units[tid].units) {
        ups << d.unit;
      }
      emit dataChanged(idx, idx, roles);
      emit dataChanged(top, bot, roles);
      return true;
    }
    if (state == Qt::Unchecked) {
      if (ups.isEmpty()) return false;
      ups.clear();
      emit dataChanged(idx, idx, roles);
      emit dataChanged(top, bot, roles);
      return true;
    }

    return false;
  }

  // unit item
  TypeId tid = idx.parent().internalId();
  if (!m_upgradables.contains(tid)) return false;
  UnitVector& ups = m_upgradables[tid];
  UnitData d = m_units[tid].units[idx.row()];
  if (state == Qt::Checked) {
    if (ups.contains(d.unit)) return false;
    ups.append(d.unit);
    emit dataChanged(idx, idx, roles);
    emit dataChanged(idx.parent(), idx.parent(), roles);
    return true;
  }
  if (state == Qt::Unchecked) {
    if (!ups.contains(d.unit)) return false;
    ups.removeAll(d.unit);
    emit dataChanged(idx, idx, roles);
    emit dataChanged(idx.parent(), idx.parent(), roles);
    return true;
  }

  return false;
}

Qt::ItemFlags UnitTreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags baseflags = Qt::ItemIsEnabled;

  if (index.column() > 0) return baseflags;

  if (index.internalId() == rootId) {
    return baseflags;
  }

  TypeId tid = internalToTypeId(index.internalId());


  if (tid < 0) {
    tid = index.internalId();
    if (!m_upgradables.contains(tid)) return baseflags;
    return baseflags | Qt::ItemIsAutoTristate | Qt::ItemIsUserCheckable;
  }

  if (!m_upgradables.contains(tid)) return baseflags;

  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}


// UnitDelegate implementation


UnitDelegate::UnitDelegate(QObject *parent)
  : QStyledItemDelegate(parent) {}


void UnitDelegate::paint(QPainter *p,
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

bool UnitDelegate::editorEvent(QEvent *ev,
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

  auto id = static_cast<UnitTreeModel::UnitId>(d.toInt());
  unit* punit = game_unit_by_number(id);
  if (punit == nullptr) return false;

  center_tile_mapcanvas(unit_tile(punit));
  link_marks_clear_all();
  link_mark_restore(TLT_UNIT, id);
  return true;
}

