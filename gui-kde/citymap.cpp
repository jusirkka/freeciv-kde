#include "citymap.h"
#include "canvas.h"
#include "logging.h"
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <QMenu>

#include "cma_core.h"
#include "citydlg_common.h"
#include "cma_fec.h"
#include "client_main.h"
#include "tilespec.h"

using namespace KV;

CityMap::CityMap(QWidget *parent)
  : QWidget(parent)
{}

CityMap::~CityMap()
{}

void CityMap::paintEvent(QPaintEvent *event) {

  if (m_city == nullptr) return;
  QPainter painter;

  painter.begin(this);

  painter.fillRect(event->rect(), QColor(Qt::black));

  int w =  m_displayPixmap.width();
  int h =  m_displayPixmap.height();
  int x = (event->rect().width() - w) / 2;
  int y = (event->rect().height() - h) / 2;
  painter.drawPixmap(x, y, w, h, m_displayPixmap);

  if (cma_is_city_under_agent(m_city, nullptr)) {
    painter.fillRect(event->rect(), QBrush(QColor(60, 60, 60, 110)));
    painter.setPen(Qt::white);
    /* TRANS: %1 is custom string chosen by the player. */
    auto gov = QString(_("Governor %1")).arg(cmafec_get_short_descr_of_city(m_city));
    painter.drawText(5, event->rect().height() - 10, gov);
  }

  painter.end();
}

void CityMap::changeCity(city *c)
{
  m_city = c;

  auto view = canvas_create(get_citydlg_canvas_width(), get_citydlg_canvas_height());
  view->map_pixmap.fill(Qt::black);
  city_dialog_redraw_map(m_city, view);
  m_cityPixmap = view->map_pixmap;
  canvas_free(view);

  createDisplayPixmap(size());
  repaint();
}

void CityMap::createDisplayPixmap(const QSize& s) {
  m_displayPixmap = m_cityPixmap
      .scaledToHeight(s.height(), Qt::SmoothTransformation);
}

void CityMap::resizeEvent(QResizeEvent *event) {
  createDisplayPixmap(event->size());
  QWidget::resizeEvent(event);
}

QSize CityMap::minimumSizeHint() const {
  return QSize(360, 180);
}

void CityMap::mousePressEvent(QMouseEvent *event)
{

  if (!can_client_issue_orders()) return;
  if (event->button() == Qt::LeftButton) {
    // position of the display map in the widget
    QPoint d(m_displayPixmap.size().width(), m_displayPixmap.size().height());
    QPoint s(size().width(), size().height());
    QPoint q = (s - d) / 2;

    // position of the mouse in the display map
    auto p = event->pos() - q;

    float zoom = static_cast<float>(m_displayPixmap.height()) / m_cityPixmap.height();
    // position of the mouse in the city map
    QPointF z = p / zoom;

    int x, y;
    if (canvas_to_city_pos(&x, &y, city_map_radius_sq_get(m_city), z.x(), z.y())) {
      city_toggle_worker(m_city, x, y);
    }
    return;
  }

  if (event->button() == Qt::RightButton) {
    QMenu menu;
    menu.setTitle("Apply preset governor");
    int num = cmafec_preset_num();
    for (int i = 0; i < num; i++) {
      auto a = new QAction(cmafec_preset_get_descr(i));
      a->setData(i);
      menu.addAction(a);
    }
    if (cma_is_city_under_agent(m_city, nullptr)) {
      menu.addSeparator();
      auto a = new QAction("Disable Governor");
      a->setData(-1);
      menu.addAction(a);
    }

    auto a = menu.exec(QCursor::pos());
    if (a == nullptr) return;

    int sel = a->data().toInt();
    if (sel == -1) {
      cma_release_city(m_city);
    } else {
      emit governorChanged(m_city, sel);
    }
  }
}

