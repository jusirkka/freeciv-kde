#include "unitlistwidget.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include "sprite.h"
#include "canvas.h"
#include <QContextMenuEvent>
#include <QMouseEvent>

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
  : QFrame(parent)
  , m_lay(new QHBoxLayout)
{

  setLayout(m_lay);
}



void UnitListWidget::changeCity(city *c) {
  m_city = c;
  qDeleteAll(children());
  if (m_city == nullptr) return;
  m_lay = new QHBoxLayout;
  createUnits();
  setLayout(m_lay);
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

  unit_list_iterate(units, punit) {
    int bonus = get_city_bonus(m_city, EFT_MAKE_CONTENT_MIL);
    int happyCost = city_unit_unhappiness(punit, &bonus);
    m_lay->addWidget(new SupportedUnitItem(punit, happyCost));
  } unit_list_iterate_end;

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

  unit_list_iterate(units, punit) {
    m_lay->addWidget(new PresentUnitItem(punit));
  } unit_list_iterate_end;


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

  setFixedWidth(c->map_pixmap.width() + 4);
  setFixedHeight(c->map_pixmap.height());
  setToolTip(unit_description(m_unit));
  m_pix = c->map_pixmap;
  canvas_free(c);
  setPixmap(m_pix);
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
  QPixmap pix(m_pix.size());
  QPainter p;
  p.begin(&pix);
  p.fillRect(0, 0, m_pix.width(), m_pix.height(),
             QColor(palette().color(QPalette::Highlight)));
  p.drawPixmap(0, 0, m_pix);
  p.end();

  setPixmap(pix);
}

void UnitItem::leaveEvent(QEvent */*event*/)
{
  setPixmap(m_pix);
}

void UnitItem::mousePressEvent(QMouseEvent *event)
{
  if (m_unit == nullptr) return;
  if (event->button() == Qt::LeftButton) {
    unit_focus_set(m_unit);
    popdown_all_city_dialogs();
  }
}

