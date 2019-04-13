#include "minimapview.h"
#include <QPainter>
#include <QMoveEvent>
#include <QWheelEvent>
#include <QShowEvent>
#include <QPixmap>
#include <QApplication>
#include "canvas.h"
#include "logging.h"

#include "options.h"
#include "tilespec.h"
#include "mapview_common.h"
#include "overview_common.h"
#include "client_main.h"

using namespace KV;



MinimapView::MinimapView(QWidget *parent)
  : QWidget(parent)
{
  setCursor(Qt::CrossCursor);
}

void MinimapView::paintEvent(QPaintEvent */*event*/)
{

  if (!isVisible()) return;
  if (gui_options.overview.window == nullptr) return;

  auto pix = gui_options.overview.window->map_pixmap;

  QPainter painter;

  painter.begin(this);
  painter.drawPixmap(0, 0, pix.scaled(size()));
  painter.setPen(QColor(palette().color(QPalette::Highlight)));
  painter.drawRect(1, 1, width() - 2, height() - 2);
  painter.end();
}


void MinimapView::mousePressEvent(QMouseEvent* event)
{
  m_cursor = event->globalPos();
  m_topLeft = geometry().topLeft();
  m_size = size();
  m_upper = event->pos().y() < m_size.height() / 2;
  m_left = event->pos().x() < m_size.width() / 2;

  if (event->button() == Qt::RightButton) {
    viewportCenterTo(event->pos());
  }
  event->accept();
}

void MinimapView::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons() & Qt::LeftButton) {

    if (event->modifiers().testFlag(Qt::ShiftModifier)) {
      setCursor(Qt::SizeAllCursor);
      QPoint d = event->globalPos() - m_cursor;
      QPoint n;
      if (m_upper) {
        if (m_left) {
          n.setX(qMax(50, -d.x() + m_size.width()));
          n.setY(qMax(50, -d.y() + m_size.height()));
          move(m_topLeft.x() + d.x(), m_topLeft.y() + d.y());
        } else {
          n.setX(qMax(50, d.x() + m_size.width()));
          n.setY(qMax(50, -d.y() + m_size.height()));
          move(m_topLeft.x(), m_topLeft.y() + d.y());
        }
      } else {
        if (m_left) {
          n.setX(qMax(50, -d.x() + m_size.width()));
          n.setY(qMax(50, d.y() + m_size.height()));
          move(m_topLeft.x() + d.x(), m_topLeft.y());
        } else {
          n.setX(qMax(50, d.x() + m_size.width()));
          n.setY(qMax(50, d.y() + m_size.height()));
        }
      }
      resize(n.x(), n.y());

    } else {
      setCursor(Qt::OpenHandCursor);
      move(m_topLeft + event->globalPos() - m_cursor);
    }
  }
  event->accept();
}

void MinimapView::mouseReleaseEvent(QMouseEvent* event) {
  setCursor(Qt::CrossCursor);
  repaint();
  event->accept();
}

void MinimapView::viewportCenterTo(const QPoint& pos) {

  float ox = static_cast<float>(pos.x()) / size().width() * gui_options.overview.width;
  float oy = static_cast<float>(pos.y()) / size().height() * gui_options.overview.height;

  int x;
  int y;
  overview_to_map_pos(&x, &y, ox, oy);
  center_tile_mapcanvas(map_pos_to_tile(&(wld.map), x, y));
}


