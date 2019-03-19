#include "buildables.h"
#include <QHeaderView>
#include <QApplication>
#include <cmath>
#include <QDesktopWidget>
#include "sprite.h"
#include <QPainter>
#include <QHelpEvent>
#include <QToolTip>
#include "logging.h"
#include <QItemSelectionModel>

#include "city.h"
#include "tech.h"
#include "movement.h"
#include "helpdata.h"
#include "tilespec.h"
#include "game.h"
#include "client_main.h"

using namespace KV;


static void pixmap_put_x(QPixmap *pix)
{
  QPen pen(QColor(0, 0, 0));
  QPainter p;

  pen.setWidth(2);
  p.begin(pix);
  p.setRenderHint(QPainter::Antialiasing);
  p.setPen(pen);
  p.drawLine(0, 0, pix->width(), pix->height());
  p.drawLine(pix->width(), 0, 0, pix->height());
  p.end();
}

BuildablesModel::BuildablesModel(city *c, QObject *parent)
  : QAbstractListModel(parent)
  , m_city(c)
{
  universal targets[MAX_NUM_PRODUCTION_TARGETS];
  auto numTargets = collect_eventually_buildable_targets(targets, m_city, true);
  item props[numTargets];
  name_and_sort_items(targets, numTargets, props, false, m_city);

  for (int row = 0; row < numTargets; row++) {
    m_targets << cid_encode(props[row].item);
  }
}

int BuildablesModel::rowCount(const QModelIndex &/*index*/) const {
  return m_targets.size();
}

QVariant BuildablesModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  int row = index.row();


  if (role == Qt::ToolTipRole) {
    universal u = cid_decode(m_targets[row]);
    if (u.kind == VUT_UTYPE) {
      return unitTooltip(u.value.utype);
    }
    if (!improvement_has_flag(u.value.building, IF_GOLD)) {
      return improvementTooltip(u.value.building);
    }
    return QVariant();
  }

  if (role == Qt::UserRole) {
    // qCDebug(FC) << "BuildablesModel::data" << row;
    return m_targets[row];
  }

  return QVariant();
}

QString BuildablesModel::improvementTooltip(impr_type *building) const
{

  QString tip = "<p style='white-space:pre'>";

  tip += QString("<b>%1</b>\n")
      .arg(improvement_name_translation(building));

  int upkeep = building->upkeep;
  if (m_city != nullptr) {
    upkeep = city_improvement_upkeep(m_city, building);
  }

  tip += QString(_("Cost: %1, Upkeep: %2\n"))
             .arg(impr_build_shield_cost(m_city,building))
             .arg(upkeep);

  QString req = _("?tech:None");
  requirement_vector_iterate(&building->obsolete_by, pobs) {
    if (pobs->source.kind == VUT_ADVANCE) {
      req = advance_name_translation(pobs->source.value.advance);
      break;
    }
  } requirement_vector_iterate_end;

  tip += QString(_("Obsolete by: % 1\n"))
      .arg(req);

  return tip;
}

QString BuildablesModel::unitTooltip(unit_type *unit) const
{
  auto tip = QString("<b>%1</b>\n")
      .arg(utype_name_translation(unit));

  tip += "<table width='100\%'>";

  tip += QString("<tr><td><b>%1</b> %2</td><td><b>%3</b> %4</td><td><b>%5</b> %6</td></tr>")
      .arg(_("Attack:")).arg(unit->attack_strength)
      .arg(_("Defense:")).arg(unit->defense_strength)
      .arg(_("Move:")).arg(move_points_text(unit->move_rate, true));

  tip += QString("<tr><td><b>%1</b> %2</td><td colspan='2'><b>%3</b> %4</td><td><b>%5</b> %6</td></tr>")
      .arg(_("Cost:")).arg(utype_build_shield_cost_base(unit))
      .arg(_("Basic Upkeep:")).arg(helptext_unit_upkeep_str(unit));

  tip += QString("<tr><td><b>%1</b> %2</td><td><b>%3</b> %4</td><td><b>%5</b> %6</td></tr>")
      .arg(_("Hitpoints:")).arg(unit->hp)
      .arg(_("FirePower:")).arg(unit->firepower)
      .arg(_("Vision:")).arg(static_cast<int>(sqrt(unit->vision_radius_sq)));

  auto obsolete = unit->obsoleted_by;
  if (obsolete) {
    auto tech = obsolete->require_advance;
    if (tech && tech != advance_by_number(0)) {
      tip += QString("<tr><td colspan='3'>Obsoleted by %1 (%2)</td></tr>")
          .arg(utype_name_translation(obsolete))
          .arg(advance_name_translation(tech));
    } else {
      tip += QString("<tr><td colspan='3'>Obsoleted by %1</td></tr>")
          .arg(utype_name_translation(obsolete));
    }
  }

  tip += "</table><p style='white-space:pre'>";

  return tip;
}

BuildablesFilter::BuildablesFilter(city* c, int flags, QObject* parent)
  : QSortFilterProxyModel(parent)
  , m_city(c)
  , m_units((flags & Units) == Units)
  , m_buildings((flags & Buildings) == Buildings)
  , m_wonders((flags & Wonders) == Wonders)
  , m_future((flags & Future) == Future)
{}

bool BuildablesFilter::filterAcceptsRow(int row, const QModelIndex &/*parent*/) const {

  auto idx = sourceModel()->index(row, 0);
  cid id = sourceModel()->data(idx, Qt::UserRole).toInt();
  universal u = cid_decode(id);

  if (!m_future && !can_city_build_now(m_city, &u)) return false;
  if (m_units && u.kind == VUT_UTYPE) return true;
  if (u.kind != VUT_UTYPE) {
    if (m_wonders && is_improvement(u.value.building)) return true;
    if (m_buildings && is_improvement(u.value.building)) return true;
    if (m_buildings && is_special_improvement(u.value.building)) return true;
    if (improvement_has_flag(u.value.building, IF_GOLD)) return true;
  }

  return false;

}

BuildablesTableModel::BuildablesTableModel(int columns, QObject* parent)
  : QAbstractProxyModel(parent)
  , m_columns(columns)
{}

int BuildablesTableModel::columnCount(const QModelIndex &parent) const {
  int sourceRows = sourceModel()->rowCount(parent);
  if (sourceRows < m_columns) {
    return sourceRows;
  }
  return m_columns;
}

int BuildablesTableModel::rowCount(const QModelIndex &parent) const {
  int sourceRows = sourceModel()->rowCount(parent);
  return (sourceRows + m_columns - 1) / m_columns;
}

QModelIndex BuildablesTableModel::mapFromSource(const QModelIndex &sourceIndex) const {
  if (sourceIndex.column() > 0) return QModelIndex();
  int col = sourceIndex.row() % m_columns;
  int row = sourceIndex.row() / m_columns;
  return createIndex(row, col);
}

QModelIndex BuildablesTableModel::mapToSource(const QModelIndex &proxyIndex) const {
  int row = proxyIndex.row() * m_columns + proxyIndex.column();
  if (row >= sourceModel()->rowCount()) return QModelIndex();
  return sourceModel()->index(row, 0);
}

QModelIndex BuildablesTableModel::parent(const QModelIndex &/*child*/) const {
  return QModelIndex();
}

QModelIndex BuildablesTableModel::index(int row, int column, const QModelIndex &parent) const {
  return createIndex(row, column);
}

BuildablesDelegate::BuildablesDelegate(city* c, QSize hint, QObject* parent)
  : QItemDelegate(parent)
  , m_city(c)
  , m_hint(hint)
{}

void BuildablesDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
  if (!option.rect.isValid()) {
    // qCDebug(FC) << "BuildablesDelegate: invalid rect";
    return;
  }
  if (!index.isValid()) {
    // qCDebug(FC) << "BuildablesDelegate: invalid index";
    return;
  }


  // auto opt = QItemDelegate::setOptions(index, option);
  painter->save();
  // opt.displayAlignment = Qt::AlignLeft;
  // opt.textElideMode = Qt::ElideMiddle;
  QItemDelegate::drawBackground(painter, option, index);

  QString name;
  QPixmap pix;
  bool useless = false;
  bool is_coinage = false;
  bool is_neutral = false;
  bool is_sea = false;
  bool is_flying = false;
  bool is_unit = false;

  auto d = index.data(Qt::UserRole);
  if (!d.isValid()) {
    QIcon icon = qApp->style()->standardIcon(QStyle::SP_DialogCancelButton);
    pix = icon.pixmap(100, 100);
    name = _("Cancel");
  } else {
    auto u = cid_decode(d.toInt());
    if (u.kind == VUT_UTYPE) {
      name = utype_name_translation(u.value.utype);
      pix = get_unittype_sprite(get_tileset(), u.value.utype,
                                direction8_invalid())->pm;
      auto flags = getUnitFlags(u.value.utype);
      is_neutral = (flags & Neutral) == Neutral;
      is_sea = (flags & Sea) == Sea;
      is_flying = (flags & Flying) == Flying;
      is_unit = true;
    } else {
      name = improvement_name_translation(u.value.building);
      pix = get_building_sprite(get_tileset(), u.value.building)->pm;
      useless = is_improvement_redundant(m_city, u.value.building);
      is_coinage = improvement_has_flag(u.value.building, IF_GOLD);
    }
  }
  pix = pix.scaledToHeight(m_hint.height() - 2, Qt::SmoothTransformation);
  if (useless) {
    pixmap_put_x(&pix);
  }

  auto rect = option.rect;
  rect.setLeft(rect.left() + pix.width() + 4);
  rect.setTop(rect.top() + (rect.height() - painter->fontMetrics().height()) / 2);
  QItemDelegate::drawDisplay(painter, option, rect, name);

  QPixmap deco(option.rect.width(), option.rect.height());

  if (is_unit) {
    if (is_sea) {
      deco.fill(QColor(0, 0, 255, 80));
    } else if (is_flying) {
      deco.fill(QColor(220, 0, 0, 80));
    } else if (is_neutral) {
      deco.fill(QColor(0, 120, 0, 40));
    } else {
      deco.fill(QColor(0, 0, 150, 40));
    }

    QItemDelegate::drawDecoration(painter, option, option.rect, deco);
  }

  if (is_coinage) {
    deco.fill(QColor(255, 255, 0, 70));
    QItemDelegate::drawDecoration(painter, option, option.rect, deco);
  }

  if (!pix.isNull()) {
    auto rect = option.rect;
    rect.setWidth(pix.width() + 4);
    QItemDelegate::drawDecoration(painter, option, rect, pix);
  }

  drawFocus(painter, option, option.rect);

  painter->restore();

}

int BuildablesDelegate::getUnitFlags(unit_type* ut) const {
  int flags = 0;

  if (utype_has_flag(ut, UTYF_CIVILIAN)) {
    flags |= Neutral;
  }

  auto c = utype_class(ut);
  if (!uclass_has_flag(c, UCF_TERRAIN_DEFENSE)
    && !uclass_has_flag(c, UCF_CAN_FORTIFY)
    && !uclass_has_flag(c, UCF_ZOC)) {
    flags |= Sea;
  }

  if ((utype_fuel(ut)
     && !uclass_has_flag(c, UCF_TERRAIN_DEFENSE)
     && !uclass_has_flag(c, UCF_CAN_PILLAGE)
     && !uclass_has_flag(c, UCF_CAN_FORTIFY)
     && !uclass_has_flag(c, UCF_ZOC))
    /* FIXME: Assumed to be flying since only missiles can do suicide
     * attacks in classic-like rulesets. This isn't true for all
     * rulesets. Not a high priority to fix since all is_flying and
     * is_sea is used for is to set a color. */
    || utype_can_do_action(ut, ACTION_SUICIDE_ATTACK)) {
    flags &= ~Sea;
    flags |= Flying;
  }

  return flags;
}

void BuildablesDelegate::drawFocus(QPainter *painter,
    const QStyleOptionViewItem &option,
    const QRect &rect) const
{

  if ((option.state & QStyle::State_MouseOver) == 0 || !rect.isValid()) {
    return;
  }

  QPixmap pix(rect.width(), rect.height());
  pix.fill(QColor(50, 50, 50, 50));
  QItemDelegate::drawDecoration(painter, option, rect, pix);
}

QSize BuildablesDelegate::sizeHint(const QStyleOptionViewItem &/*option*/,
    const QModelIndex &/*index*/) const
{
  return m_hint;
}




Buildables::Buildables(city* c, quint64 flags, QWidget* parent)
  : QTableView(parent)
  , m_city(c)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(Qt::Popup);

  verticalHeader()->setVisible(false);
  horizontalHeader()->setVisible(false);
  setProperty("showGrid", false);

  installEventFilter(this);

  auto buildables = new BuildablesModel(m_city, this);
  auto filter = new BuildablesFilter(m_city, flags, this);
  filter->setSourceModel(buildables);
  auto table = new BuildablesTableModel(3, this);
  table->setSourceModel(filter);
  setModel(table);
  auto hint = computeCellSize(filter);
  auto delegate = new BuildablesDelegate(m_city, hint, this);
  setItemDelegate(delegate);

  connect(selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &Buildables::buildingSelected);

  resizeRowsToContents();
  resizeColumnsToContents();
  setFixedWidth(3 * hint.width() + 6);
  setFixedHeight(table->rowCount() * hint.height() + 6);

  int dw = QApplication::desktop()->width();
  if (width() > dw) {
    setFixedWidth(dw);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  } else {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }

  int dh = QApplication::desktop()->height();
  if (height() > dh) {
    setFixedHeight(dh);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  } else {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  }

  auto pos = QCursor::pos();

  if (pos.x() + width() > dw) {
    pos.setX(dw - width());
  } else if (pos.x() - width() < 0) {
    pos.setX(0);
  }

  if (pos.y() + height() > dh) {
    pos.setY(dh - height());
  } else if (pos.y() - height() < 0) {
    pos.setY(0);
  }

  move(pos);
  setMouseTracking(true);
  setFocus();
}

QSize Buildables::computeCellSize(BuildablesFilter *filter) const {

  int h = fontMetrics().height() * 2;
  int w = 0;

  int numRows = filter->rowCount();
  for (int row = 0; row < numRows; row++) {
    auto u = cid_decode(filter->data(filter->index(row, 0), Qt::UserRole).toInt());
    if (u.kind == VUT_UTYPE) {
      w = qMax(w, fontMetrics().width(utype_name_translation(u.value.utype)));
    } else {
      w = qMax(w, fontMetrics().width(improvement_name_translation(u.value.building)));
    }
  }

  w += 2 * h;
  w = qMin(w, 250);

  return QSize(w, h);
}


bool Buildables::eventFilter(QObject *object, QEvent *event) {

  if (object != this) return false;

  if (event->type() == QEvent::MouseButtonPress) {
    QRect rect;
    rect.setTopLeft(pos());
    rect.setBottomRight(pos() + QPoint(width(), height()));

    if (!rect.contains(QCursor::pos())) {
      close();
      return true;
    }
    return false;
  }


  if (event->type() == QEvent::ToolTip) {

    auto helpEvent = static_cast<QHelpEvent*>(event);
    QPoint pos = helpEvent->pos();
    QModelIndex index = indexAt(pos);

    if (!index.isValid()) return false;

    auto tip = model()->data(index, Qt::ToolTipRole).toString();
    auto rect = visualRect(index);
    rect.setTopLeft(rect.topLeft() + helpEvent->globalPos());

    if (!tip.isEmpty()) {
      QToolTip::showText(helpEvent->globalPos(), tip, this, rect);
    } else {
      QToolTip::hideText();
    }

    return true;
  }

  return false;
}

void Buildables::buildingSelected(const QItemSelection &sl,
                                  const QItemSelection &ds)
{
  // qCDebug(FC) << "buildingSelected";

  QModelIndexList indexes = selectionModel()->selectedIndexes();

  if (indexes.isEmpty() || client_is_observer()) return;


  auto d = indexes[0].data(Qt::UserRole);
  if (!d.isValid()) return;

  auto u = cid_decode(d.toInt());

  emit selected(u);
}



