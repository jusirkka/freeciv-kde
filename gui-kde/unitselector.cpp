#include "unitselector.h"
#include <QPainter>
#include <QWheelEvent>
#include "canvas.h"
#include "application.h"

#include "tilespec.h"
#include "control.h"
#include "mapview_common.h"
#include "movement.h"


using namespace KV;


UnitSelector::UnitSelector(tile* t, QWidget *parent)
  : QWidget(parent)
  , m_itemSize(0, 0)
  , m_indexHigh(-1)
  , m_unitOffset(0)
  , m_tile(t)
{
  setAttribute(Qt::WA_DeleteOnClose);

  m_font.setItalic(true);
  m_infoFont.setPointSize(12);

  setMouseTracking(true);

  updateUnits();

  QPoint p = mapFromGlobal(QCursor::pos());
  if (p.x() + width() > parentWidget()->width()) {
    p.setX(parentWidget()->width() - width());
  }
  if (p.y() - height() < 0) {
    p.setY(height());
  }
  move(p.x(), p.y() - height());
  setFocus();

  connect(&m_delay, &QTimer::timeout, this, [=] () {
    close();
  });

  connect(Application::instance(), &Application::updateUnitSelector,
          this, &UnitSelector::updateUnits);
}


void UnitSelector::updateUnits() {
  updateUnitList();
  createPixmap();
  setFixedWidth(m_pix.width() + 20);
  setFixedHeight(m_pix.height() + 2 * QFontMetrics(m_infoFont).height() + 12);
}



void UnitSelector::updateUnitList() {

  if (m_tile == nullptr) {
    unit *punit = head_of_units_in_focus();
    if (punit) {
      m_tile = unit_tile(punit);
    }
  }

  m_unitList.clear();

  if (m_tile != nullptr && m_tile->units != nullptr) {
    unit_list_iterate(m_tile->units, punit) {
      m_unitList << punit;
    } unit_list_iterate_end;
  }
}




void UnitSelector::createPixmap()
{

  if (m_unitList.isEmpty()) return;

  m_itemSize.setWidth(tileset_full_tile_width(tileset));
  m_itemSize.setHeight(tileset_tile_height(tileset) * 3 / 2);

  m_pixHigh = QPixmap(m_itemSize);
  m_pixHigh.fill(palette().color(QPalette::HighlightedText));
  int unitCount = m_unitList.count() - m_unitOffset;
  if (unitCount < 5) {
    m_pix = QPixmap(m_itemSize.width() * m_unitList.size(),
                    m_itemSize.height());
  } else if (unitCount < 9) {
    m_pix = QPixmap(4 * m_itemSize.width(), 2 * m_itemSize.height());
  } else {
    m_pix = QPixmap(4 * m_itemSize.width(), 3 * m_itemSize.height());
  }
  m_pix.fill(Qt::transparent);

  QVector<QPixmap> pixes;
  for (int i = m_unitOffset; i < unitCount; i++) {
    unit* punit = m_unitList[i];
    auto pix = canvas_create(tileset_full_tile_width(tileset),
                             tileset_tile_height(tileset) * 3 / 2);
    pix->map_pixmap.fill(Qt::transparent);
    put_unit(punit, pix, 1.0, 0, 0);
    pixes << pix->map_pixmap;
    canvas_free(pix);
  }

  QPainter p;
  p.begin(&m_pix);
  m_font.setPixelSize(qMin(m_itemSize.width() / 4, 12));
  p.setFont(m_font);
  QPen pen;
  pen.setColor(palette().color(QPalette::Text));
  p.setPen(pen);

  for (int i = 0; i < pixes.count(); i++) {

    int x = (i % 4) * m_itemSize.width();
    int y = (i / 4) * m_itemSize.height();

    if (i + m_unitOffset == m_indexHigh) {
      p.drawPixmap(x, y, m_pixHigh);
    }
    p.drawPixmap(x, y, pixes[i]);

    auto punit = m_unitList[i];
    auto points = unit_type_get(punit)->move_rate * (punit->fuel - 1);
    auto str = QString(move_points_text(punit->moves_left, false));
    if (utype_fuel(unit_type_get(punit))) {
      str = QString("%1(%2)")
          .arg(str)
          .arg(move_points_text(points + punit->moves_left, false));
    }
    /* TRANS: MP = Movement points */
    p.drawText(x, y + m_itemSize.height() - 4, QString(_("MP:%1")).arg(str));
  }
  p.end();
}


void UnitSelector::paintEvent(QPaintEvent */*event*/)
{

  QString textHigh;
  if (m_indexHigh >= 0 && m_indexHigh < m_unitList.count()) {
    auto punit = m_unitList[m_indexHigh];
    /* TRANS: HP - hit points */
    textHigh = QString(_("%1 HP:%2/%3"))
        .arg(QString(unit_activity_text(punit)))
        .arg(punit->hp)
        .arg(unit_type_get(punit)->hp);
  }
  QString text = QString(PL_("%1 unit", "%1 units", m_unitList.count()))
      .arg(m_unitList.count());

  for (int i = m_infoFont.pointSize(); i > 4; i--) {
    QFontMetrics fm(m_infoFont);
    if (10 + fm.width(textHigh) < width() && 10 + fm.width(text) < width()) {
      break;
    }
    m_infoFont.setPointSize(i);
  }

  QFontMetrics fm(m_infoFont);
  int h = fm.height();

  QPainter painter;

  painter.begin(this);

  painter.setBrush(palette().color(QPalette::Background));
  painter.setPen(palette().color(QPalette::Background));
  painter.drawRect(0, 0, width(), height());

  painter.drawPixmap(10, h + 3, m_pix);

  QPen pen;
  pen.setColor(palette().color(QPalette::Text));
  painter.setPen(pen);
  painter.setFont(m_infoFont);
  painter.drawText(10, h, text);
  if (!textHigh.isEmpty()) {
    painter.drawText(10, height() - 5, textHigh);
  }
  // draw scroll
  if (m_unitList.size() > 12) {
    painter.setBrush(palette().color(QPalette::HighlightedText).darker());
    painter.setPen(palette().color(QPalette::HighlightedText).darker());
    painter.drawRect(m_pix.width() + 10,
                     h,
                     8,
                     h + m_pix.height());
    pen.setColor(palette().color(QPalette::HighlightedText));
    painter.setPen(pen);
    int maxLine = (m_unitList.size() + 3) / 4 - 3;
    int colHeight = m_pix.height() * 3 / (3 + maxLine);
    int colStart = (m_unitOffset / 4) * m_pix.height() / (3 + maxLine);
    painter.drawRoundedRect(m_pix.width() + 10,
                            h + colStart,
                            8,
                            h + colHeight, 2, 2);
  }
  painter.end();
}


void UnitSelector::mouseMoveEvent(QMouseEvent *event)
{
  QFontMetrics fm(m_infoFont);

  if (event->x() > width() - 11 || event->y() > height() - fm.height() - 5
      || event->y() < fm.height() + 3 || event->x() < 11) {
    return;
  }

  int col = (event->x() - 10) / m_itemSize.width();
  int row = (event->y() - fm.height() - 3) / m_itemSize.height();
  int idx = row * 4 + col + m_unitOffset;

  if (m_indexHigh != idx) {
    m_indexHigh = idx;
    createPixmap();
    update();
  }
}

void UnitSelector::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    if (m_indexHigh < 0 || m_indexHigh >= m_unitList.size()) {
      return;
    }
    auto punit = m_unitList[m_indexHigh];
    unit_focus_set(punit);
    close();
  }
}



void UnitSelector::wheelEvent(QWheelEvent *event)
{
  int maxLine = (m_unitList.size() + 3) / 4 - 3;
  if (maxLine < 1) return;
  if (event->delta() < 0) {
    m_unitOffset += 4;
    m_unitOffset = qMin(m_unitOffset, 4 * maxLine);
  } else {
    m_unitOffset -= 4;
    m_unitOffset = qMax(0, m_unitOffset);
  }
  createPixmap();
  update();
  event->accept();
}

void UnitSelector::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape) {
    close();
  }
  QWidget::keyPressEvent(event);
}

void UnitSelector::closeEvent(QCloseEvent* event)
{
  m_delay.stop();
  parentWidget()->setFocus();
  QWidget::closeEvent(event);
}

void UnitSelector::enterEvent(QEvent */*event*/) {
  m_delay.stop();
}

void UnitSelector::leaveEvent(QEvent */*event*/) {
  m_delay.setSingleShot(true);
  m_delay.start(1000);
}
