#include "unitlistwidget.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include "sprite.h"
#include "canvas.h"
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QTimer>
#include "logging.h"

#include "client_main.h"
#include "tilespec.h"
#include "text.h"
#include "mapview_common.h"
#include "control.h"
extern "C" {
#include "dialogs_g.h"
#include "citydlg_g.h"
}

using namespace KV;

UnitListWidget::UnitListWidget(QWidget *parent)
  : QScrollArea(parent)
  , m_lay(new QHBoxLayout)
{
  setWidget(new QWidget);
  setWidgetResizable(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  widget()->setLayout(m_lay);
  setContentsMargins(0, 0, 0, 0);

  m_timer = new QTimer(this);
  m_timer->setInterval(1000/25);
  connect(m_timer, &QTimer::timeout, this, &UnitListWidget::slide);
}



void UnitListWidget::changeCity(city *c) {
  m_city = c;
  qDeleteAll(widget()->children());
  if (m_city == nullptr) return;
  m_lay = new QHBoxLayout;
  m_lay->setSizeConstraint(QLayout::SetMinimumSize);
  m_lay->setContentsMargins(0, 0, 0, 0);
  m_lay->setSpacing(0);
  widget()->setLayout(m_lay);
  createUnits();
  setMinimumHeight(m_minHeight);
}


static float gravity(int dx) {
  float threshold = 2;
  float k = 1.0;
  float cutoff = 50;

  if (abs(dx) >= cutoff) return dx / abs(dx) * k * cutoff ;
  if (abs(dx) >= threshold) return k * dx;

  return 0;
}

void UnitListWidget::mousePressEvent(QMouseEvent *event) {
  // qCDebug(FC) << "button press";
  m_dx = 0;
  m_lastX = event->x();
}

void UnitListWidget::mouseReleaseEvent(QMouseEvent */*event*/) {
  // qCDebug(FC) << "button release" << m_dx;
  if (m_dx == 0) {
    m_timer->stop();
    return;
  }
  m_timer->start();
}

void UnitListWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    m_dx = event->x() - m_lastX;
    slide();
    m_dx = gravity(m_dx);
    m_lastX = event->x();
  }
}


void UnitListWidget::slide() {
  int dx = m_dx;

  QPoint p = widget()->pos();
  int w = widget()->width();
  int w_view = viewport()->width();


  if (p.x() + dx  + w < w_view) {
    dx = 0;
    p.setX(w_view - w);
    m_timer->stop();
  } else if (p.x() + dx > 0) {
    dx = 0;
    p.setX(0);
    m_timer->stop();
  }

  widget()->move(p.x() + dx, p.y());
}

SupportedUnitsWidget::SupportedUnitsWidget(QWidget *parent)
  : UnitListWidget(parent)
{}


void SupportedUnitsWidget::createUnits() {

  unit_list *units;
  if (client_has_player() && client_player() != city_owner(m_city)) {
    units = m_city->client.info_units_supported;
  } else {
    units = m_city->units_supported;
  }


  auto label = new QLabel;
  label->setText(QString(_("Supported: %1")).arg(unit_list_size(units)));
  m_lay->addWidget(label);

  int h = fontMetrics().height();

  unit_list_iterate(units, punit) {
    int bonus = get_city_bonus(m_city, EFT_MAKE_CONTENT_MIL);
    int happyCost = city_unit_unhappiness(punit, &bonus);
    auto unit = new SupportedUnitItem(punit, happyCost);
    h = qMax(h, unit->pixmap()->height());
    m_lay->addWidget(unit);
  } unit_list_iterate_end;

  m_minHeight = h;
}

PresentUnitsWidget::PresentUnitsWidget(QWidget *parent)
  : UnitListWidget(parent)
{}


void PresentUnitsWidget::createUnits() {
  unit_list *units;
  if (client_has_player() && client_player() != city_owner(m_city)) {
    units = m_city->client.info_units_present;
  } else {
    units = m_city->tile->units;
  }

  auto label = new QLabel;
  label->setText(QString(_("Present: %1")).arg(unit_list_size(units)));
  m_lay->addWidget(label);

  int h = fontMetrics().height();

  unit_list_iterate(units, punit) {
    auto unit = new PresentUnitItem(punit);
    h = qMax(h, unit->pixmap()->height());
    m_lay->addWidget(unit);
  } unit_list_iterate_end;

  m_minHeight = h;
}


UnitItem::UnitItem(unit* punit, QWidget *parent)
  : QLabel(parent)
  , m_unit(punit)
{}

SupportedUnitItem::SupportedUnitItem(unit *punit, int happy_cost, QWidget *parent)
  : UnitItem(punit, parent)
{
  if (m_unit == nullptr) return;

  auto c = canvas_create(tileset_full_tile_width(get_tileset()),
                         tileset_unit_with_upkeep_height(get_tileset()));
  c->map_pixmap.fill(Qt::transparent);
  put_unit(m_unit, c, 1, 0, 0);
  put_unit_city_overlays(m_unit, c, 0,
                         tileset_unit_layout_offset_y(get_tileset()),
                         m_unit->upkeep, happy_cost);

  setFixedWidth(c->map_pixmap.width());
  setFixedHeight(c->map_pixmap.height());
  setToolTip(unit_description(m_unit));
  m_pix = c->map_pixmap;
  canvas_free(c);
  setPixmap(m_pix);
  m_hpix = QPixmap(m_pix.size());
  m_hpix.fill(QColor(Qt::transparent));
  QPainter p;
  p.begin(&m_hpix);
  int pw = 3;
  QPen pen(QColor(palette().color(QPalette::Highlight)), pw);
  p.setPen(pen);
  p.drawRoundedRect(QRect(pw, pw, m_pix.width() - 2 * pw, m_pix.height() - 2 * pw), 15, 15, Qt::RelativeSize);
  p.drawPixmap(0, 0, m_pix);
  p.end();
}


PresentUnitItem::PresentUnitItem(unit *punit, QWidget *parent)
  : UnitItem(punit, parent)
{
  if (m_unit == nullptr) return;

  auto c = canvas_create(tileset_full_tile_width(get_tileset()),
                         tileset_unit_height(get_tileset()));
  c->map_pixmap.fill(Qt::transparent);
  put_unit(m_unit, c, 1, 0, 0);
  setFixedWidth(c->map_pixmap.width() + 4);
  setFixedHeight(c->map_pixmap.height());
  setToolTip(unit_description(m_unit));
  m_pix = c->map_pixmap;
  canvas_free(c);
  setPixmap(m_pix);
  m_hpix = QPixmap(m_pix.size());
  m_hpix.fill(QColor(Qt::transparent));
  QPainter p;
  p.begin(&m_hpix);
  int pw = 3;
  QPen pen(QColor(palette().color(QPalette::Highlight)), pw);
  p.setPen(pen);
  p.drawRoundedRect(QRect(pw, pw, m_pix.width() - 2 * pw, m_pix.height() - 2 * pw), 15, 15, Qt::RelativeSize);
  p.drawPixmap(0, 0, m_pix);
  p.end();
}


void UnitItem::contextMenuEvent(QContextMenuEvent *event)
{
  if (m_unit == nullptr) return;
  if (!can_client_issue_orders()) return;
  if (client_has_player() && client_player() != unit_owner(m_unit)) return;

  auto menu = new QMenu(this);

  QAction* a;

  if (can_unit_do_activity(m_unit, ACTIVITY_SENTRY)) {
    a = new QAction(_("Sentry unit"));
    connect(a, &QAction::triggered, this, [=] () {
      request_unit_sentry(m_unit);
    });
    menu->addAction(a);
  }

  if (can_unit_do_activity(m_unit, ACTIVITY_FORTIFYING)) {
    a = new QAction(_("Fortify unit"));
    connect(a, &QAction::triggered, this, [=] () {
      request_unit_fortify(m_unit);
    });
    menu->addAction(a);
  }

  if (unit_can_do_action(m_unit, ACTION_DISBAND_UNIT)) {
    a = new QAction(_("Disband unit"));
    connect(a, &QAction::triggered, this, [=] () {
      auto punit = player_unit_by_number(client_player(), m_unit->id);
      if (punit == nullptr) return;
      auto punits = unit_list_new();
      unit_list_append(punits, punit);
      popup_disband_dialog(punits);
      unit_list_destroy(punits);
    });
    menu->addAction(a);
  }

  if (can_unit_change_homecity(m_unit)) {
    a = new QAction(action_id_name_translation(ACTION_HOME_CITY));
    connect(a, &QAction::triggered, this, [=] () {
      request_unit_change_homecity(m_unit);
    });
    menu->addAction(a);
  }

  auto units = unit_list_new();
  unit_list_append(units, m_unit);

  if (units_can_load(units)) {
    a = new QAction(_("Load"));
    connect(a, &QAction::triggered, this, [=] () {
      request_transport(m_unit, unit_tile(m_unit));
    });
    menu->addAction(a);
  }

  if (units_can_unload(units)) {
    a = new QAction(_("Unload"));
    connect(a, &QAction::triggered, this, [=] () {
      request_unit_unload(m_unit);
    });
    menu->addAction(a);
  }

  if (units_are_occupied(units)) {
    a = new QAction(_("Unload All From Transporter"));
    connect(a, &QAction::triggered, this, [=] () {
      request_unit_unload_all(m_unit);
    });
    menu->addAction(a);
  }

  if (units_can_upgrade(units)) {
    a = new QAction(_("Upgrade Unit"));
    connect(a, &QAction::triggered, this, [=] () {
      auto units = unit_list_new();
      unit_list_append(units, m_unit);
      popup_upgrade_dialog(units);
      unit_list_destroy(units);
    });
    menu->addAction(a);
  }

  unit_list_destroy(units);


  menu->popup(event->globalPos());
}


void UnitItem::enterEvent(QEvent */*event*/)
{
  setPixmap(m_hpix);
}

void UnitItem::leaveEvent(QEvent */*event*/)
{
  setPixmap(m_pix);
}

void UnitItem::mouseDoubleClickEvent(QMouseEvent *event)
{
  if (m_unit == nullptr) return;

  if (event->button() == Qt::LeftButton) {
    // qCDebug(FC) << "mouse double click in unit";
    handleEnterKey();
    event->accept();
    return;
  }
  event->ignore();
}

void UnitItem::handleEnterKey() {
  qCDebug(FC) << "handleEnterKey";
  unit_focus_set(m_unit);
  popdown_all_city_dialogs();
}
