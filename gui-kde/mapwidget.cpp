#include "mapwidget.h"
#include <QTimer>


using namespace KV;

MapWidget::MapWidget(QWidget *parent)
  : QWidget(parent)
  , m_cursor(-1)
  , m_cursorFrame(0)
{
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &MapWidget::animate);
  timer->start(200);
  setMouseTracking(true);
}


/**************************************************************************
  Updates cursor
**************************************************************************/
void MapWidget::updateCursor(cursor_type ct) {
  if (ct == CURSOR_DEFAULT) {
    setCursor(Qt::ArrowCursor);
    m_cursor = -1;
    return;
  }
  m_cursorFrame = 0;
  int i = static_cast<int>(ct);
  m_cursor = i;
  setCursor(*(gui()->fc_cursors[i][0]));
}

/**************************************************************************
  Timer for cursor
**************************************************************************/
void map_view::timer_event()
{
  if (gui()->infotab->underMouse()
      || gui()->minimapview_wdg->underMouse()
      || gui()->game_info_label->underMouse()
      || gui()->unitinfo_wdg->underMouse()) {
    update_cursor(CURSOR_DEFAULT);
    return;
  }
  if (cursor == -1) {
    return;
  }
  cursor_frame++;
  if (cursor_frame == NUM_CURSOR_FRAMES) {
    cursor_frame = 0;
  }
  setCursor(*(gui()->fc_cursors[cursor][cursor_frame]));
}

/**************************************************************************
  Focus lost event
**************************************************************************/
void map_view::focusOutEvent(QFocusEvent *event)
{
  update_cursor(CURSOR_DEFAULT);
}

/**************************************************************************
  Leave event
**************************************************************************/
void map_view::leaveEvent(QEvent *event)
{
  update_cursor(CURSOR_DEFAULT);
}

/**************************************************************************
  slot inherited from QPixamp
**************************************************************************/
void map_view::paintEvent(QPaintEvent *event)
{
  QPainter painter;

  painter.begin(this);
  paint(&painter, event);
  painter.end();
}

/**************************************************************************
  Redraws given rectangle on map
**************************************************************************/
void map_view::paint(QPainter *painter, QPaintEvent *event)
{
  painter->drawPixmap(event->rect(), mapview.store->map_pixmap,
                      event->rect());
}

void map_view::resizeEvent(QResizeEvent *event)
{
  QSize size;
  QSize delta;

  size = event->size();
  if (C_S_RUNNING <= client_state()) {
    map_canvas_resized(size.width(), size.height());
    gui()->infotab->resize(((size.width()
                             - gui()->end_turn_rect->sizeHint().width())
                             * gui()->qt_settings.infotab_width) / 100,
                             (size.height()
                             * gui()->qt_settings.infotab_height) / 100);
    gui()->infotab->move(0 , size.height() - gui()->infotab->height());
    gui()->unitinfo_wdg->move(width() - gui()->unitinfo_wdg->width(), 0);
    gui()->end_turn_rect->end_turn_update();
    delta = size - gui()->end_turn_rect->size();
    gui()->game_info_label->move(0, 0);
    gui()->game_info_label->resize(gui()->game_info_label->sizeHint());
    gui()->game_info_label->setMaximumWidth(size.width()
                                            - gui()->unitinfo_wdg->width());
    gui()->minimapview_wdg->move(size.width() -
                                 gui()->minimapview_wdg->width(),
                                 delta.height() -
                                 gui()->minimapview_wdg->height() - 10);
    gui()->x_vote->move(width() / 2 - gui()->x_vote->width() / 2, 0);
  }
}

