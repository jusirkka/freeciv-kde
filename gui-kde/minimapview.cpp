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
  , m_w_ratio(0)
  , m_h_ratio(0)
  , m_background(QColor(0, 0, 0))
  , m_scale(1)
  , m_thread(new MinimapThread(this))
  , m_bufferSize(2)
  , m_bufferIndex(0)
  , m_buffer(m_bufferSize)
  , m_filled(0)
  , m_empty(m_bufferSize)
{
  setCursor(Qt::CrossCursor);
  m_thread->start();
}

void MinimapView::paintEvent(QPaintEvent */*event*/)
{

  m_filled.acquire();

  QPainter painter;
  painter.begin(this);


  int x = gui_options.overview.map_x0 * m_w_ratio;
  int y = gui_options.overview.map_y0 * m_h_ratio;
  QPixmap pix = m_buffer[m_bufferIndex];
  int ix = pix.width() - x;
  int iy = pix.height() - y;

  // if (m_scale > 1) {
  // painter.drawPixmap(0, 0, pix, 0, 0, pix.width(), pix.height());
  // } else {
    painter.drawPixmap(ix, iy, pix, 0, 0, x, y);
    painter.drawPixmap(ix, 0, pix, 0, y, x, iy);
    painter.drawPixmap(0, iy, pix, x, 0, ix, y);
    painter.drawPixmap(0, 0, pix, x, y, ix, iy);
  // }
  painter.setPen(QColor(palette().color(QPalette::Highlight)));
  painter.drawRect(0, 0, width() - 1, height() - 1);
  drawViewport(&painter);

  painter.end();

  // qCDebug(FC) << "consumed slot" << m_bufferIndex;
  m_bufferIndex = (m_bufferIndex + 1) % m_bufferSize;
  m_empty.release();
}

/**********************************************************************//**
  Sets scaling factor for minimap
**************************************************************************/
void MinimapView::setScale(qreal factor)
{
  m_scale *= factor;
  if (m_scale < 0.1) {
    m_scale = 0.1;
  };
  update();
}

qreal MinimapView::scale() const {
  return m_scale;
}

void MinimapView::moveEvent(QMoveEvent* event)
{
  m_position = event->pos();
}

void MinimapView::showEvent(QShowEvent* event)
{
  move(m_position);
  event->setAccepted(true);
}




static QPoint to_overview(const QPointF& p) {
  const qreal W = tileset_tile_width(tileset);
  const qreal H = tileset_tile_height(tileset);

  QPointF t;
  if (tileset_is_isometric(tileset)) {
    t.setX(p.x() / W + p.y() / H);
    t.setY(p.y() / H - p.x() / W);
  } else {
    t.setX(p.x() / W);
    t.setY(p.y() / H);
  }

  QPointF n;
  if (MAP_IS_ISOMETRIC) {
    n.setY(t.x() + t.y() - wld.map.xsize);
    n.setX(2 * t.x() - n.y());
  } else {
    n = t;
  }

  n -= QPointF(gui_options.overview.map_x0, gui_options.overview.map_y0);
  n *= OVERVIEW_TILE_SIZE;

  QPoint r(floor(n.x()), floor(n.y()));
  if (current_topo_has_flag(TF_WRAPX)) {
    r.setX(FC_WRAP(r.x(), NATURAL_WIDTH * OVERVIEW_TILE_SIZE));
  } else if (MAP_IS_ISOMETRIC) {
    r.setX(r.x() - OVERVIEW_TILE_SIZE);
  }

  if (current_topo_has_flag(TF_WRAPY)) {
    r.setY(FC_WRAP(r.y(), NATURAL_HEIGHT * OVERVIEW_TILE_SIZE));
  }

  return r;
}

static QPoint scalePoint(const QPoint& p, qreal scale)  {

  QPointF p_map(mapview.gui_x0, mapview.gui_y0);
  QPointF r_map(mapview.width, mapview.height);

  QPointF a = to_overview(p_map + 0.5 * r_map);
  QPointF r_over(gui_options.overview.width, gui_options.overview.height);

  QPointF r = scale * (p - a) + 0.5 * r_over;

  return QPoint(r.x(), r.y());
}

static QPoint unscalePoint(const QPoint& r, qreal scale)
{
  QPointF p_map(mapview.gui_x0, mapview.gui_y0);
  QPointF r_map(mapview.width, mapview.height);

  QPointF a = to_overview(p_map + 0.5 * r_map);
  QPointF r_over(gui_options.overview.width, gui_options.overview.height);

  QPointF p = (r - 0.5 * r_over) / scale + a;

  return QPoint(p.x(), p.y());
}

static void viewportCenterTo(QPoint r) {
  r.setX(qMax(r.x(), 1));
  r.setY(qMax(r.y(), 1));
  r.setX(qMin(r.x(), gui_options.overview.width - 1));
  r.setY(qMin(r.y(), gui_options.overview.height - 1));
  int x;
  int y;
  overview_to_map_pos(&x, &y, r.x(), r.y());
  center_tile_mapcanvas(map_pos_to_tile(&(wld.map), x, y));
}

static void pixmap_copy(const QPixmap& src, QPixmap& target, const QRect& from, const QPoint& to)
{
  if (from.width() == 0 || from.height() == 0) return;

  QRect r_target = from;
  r_target.setX(to.x());
  r_target.setY(to.y());

  QPainter p;
  p.begin(&target);
  p.drawPixmap(r_target, src, from);
  p.end();
}

void MinimapView::drawViewport(QPainter *painter)
{
  if (!gui_options.overview.map) return;

  QPoint p[4];
  p[0] = to_overview(QPointF(mapview.gui_x0, mapview.gui_y0));
  p[1] = to_overview(QPointF(mapview.gui_x0 + mapview.width, mapview.gui_y0));
  p[2] = to_overview(QPointF(mapview.gui_x0 + mapview.width, mapview.gui_y0 + mapview.height));
  p[3] = to_overview(QPointF(mapview.gui_x0, mapview.gui_y0 + mapview.height));

  painter->setPen(QColor(Qt::white));

  if (m_scale > 1) {
    for (int i = 0; i < 4; i++) {
      p[i] = scalePoint(p[i], m_scale);
    }
  }

  for (int i = 0; i < 4; i++) {
    int src_x = p[i].x() * m_w_ratio;
    int src_y = p[i].y() * m_h_ratio;
    int dst_x = p[(i + 1) % 4].x() * m_w_ratio;
    int dst_y = p[(i + 1) % 4].y() * m_h_ratio;
    painter->drawLine(src_x, src_y, dst_x, dst_y);
  }
}


/**********************************************************************//**
  Sets minimap scale to default
**************************************************************************/
void MinimapView::reset()
{
  m_scale = 1;
}


/**********************************************************************//**
  Called when minimap has been resized
**************************************************************************/
void MinimapView::resizeEvent(QResizeEvent* event)
{
  auto s = event->size();

  if (client_state() >= C_S_RUNNING) {
    m_w_ratio = static_cast<float>(s.width()) / gui_options.overview.width;
    m_h_ratio = static_cast<float>(s.height()) / gui_options.overview.height;
  }
  repaint();
}

/**********************************************************************//**
  Wheel event for minimap - zooms it in or out
**************************************************************************/
void MinimapView::wheelEvent(QWheelEvent * event)
{
  if (event->delta() > 0) {
    zoomIn();
  } else {
    zoomOut();
  }
  event->accept();
}

/**********************************************************************//**
  Sets scale factor to scale minimap 20% up
**************************************************************************/
void MinimapView::zoomIn()
{
  if (m_scale < gui_options.overview.width / 8) {
    setScale(1.2);
  }
}

/**********************************************************************//**
  Sets scale factor to scale minimap 20% down
**************************************************************************/
void MinimapView::zoomOut()
{
  setScale(0.833);
}

/**********************************************************************//**
  Mouse Handler for MinimapView
  Left button - moves minimap
  Right button - recenters on some point
  For wheel look mouseWheelEvent
**************************************************************************/
void MinimapView::mousePressEvent(QMouseEvent * event)
{
  m_cursor1 = event->pos();
  m_cursor2 = event->globalPos();
  m_topLeft = geometry().topLeft();
  m_size = size();
  m_upper = m_cursor1.y() < m_size.height() / 2;
  m_left = m_cursor1.x() < m_size.width() / 2;

  if (event->button() == Qt::RightButton) {
    QPointF f = event->pos();
    f.setX(f.x() / m_w_ratio);
    f.setY(f.y() / m_h_ratio);
    QPoint r(f.x(), f.y());
    if (m_scale != 1) {
      r = unscalePoint(r, m_scale);
    }
    viewportCenterTo(r);
    update();
  }
  event->setAccepted(true);
}

/**********************************************************************//**
  Called when mouse button was pressed. Used to moving minimap.
**************************************************************************/
void MinimapView::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons() & Qt::LeftButton) {
    if(QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier)) {
      setCursor(Qt::SizeAllCursor);
      QPoint d = event->globalPos() - m_cursor2;
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
      move(m_topLeft + event->globalPos() - m_cursor2);
    }
  }
}

/**********************************************************************//**
  Called when mouse button unpressed. Restores cursor.
**************************************************************************/
void MinimapView::mouseReleaseEvent(QMouseEvent* /*event*/)
{
  setCursor(Qt::CrossCursor);
  update();
}

void MinimapView::acquireEmptyFrame() {
  m_empty.acquire();
}

void MinimapView::releaseFilledFrame() {
  m_filled.release();
}

MinimapView::FrameVector& MinimapView::buffer() {
  return m_buffer;
}

int MinimapView::bufferSize() const {
  return m_bufferSize;
}

MinimapThread::MinimapThread(MinimapView *parent)
  : QThread(parent)
  , m_parent(parent)
{}

MinimapThread::~MinimapThread() {
  wait();
}

void MinimapThread::run()
{
  int slot = 0;
  while (gui_options.overview.map != nullptr) {

    m_parent->acquireEmptyFrame();

    QPixmap pix;

    if (m_parent->scale() > 1) {
      // move minimap now, scale later and draw without looking for origin
      const QPixmap& src = gui_options.overview.map->map_pixmap;
      QPixmap& dst = gui_options.overview.window->map_pixmap;
      QPair<QPoint, QRect> data[4];
      auto p = QPoint(gui_options.overview.map_x0, gui_options.overview.map_y0);
      auto r_over = QPoint(gui_options.overview.width, gui_options.overview.height);
      auto m = p - r_over;
      data[0].first = m;
      data[1].first = QPoint(m.x(), 0);
      data[2].first = QPoint(0, m.y());
      data[3].first = QPoint(0, 0);
      data[0].second = QRect(QPoint(0, 0), QSize(p.x(), p.y()));
      data[1].second = QRect(QPoint(0, p.y()), QSize(p.x(), m.y()));
      data[2].second = QRect(QPoint(p.x(), 0), QSize(m.x(), p.y()));
      data[3].second = QRect(p, QSize(m.x(), m.y()));
      for (int i = 0; i < 4; i++) {
        pixmap_copy(src, dst, data[i].second, data[i].first);
      }
      QPoint r_scaled = r_over / m_parent->scale();
      QPoint c = unscalePoint(QPoint(0, 0), m_parent->scale());
      QPixmap big(2 * QSize(r_over.x(), r_over.y()));
      big.fill(Qt::black);
      pixmap_copy(dst, big, QRect(0.5 * r_over, QSize(r_over.x(), r_over.y())), QPoint(0, 0));
      pix = big
          .copy(QRect(c + 0.5 * r_over, c + 0.5 * r_over  + r_scaled))
          .scaled(m_parent->width(), m_parent->height(),
                  Qt::IgnoreAspectRatio, Qt::FastTransformation);
    } else {
      pix = gui_options.overview.map->map_pixmap
          .scaled(m_parent->width(), m_parent->height(),
                  Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }

    m_parent->buffer()[slot] = pix;
    // qCDebug(FC) << "produced slot" << slot;
    slot = (slot + 1) % m_parent->bufferSize();
    m_parent->releaseFilledFrame();
  }
}
