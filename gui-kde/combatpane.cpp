#include "combatpane.h"
#include "conf_combatpane.h"
#include "application.h"
#include "logging.h"
#include <QVBoxLayout>
#include <QTableWidget>
#include "canvas.h"
#include <QHeaderView>
#include <QScrollBar>
#include "combatpaneconfigdialog.h"

#include "game.h"
#include "mapview_common.h"


using namespace KV;

CombatPane::CombatPane()
  : IOutputPane()
  , m_combats(new QTableWidget)
{

  m_combats->setSelectionMode(QAbstractItemView::NoSelection);
  // turn, attacker, attacker result, location, defender result, defender
  m_combats->setColumnCount(6);
  m_combats->horizontalHeader()->setVisible(false);
  m_combats->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_combats->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_combats->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_combats->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  m_combats->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
  m_combats->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
  m_combats->verticalHeader()->setVisible(false);

  m_combats->setTextElideMode(Qt::ElideMiddle);

  m_combats->setToolTip("Legend:\n"
                        "Column 1: Turn\n"
                        "Column 2: Attacker (gray background means dead unit,\n"
                        "  boldface text means acquired veteran status).\n"
                        "Column 3: Attacker health loss.\n"
                        "Column 4: Location.\n"
                        "Column 5: Defender health loss.\n"
                        "Column 6: Defender (gray background means dead unit,\n"
                        "  boldface text means acquired veteran status).\n"
                        );

  connect(Application::instance(), &Application::combatInfo,
          this, &CombatPane::addCombat);

  m_maxLines = Conf::CombatPane::combatsCount();
}

void CombatPane::addCombat(const Hostile &att, const Hostile &def) {
  int row = m_combats->rowCount();
  m_combats->insertRow(row);
  int h = m_combats->fontMetrics().height() + 30;
  // turn
  auto item = new QTableWidgetItem(QString("%1").arg(game.info.turn));
  m_combats->setItem(row, 0, item);
  // attacker
  auto a = game_unit_by_number(att.id);
  if (a == nullptr) {
    m_combats->setItem(row, 1, new QTableWidgetItem("N/A"));
  } else {
    auto name = QString("%1 %2 %3")
        .arg(nation_adjective_translation(nation_of_unit(a)))
        .arg(unit_name_translation(a))
        .arg(att.id);
    item = new QTableWidgetItem(name);

    auto c = canvas_create(tileset_unit_width(get_tileset()),
                           tileset_unit_height(get_tileset()));
    if (att.health == 0) {
      c->map_pixmap.fill(Qt::gray);
    } else {
      c->map_pixmap.fill(Qt::transparent);
    }
    put_unit(a, c,  1.0, 0, 0);
    auto pix = Application::SaneMargins(c->map_pixmap, QSize(80, 80));
    pix = pix.scaledToHeight(h);
    item->setData(Qt::DecorationRole, pix);

    QSize s(pix.width() + m_combats->fontMetrics().width(name) + 6, h);
    item->setSizeHint(s);
    m_combats->setItem(row, 1, item);
  }
  // attacker result
  if (a == nullptr) {
    m_combats->setItem(row, 2, new QTableWidgetItem("N/A"));
  } else {
    item = new QTableWidgetItem(QString("%1").arg(att.health - a->hp));
    item->setTextAlignment(Qt::AlignRight);
    m_combats->setItem(row, 2, item);
  }
  if (att.veteran) {
    auto f = m_combats->font();
    f.setBold(true);
    m_combats->item(row, 1)->setFont(f);
  }
  // defender
  auto d = game_unit_by_number(def.id);
  if (d == nullptr) {
    m_combats->setItem(row, 5, new QTableWidgetItem("N/A"));
  } else {
    auto name = QString("%1 %2 %3")
        .arg(nation_adjective_translation(nation_of_unit(d)))
        .arg(unit_name_translation(d))
        .arg(def.id);

    item = new QTableWidgetItem(name);

    auto c = canvas_create(tileset_unit_width(get_tileset()),
                           tileset_unit_height(get_tileset()));
    if (def.health == 0) {
      c->map_pixmap.fill(Qt::gray);
    } else {
      c->map_pixmap.fill(Qt::transparent);
    }

    put_unit(d, c,  1.0, 0, 0);
    auto pix = Application::SaneMargins(c->map_pixmap, QSize(80, 80));
    pix = pix.scaledToHeight(h);
    item->setData(Qt::DecorationRole, pix);

    QSize s(pix.width() + m_combats->fontMetrics().width(name) + 6, h);
    item->setSizeHint(s);
    m_combats->setItem(row, 5, item);
  }
  // defender result
  if (d == nullptr) {
    m_combats->setItem(row, 4, new QTableWidgetItem("N/A"));
  } else {
    item = new QTableWidgetItem(QString("%1").arg(def.health - d->hp));
    item->setTextAlignment(Qt::AlignRight);
    m_combats->setItem(row, 4, item);
  }
  if (def.veteran) {
    auto f = m_combats->font();
    f.setBold(true);
    m_combats->item(row, 5)->setFont(f);
  }
  // location
  if (a == nullptr && d == nullptr) {
    m_combats->setItem(row, 3, new QTableWidgetItem("N/A"));
  } else {
    tile* t;
    if (a == nullptr && d != nullptr) {
      t = unit_tile(d);
    } else if (a != nullptr && d == nullptr) {
      t = unit_tile(a);
    } else if (def.health == 0) {
      t = unit_tile(a);
    } else {
      t = unit_tile(d);
    }
    auto c = nearestCity(t);
    QString loc = "Location";
    if (c != nullptr) {
      int dist = sq_map_distance(t, c->tile);
      if (dist == 0) {
        loc = QString("In %1").arg(city_name_get(c));
      } else {
        loc = QString("Near %1").arg(city_name_get(c));
      }
    }
    auto link = QString("<a href=%1>%2</a>").arg(tile_index(t)).arg(loc);
    auto label = new QLabel(link);
    label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    label->setTextFormat(Qt::RichText);
    connect(label, &QLabel::linkActivated, this, [] (const QString& s) {
      int id = s.toInt();
      center_tile_mapcanvas(index_to_tile(&(wld.map), id));
      link_mark_restore(TLT_TILE, id);
    });
    item = new QTableWidgetItem;
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    m_combats->setItem(row, 3, item);
    m_combats->setCellWidget(row, 3, label);
  }

  if (m_maxLines > 0) {
    while (m_combats->rowCount() > m_maxLines) {
      m_combats->removeRow(0);
    }
  }

  auto bar = m_combats->verticalScrollBar();
  bar->setSliderPosition(bar->maximum());

  emit flash();
}


QWidget* CombatPane::outputWidget(QWidget */*parent*/) {
  return m_combats;
}

QVector<QWidget*> CombatPane::toolBarWidgets() const {
  return {};
}

QString CombatPane::displayName() const {
  return tr("Combats");
}

int CombatPane::priorityInStatusBar() const {
  return 50;
}

void CombatPane::clearContents() {
  m_combats->clearContents();
  m_combats->setRowCount(0);
}

void CombatPane::refreshContents() {
  // noop
}

void CombatPane::configureOutput() {
  auto d = new CombatPaneConfigDialog(m_combats);
  if (d->exec() == QDialog::Accepted) {
    m_maxLines = d->combatCount();
    Conf::CombatPane::setCombatsCount(m_maxLines);
    Conf::CombatPane::self()->save();
  }
}

void CombatPane::visibilityChanged(bool /*visible*/) {
  // noop
}

bool CombatPane::canConfigure() const {
  return true;
}

bool CombatPane::canRefresh() const {
  return false;
}

city* KV::nearestCity(const tile *t) {
  quint32 mindist = 0xffffffff;
  city* nearest;
  players_iterate(pplayer) {
    city_list_iterate(pplayer->cities, c) {
      int d = sq_map_distance(c->tile, t);
      if (d == 0) return c;
      if (d < mindist) {
        nearest = c;
        mindist = d;
      }
    } city_list_iterate_end;
  } players_iterate_end;
  return nearest;
}
