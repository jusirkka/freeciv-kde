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
#include <QMimeData>
#include "proxytablemodel.h"

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
  if (m_city != nullptr) {
    changeCity(m_city);
  }
}

int BuildablesModel::rowCount(const QModelIndex &/*index*/) const {
  return m_targets.size();
}

QVariant BuildablesModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) return QVariant();

  int row = index.row();

  if (role == Qt::UserRole) {
    return m_targets[row];
  }

  universal u = cid_decode(m_targets[row]);

  if (role == Qt::DisplayRole) {
    if (u.kind == VUT_UTYPE) {
      return QString(utype_name_translation(u.value.utype));
    }
    return QString(improvement_name_translation(u.value.building));
  }

  if (role == Qt::DecorationRole) {
    int h = QApplication::fontMetrics().height() + 6;
    QPixmap pix;
    if (u.kind == VUT_UTYPE) {
      pix = get_unittype_sprite(get_tileset(),
                                u.value.utype,
                                direction8_invalid())->pm;
    } else {
      pix = get_building_sprite(get_tileset(), u.value.building)->pm;
      if (is_improvement_redundant(m_city, u.value.building)) {
        pixmap_put_x(&pix);
      }
    }
    return pix.scaledToHeight(h);
  }

  if (role == Qt::BackgroundRole) {
    return getItemColor(u);
  }

  if (role == Qt::ToolTipRole) {
    if (u.kind == VUT_UTYPE) {
      return unitTooltip(u.value.utype);
    }
    if (!improvement_has_flag(u.value.building, IF_GOLD)) {
      return improvementTooltip(u.value.building);
    }
    return QVariant();
  }


  return QVariant();
}


QColor BuildablesModel::getItemColor(const universal &u) const {

  if (u.kind == VUT_UTYPE) {
    auto ut = u.value.utype;
    auto c = utype_class(ut);

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
      return QColor(220, 0, 0, 80);
    }

    if (!uclass_has_flag(c, UCF_TERRAIN_DEFENSE)
      && !uclass_has_flag(c, UCF_CAN_FORTIFY)
      && !uclass_has_flag(c, UCF_ZOC)) {
      return QColor(0, 0, 255, 80);
    }

    if (utype_has_flag(ut, UTYF_CIVILIAN)) {
      return QColor(0, 120, 0, 40);
    }

    return QColor(0, 0, 150, 40);

  }

  if (improvement_has_flag(u.value.building, IF_GOLD)) {
    return QColor(255, 255, 0, 70);
  }

  return QColor(Qt::transparent);
}

void BuildablesModel::changeCity(city *c) {
  beginResetModel();
  m_city = c;
  m_targets.clear();
  if (m_city != nullptr) {
    universal targets[MAX_NUM_PRODUCTION_TARGETS];
    auto numTargets = collect_eventually_buildable_targets(targets, m_city, true);
    item props[numTargets];
    name_and_sort_items(targets, numTargets, props, false, m_city);

    for (int row = 0; row < numTargets; row++) {
      m_targets << cid_encode(props[row].item);
    }
  }
  endResetModel();
}

QString BuildablesModel::improvementTooltip(impr_type *building) const
{

  if (m_city == nullptr) return "";

  QString tip = "<p style='white-space:pre'>";

  tip += QString("<b>%1</b>\n")
      .arg(improvement_name_translation(building));

  int upkeep = building->upkeep;
  upkeep = city_improvement_upkeep(m_city, building);

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

  tip += QString(_("Obsoleted by: %1\n"))
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
  , m_flags(flags)
{
  m_flags = flags;
}


void BuildablesFilter::setFilterFlags(int flags) {
  beginResetModel();
  m_flags = flags;
  endResetModel();
}

int BuildablesFilter::filterFlags() const {
  return m_flags;
}

bool BuildablesFilter::filterAcceptsRow(int row, const QModelIndex &/*parent*/) const {

  if (m_city == nullptr) return true;

  auto idx = sourceModel()->index(row, 0);
  cid id = sourceModel()->data(idx, Qt::UserRole).toInt();
  universal u = cid_decode(id);

  if (!(m_flags & Future) && !can_city_build_now(m_city, &u)) return false;
  if ((m_flags & Units) && u.kind == VUT_UTYPE) return true;
  if (u.kind != VUT_UTYPE) {
    if ((m_flags & Wonders) && is_wonder(u.value.building)) return true;
    if ((m_flags & Buildings) && is_improvement(u.value.building)) return true;
    if ((m_flags & Buildings) && is_special_improvement(u.value.building)) return true;
    if (improvement_has_flag(u.value.building, IF_GOLD)) return true;
  }

  return false;

}

void BuildablesFilter::changeCity(city* c) {
  auto src = qobject_cast<BuildablesModel*>(sourceModel());
  if (src) { // source model takes care of resetting
    m_city = c;
    src->changeCity(c);
  } else {
    beginResetModel();
    m_city = c;
    endResetModel();
  }
}

const char* BuildablesDragModel::CidMimeType = "freeciv/cid";

BuildablesDragModel::BuildablesDragModel(QObject *parent)
  : QIdentityProxyModel(parent)
  , m_mimeTypes{CidMimeType}
{}



QMimeData* BuildablesDragModel::mimeData(const QModelIndexList &indices) const {
  auto data = new QMimeData;
  QByteArray encoded;
  QDataStream stream(&encoded, QIODevice::WriteOnly);

  for (auto idx: indices) {
    auto sourceIndex = mapToSource(idx);
    if (sourceIndex.isValid()) {
      cid id = sourceIndex.data(Qt::UserRole).value<cid>();
      stream << id;
    }
  }
  data->setData(CidMimeType, encoded);

  return data;
}


QStringList BuildablesDragModel::mimeTypes() const {
  return m_mimeTypes;
}

Qt::ItemFlags BuildablesDragModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags defaultFlags = QIdentityProxyModel::flags(index);

  if (index.isValid()) {
    return Qt::ItemIsDragEnabled | defaultFlags;
  }

  return defaultFlags;
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

  auto d = index.data(Qt::DecorationRole);
  if (!d.isValid()) return;

  auto opt = QItemDelegate::setOptions(index, option);
  painter->save();
  opt.displayAlignment = Qt::AlignLeft;
  opt.textElideMode = Qt::ElideMiddle;
  QItemDelegate::drawBackground(painter, opt, index);

  auto pix = d.value<QPixmap>();
  pix = pix.scaledToHeight(m_hint.height() - 2, Qt::SmoothTransformation);

  auto rect = option.rect;
  rect.setLeft(rect.left() + pix.width() + 4);
  rect.setTop(rect.top() + (rect.height() - painter->fontMetrics().height()) / 2);
  auto name = index.data(Qt::DisplayRole).toString();
  QItemDelegate::drawDisplay(painter, opt, rect, name);

  QPixmap deco(option.rect.width(), option.rect.height());
  auto c = index.data(Qt::BackgroundRole).value<QColor>();
  deco.fill(c);

  QItemDelegate::drawDecoration(painter, option, option.rect, deco);

  rect = option.rect;
  rect.setWidth(pix.width() + 4);
  QItemDelegate::drawDecoration(painter, option, rect, pix);

  drawFocus(painter, option, option.rect);

  painter->restore();
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
  auto table = new ProxyTableModel(3, this);
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

void Buildables::buildingSelected(const QItemSelection &,
                                  const QItemSelection &)
{
  // qCDebug(FC) << "buildingSelected";

  QModelIndexList indexes = selectionModel()->selectedIndexes();

  if (indexes.isEmpty() || client_is_observer()) return;


  auto d = indexes[0].data(Qt::UserRole);
  if (!d.isValid()) return;

  auto u = cid_decode(d.toInt());

  emit selected(u);
}



