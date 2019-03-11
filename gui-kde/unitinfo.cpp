#include "unitinfo.h"
#include "mapview.h"
#include <QMouseEvent>
#include "application.h"
#include "logging.h"

#include "client_main.h"
#include "mapview_common.h"
#include "canvas.h"
#include "text.h"
#include "city.h"
#include "unittype.h"
#include "control.h"
#include "sprite.h"
extern "C" {
#include "dialogs_g.h"
}


using namespace KV;

UnitInfo::UnitInfo(QWidget* parent)
  : QWidget(parent)
  , m_unit(nullptr)
  , m_highlightPix(false)
  , m_numUnits(0)
{
  m_selectionArea.setWidth(0);

  setMouseTracking(true);

  m_arrow = get_arrow_sprite(tileset, ARROW_PLUS)->pm.scaledToHeight(50);

  setFixedWidth(0);
  setFixedHeight(52);

  m_font.setPixelSize(height() / 3);

  connect(Application::instance(), &Application::updateUnitInfo,
          this, [=] (void* p) {
    this->updateUnits(static_cast<unit_list*>(p));
  });

  updateUnits(get_units_in_focus());
}



void UnitInfo::updateUnits(unit_list *punits) {

  // qCDebug(FC) << "UnitInfo::updateUnits" << m_numUnits;

  m_numUnits = 0;
  m_unit = nullptr;

  if (punits == nullptr) return;

  if (unit_list_size(punits) == 0 || client_state() == C_S_OVER) {
    update();
    return;
  }


  m_unit = unit_list_get(punits, 0);
  // qCDebug(FC) << "UnitInfo::updateUnits" << unit_list_size(punits) << unit_list_size(unit_tile(m_unit)->units);
  if (unit_list_size(punits) == 1) {
    m_numUnits = unit_list_size(unit_tile(m_unit)->units);
  }

  if (m_numUnits == 0) {
    update();
    return;
  }

  m_label1 = get_unit_info_label_text1(punits);
  auto owner = unit_owner(m_unit);
  auto pcity = player_city_by_number(owner, m_unit->homecity);
  if (pcity != nullptr && unit_list_size(punits) == 1) {
    /* TRANS: unitX from cityZ */
    m_label1 = QString(_("%1 from %2"))
        .arg(get_unit_info_label_text1(punits), city_name_get(pcity));
  }
  /* TRANS: HP - hit points */
  m_label2 = QString(_("%1 HP:%2/%3"))
      .arg(unit_activity_text(m_unit))
      .arg(m_unit->hp)
      .arg(unit_type_get(m_unit)->hp);

  auto punit = head_of_units_in_focus();
  int w_width = 0;
  if (punit) {
    float zoom = (qobject_cast<MapView*>(parentWidget()))->zoomLevel();
    int w = tileset_unit_width(tileset);
    int h = tileset_unit_height(tileset);

    auto unitCanvas = canvas_create(w, h);
    unitCanvas->map_pixmap.fill(Qt::transparent);
    put_unit(punit, unitCanvas, zoom, 0, 0);
    m_pix = unitCanvas->map_pixmap.scaledToHeight(height());
    w_width += m_pix.width() + 1;

    auto tileCanvas = canvas_create(w, h);
    tileCanvas->map_pixmap.fill(QColor(0 , 0 , 0 , 85));
    put_terrain(punit->tile, tileCanvas, zoom, 0, 0);
    m_tile = tileCanvas->map_pixmap.scaledToHeight(height());
    w_width += m_tile.width() + 1;

    canvas_free(tileCanvas);
    canvas_free(unitCanvas);
  }

  QFontMetrics fm(m_font);
  w_width += qMax(fm.width(m_label1), fm.width(m_label2));
  m_skip = qMax(1.0, 0.1 * fm.height());

  if (m_numUnits > 1) {
    w_width += m_arrow.width() + 1;
  }
  setFixedWidth(w_width + 5);
  move(parentWidget()->width() - width(), 0);
  update();
}


void UnitInfo::mousePressEvent(QMouseEvent *event) {

  if (event->button() == Qt::LeftButton) {
    if (m_selectionArea.contains(event->x(), event->y())) {
      if (m_unit != nullptr && m_selectionArea.width() > 0) {
        unit_select_dialog_popup(unit_tile(m_unit));
      }
    }
  }
}

void UnitInfo::mouseMoveEvent(QMouseEvent *event) {
  bool redraw = false;
  // qCDebug(FC) << event->x() << event->y();
  if (m_selectionArea.contains(event->x(), event->y())) {
    redraw = !m_highlightPix;
    m_highlightPix = true;
  } else {
    redraw = m_highlightPix;
    m_highlightPix = false;
  }
  if (redraw) {
    update();
  }
}

void UnitInfo::updateArrowPix() {
  m_arrow = get_arrow_sprite(tileset, ARROW_PLUS)->pm.scaledToHeight(height());
}

void UnitInfo::paintEvent(QPaintEvent*)
{
  QPainter painter;

  painter.begin(this);

  QFontMetrics fm(m_font);

  QPen pen;
  pen.setWidth(1);
  pen.setColor(QColor(232, 255, 0));
  painter.setBrush(QColor(0, 0, 0, 135));
  painter.drawRect(0, 0, width(), height());
  painter.setFont(m_font);
  painter.setPen(pen);

  int w = 0;
  m_selectionArea.setWidth(0);
  QPainter::CompositionMode oldMode = painter.compositionMode();
  if (m_numUnits > 0) {
    painter.drawPixmap(w, (height() - m_pix.height()) / 2, m_pix);
    w += m_pix.width() + 1;
    if (m_numUnits > 1) {
      if (m_highlightPix) {
        painter.setCompositionMode(QPainter::CompositionMode_HardLight);
      }
      painter.drawPixmap(w, 0, m_arrow);
      m_selectionArea.setRect(w, 5, m_arrow.width(), m_arrow.height() - 10);
      w += m_arrow.width() + 1;
    }
    painter.setCompositionMode(oldMode);
    painter.drawText(w, m_skip + fm.height(), m_label1);
    painter.drawText(w, 2 * m_skip + 2 * fm.height(), m_label2);
    w += 5 + qMax(fm.width(m_label1), fm.width(m_label2));
    if (!m_tile.isNull()) {
      painter.drawPixmap(w, (height() - m_tile.height()) / 2, m_tile);
    }
  } else {
    painter.drawText(5, height() / 3 + 5, _("No units selected."));
  }

  painter.end();
}
