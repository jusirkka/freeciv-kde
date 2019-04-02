#include "mapview.h"
#include "application.h"
#include "logging.h"
#include "mainwindow.h"
#include <QTimer>
#include <QResizeEvent>
#include <QGraphicsItem>
#include <QApplication>
#include <QPaintEvent>
#include "unitinfo.h"
#include "gameinfo.h"
#include "endturnrect.h"
#include "minimapview.h"
#include "chatlineedit.h"
#include "tileinfo.h"
#include "buildables.h"
#include "unitselector.h"

#include "mapview_common.h"
#include "citydlg_common.h"
#include "sprite.h"
#include "canvas.h"
#include "mapctrl_common.h"
#include "client_main.h"
#include "climap.h"
#include "goto.h"


using namespace KV;

MapView::MapView(MainWindow* parent)
  : QWidget(parent)
  , m_cursor(-1)
  , m_cursorFrame(0)
  , m_dirties(m_maxCountDirties)
  , m_dirtyCount(0)
  , m_zoom(1.0)
  , m_mainWindow(parent)
  , m_unitSelector(nullptr)
{

  m_unitInfo = new UnitInfo(this);
  m_gameInfo = new GameInfo(this);
  m_endTurn = new EndTurnRect(this);
  m_minimap = new MinimapView(this);

  setAttribute(Qt::WA_OpaquePaintEvent, true);

  int x, y;
  for (int cursor = 0; cursor < CURSOR_LAST; cursor++) {
    for (int frame = 0; frame < NUM_CURSOR_FRAMES; frame++) {
      auto curs = static_cast<cursor_type>(cursor);
      auto s = get_cursor_sprite(tileset, curs, &x, &y, frame);
      m_cursors[cursor] <<  QCursor(s->pm, x, y);
    }
  }

  connect(Application::instance(), &Application::dirtyAll,
          this, &MapView::dirtyAll);
  connect(Application::instance(), &Application::dirtyRect,
          this, &MapView::dirtyRect);
  connect(Application::instance(), &Application::flushDirty,
          this, &MapView::flushDirty);
  connect(Application::instance(), &Application::updateCursor,
          this, &MapView::updateCursor);
  connect(Application::instance(), &Application::createLineAtMousePos,
          this, &MapView::createLine);
  connect(Application::instance(), &Application::unitSelectDialog,
          this, &MapView::popupUnitSelector);
  connect(Application::instance(), &Application::flushMapview,
          this, QOverload<>::of(&MapView::repaint));

  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MapView::animate);
  timer->start(200);

  setMouseTracking(true);
  setMinimumWidth(650);
  setMinimumHeight(300);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void MapView::zoomIn() {
  m_zoom *= 1.2;
  if (m_zoom > 10) m_zoom = 10;
}

void MapView::zoomOut() {
  m_zoom *= 0.833;
  if (m_zoom < 0.1) m_zoom = 0.1;
}

void MapView::popupUnitSelector(tile *t) {
  if (m_unitSelector) {
    m_unitSelector->close();
  }
  m_unitSelector = new UnitSelector(t, this);
  connect(m_unitSelector, &UnitSelector::destroyed, this, [=] () {
    m_unitSelector = nullptr;
  });
  m_unitSelector->show();
}

void MapView::createLine() {

  auto p = mapFromGlobal(QCursor::pos());

  if (p.x() >= 0 && p.y() >= 0 && p.x() < mapview.width && p.y() < mapview.width) {
    update_line(p.x(), p.y());
  }
}

void MapView::updateCursor(cursor_type ct) {
  // qCDebug(FC) << "MapView::updateCursor" << ct << CURSOR_DEFAULT;
  if (ct == CURSOR_DEFAULT) {
    setCursor(QCursor(Qt::ArrowCursor));
    m_cursor = -1;
    return;
  }
  m_cursorFrame = 0;
  m_cursor = static_cast<int>(ct);
  setCursor(m_cursors[m_cursor][m_cursorFrame]);
}

void MapView::animate()
{
  if (m_cursor == -1) return;
  m_cursorFrame = (m_cursorFrame + 1) % NUM_CURSOR_FRAMES;
  // qCDebug(FC) << "MapView::animate" << m_cursorFrame;
  setCursor(m_cursors[m_cursor][m_cursorFrame]);
}

void MapView::focusOutEvent(QFocusEvent */*event*/)
{
  updateCursor(CURSOR_DEFAULT);
}

void MapView::leaveEvent(QEvent */*event*/)
{
  updateCursor(CURSOR_DEFAULT);
}

void MapView::resizeEvent(QResizeEvent *event) {

  auto s = event->size();
  map_canvas_resized(s.width(), s.height());

  m_unitInfo->move(s.width() - m_unitInfo->width(), 0);

  m_gameInfo->move(0, 0);
  m_gameInfo->resize(m_gameInfo->sizeHint());
  m_gameInfo->setMaximumWidth(s.width() - m_unitInfo->width());

  m_endTurn->move(s.width() - m_endTurn->width(), s.height() - m_endTurn->height());

  m_minimap->resize(height() * 0.2, height() * 0.2);
  m_minimap->move(s.width() - m_minimap->width() - 5,
                  s.height() - m_endTurn->height() - m_minimap->height() - 10);
}



void MapView::flushDirty() {
  if (m_dirtyCount >= m_maxCountDirties) {
    repaint();
  } else {
    for (int i = 0; i < m_dirtyCount; i++) {
      repaint(m_dirties[i]);
    }
  }
  m_dirtyCount = 0;
}


void MapView::paintEvent(QPaintEvent *event) {
  QPainter p;
  p.begin(this);
  p.drawPixmap(event->rect(), mapview.store->map_pixmap, event->rect());
  p.end();
}


void MapView::dirtyAll() {
  m_dirtyCount = m_maxCountDirties;
}

void MapView::dirtyRect(const QRect &r) {
  if (m_dirtyCount >= m_maxCountDirties) return;
  m_dirties[m_dirtyCount] = r;
  m_dirtyCount++;
}

void MapView::keyPressEvent(QKeyEvent * event)
{

  // TODO: replace with QActions

  if (client_state() != C_S_RUNNING) return;

  bool shift = event->modifiers().testFlag(Qt::ShiftModifier);
  bool none = event->modifiers().testFlag(Qt::NoModifier);
  bool alt = event->modifiers().testFlag(Qt::AltModifier);

  switch (event->key()) {
  case Qt::Key_Up:
  case Qt::Key_8:
    if (shift) {
      key_unit_move(DIR8_NORTH);
    } else if (alt) {
      recenter_button_pressed(width() / 2, 0);
    }
    return;

  case Qt::Key_Left:
  case Qt::Key_4:
    if (shift) {
      key_unit_move(DIR8_WEST);
    } else if (alt)  {
      recenter_button_pressed(0, height() / 2);
    }
    return;

  case Qt::Key_Right:
  case Qt::Key_6:
    if (shift) {
      key_unit_move(DIR8_EAST);
    } else if (alt)  {
      recenter_button_pressed(width(), height() / 2);
    }
    return;

  case Qt::Key_Down:
  case Qt::Key_2:
    if (shift) {
      key_unit_move(DIR8_SOUTH);
    } else if (alt)  {
      recenter_button_pressed(width() / 2, height());
    }
    return;

  case Qt::Key_PageUp:
  case Qt::Key_9:
    if (shift) key_unit_move(DIR8_NORTHEAST);
    return;

  case Qt::Key_PageDown:
  case Qt::Key_3:
    if (shift) key_unit_move(DIR8_SOUTHEAST);
    return;

  case Qt::Key_Home:
  case Qt::Key_7:
    if (shift) key_unit_move(DIR8_NORTHWEST);
    return;

  case Qt::Key_End:
  case Qt::Key_1:
    if (shift) key_unit_move(DIR8_SOUTHWEST);
    return;

  case Qt::Key_5:
  case Qt::Key_Clear:
    if (alt) key_recall_previous_focus_unit();
    return;

  case Qt::Key_Escape:
    if (none) key_cancel_action();
    return;

  default:
    break;
  }
  QWidget::keyPressEvent(event);
}

void MapView::mousePressEvent(QMouseEvent *event)
{
  bool shift = event->modifiers().testFlag(Qt::ShiftModifier);
  bool ctrl = event->modifiers().testFlag(Qt::ControlModifier);
  bool alt = event->modifiers().testFlag(Qt::AltModifier);
  bool none = event->modifiers().testFlag(Qt::NoModifier);
  QPoint pos = event->pos();

  if (event->button() == Qt::LeftButton) {
    if (none) {
      handleSelectPress(pos);
      return;
    }
    if (shift) {
      if (ctrl) {
        handleQuickBuy(pos);
        return;
      }
      if (alt) {
        return;
      }
      handleTileInfoPopup(pos);
      return;
    }
    if (ctrl) {
      if (alt) {
        handleAdjustWorkers(pos);
        return;
      }
      handleProductionSelect(pos);
      return;
    }
    if (alt) {
      handleAppendFocus(pos);
      return;
    }
    return;
  }

  if (event->button() == Qt::RightButton) {
    if (none) {
      handleScroll(pos);
      return;
    }
    if (shift) {
      if (ctrl) {
        handlePasteProduction(pos);
        return;
      }
      if (alt) {
        return;
      }
      handleCopyProduction(pos);
      return;
    }
    if (ctrl) {
      if (alt) {
        handleShowLink(pos);
        return;
      }
      return;
    }
    if (alt) {
      handleHideWorkers(pos);
      return;
    }
    return;
  }

  if (event->button() == Qt::MiddleButton) {
    if (none) {
      return;
    }
    if (shift) {
      if (ctrl) {
        return;
      }
      if (alt) {
        return;
      }
      return;
    }
    if (ctrl) {
      if (alt) {
        return;
      }
      handleWakeupCentries(pos);
      return;
    }
    if (alt) {
      return;
    }
    return;
  }
}

void MapView::handleWakeupCentries(const QPoint &p) {
  wakeup_button_pressed(p.x(), p.y());
}

void MapView::handleTileInfoPopup(const QPoint &p) {
  tile* ptile = canvas_pos_to_tile(p.x(), p.y());
  if (client_tile_get_known(ptile) != TILE_UNKNOWN) {
    mapdeco_set_crosshair(ptile, true);
    auto punit = find_visible_unit(ptile);
    if (punit) {
      mapdeco_set_gotoroute(punit);
      if (punit->goto_tile && unit_has_orders(punit)) {
        mapdeco_set_crosshair(punit->goto_tile, true);
      }
    }
    auto info = new TileInfo(ptile, this);
    info->show();
  }
}

void MapView::handleShowLink(const QPoint &p) {
  tile* ptile = canvas_pos_to_tile(p.x(), p.y());
  m_mainWindow->chatLine()->makeLink(ptile);
}

void MapView::handleHideWorkers(const QPoint &p) {
  key_city_overlay(p.x(), p.y());
}

void MapView::handlePasteProduction(const QPoint &p) {
  tile* ptile = canvas_pos_to_tile(p.x(), p.y());
  city* pcity = tile_city(ptile);
  bool mycity = pcity  && pcity->owner == client_player();
  if (mycity) {
    clipboard_paste_production(pcity);
  }
}

void MapView::handleCopyProduction(const QPoint &p) {
  tile* ptile = canvas_pos_to_tile(p.x(), p.y());
  if (ptile) {
    clipboard_copy_production(ptile);
  }
}

void MapView::handleScroll(const QPoint &p) {
  recenter_button_pressed(p.x(), p.y());
}

void MapView::handleAppendFocus(const QPoint &p) {
  action_button_pressed(p.x(), p.y(), SELECT_APPEND);
}

void MapView::handleAdjustWorkers(const QPoint &p) {
  adjust_workers_button_pressed(p.x(), p.y());
}

void MapView::handleProductionSelect(const QPoint &p) {
  tile* ptile = canvas_pos_to_tile(p.x(), p.y());
  city* pcity = tile_city(ptile);
  bool mycity = pcity  && pcity->owner == client_player();
  if (mycity) {
    auto w = new Buildables(pcity, BuildablesFilter::UnitsBuildingsWonders, this);
    connect(w, &Buildables::selected, this, [=] (universal u){
      city_change_production(pcity, &u);
      w->close();
    });
    w->show();
  }
}

void MapView::handleQuickBuy(const QPoint &p) {
  tile* ptile = canvas_pos_to_tile(p.x(), p.y());
  city* pcity = tile_city(ptile);
  bool mycity = pcity  && pcity->owner == client_player();
  if (mycity) {
    auto w = new Buildables(pcity, BuildablesFilter::UnitsBuildingsWonders, this);
    connect(w, &Buildables::selected, this, [=] (universal u){
      city_change_production(pcity, &u);
      city_buy_production(pcity);
      w->close();
    });
    w->show();
  }
}

void MapView::handleSelectPress(const QPoint &p) {
  m_storedAutocenter = gui_options.auto_center_on_unit;
  gui_options.auto_center_on_unit = false;
  action_button_pressed(p.x(), p.y(), SELECT_POPUP);
  gui_options.auto_center_on_unit = m_storedAutocenter;
}


void MapView::mouseReleaseEvent(QMouseEvent *event)
{
  bool none = event->modifiers().testFlag(Qt::NoModifier);
  bool shift = event->modifiers().testFlag(Qt::ShiftModifier);
  QPoint p = event->pos();

  if (event->button() == Qt::LeftButton) {
    if (none) {
      handleSelectRelease(p);
      return;
    }
    if (shift) {
      handleTileInfoPopdown(p);
      return;
    }
    return;
  }
}


void MapView::handleTileInfoPopdown(const QPoint &) {
  mapdeco_clear_crosshairs();
  mapdeco_clear_gotoroutes();
  auto labels = findChildren<TileInfo*>(QString(), Qt::FindDirectChildrenOnly);
  for (auto label: labels) {
    label->close();
    label->deleteLater();
  }
}

void MapView::handleSelectRelease(const QPoint &p) {
  release_goto_button(p.x(), p.y());
}


void MapView::mouseMoveEvent(QMouseEvent *event)
{
  QPoint p = event->pos();
  if (goto_is_active()) {
    update_line(p.x(), p.y());
  }
  control_mouse_cursor(canvas_pos_to_tile(p.x(), p.y()));
}
