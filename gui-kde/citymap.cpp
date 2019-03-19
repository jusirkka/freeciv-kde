#include "citymap.h"
#include "canvas.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>

#include "cma_core.h"
#include "citydlg_common.h"
#include "cma_fec.h"
#include "client_main.h"

using namespace KV;

CityMap::CityMap(QWidget *parent)
  : QWidget(parent)
{}

CityMap::~CityMap()
{}

void CityMap::paintEvent(QPaintEvent */*event*/) {
  if (m_city == nullptr) return;

  QPainter painter;

  painter.begin(this);
  painter.drawPixmap(0, 0, m_rect.x(), m_rect.y(), m_pixmap);

  if (cma_is_city_under_agent(m_city, NULL)) {
    painter.fillRect(0, 0, m_pixmap.width(), m_pixmap.height(),
                     QBrush(QColor(60, 60 , 60 , 110)));
    painter.setPen(QColor(255, 255, 255));
    /* TRANS: %1 is custom string choosen by player. */
    auto gov = QString(_("Governor %1"))
          .arg(cmafec_get_short_descr_of_city(m_city));
    painter.drawText(5, m_pixmap.height() - 10, gov);
  }

  painter.end();
}

void CityMap::changeCity(city *c)
{
  m_city = c;

  QPoint rect_c(get_citydlg_canvas_width(), get_citydlg_canvas_height());
  auto view = canvas_create(rect_c.x(), rect_c.y());
  view->map_pixmap.fill(Qt::black);
  city_dialog_redraw_map(m_city, view);

  auto r = sqrt(city_map_radius_sq_get(m_city));
  auto f = (r + 1) / sqrt(rs_max_city_radius_sq());

  m_rect = rect_c * f;
  m_rect.setX(qMin(m_rect.x(), rect_c.x()));
  m_rect.setY(qMin(m_rect.y(), rect_c.y()));

  m_trans = (rect_c - m_rect) * 0.5;

  auto miniview = canvas_create(m_rect.x(), m_rect.y());
  miniview->map_pixmap.fill(Qt::black);
  canvas_copy(miniview, view, m_trans.x(), m_trans.y(),
              0, 0, m_rect.x(), m_rect.y());

  m_pixmap = miniview->map_pixmap;

  canvas_free(miniview);
  canvas_free(view);
}

QSize CityMap::sizeHint() const {
  return m_pixmap.size();
}

QSize CityMap::minimumSizeHint() const {
  return m_pixmap.size();
}

void CityMap::mousePressEvent(QMouseEvent *event)
{

  if (!can_client_issue_orders()) return;
  if (event->button() != Qt::LeftButton) return;

  auto p = event->pos() + m_trans;

  int x, y;
  if (canvas_to_city_pos(&x, &y, city_map_radius_sq_get(m_city), p.x(), p.y())) {
    city_toggle_worker(m_city, x, y);
  }
}

