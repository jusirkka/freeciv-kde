#include "tileinfo.h"
#include <QPainter>
#include "logging.h"

#include "tilespec.h"
#include "text.h"
#include "mapview_common.h"

using namespace KV;

TileInfo::TileInfo(tile* ptile, QWidget *parent):
  QWidget(parent)
{

  m_infoFont.setPointSize(14);

  QString text = popup_info_text(ptile);
  m_lines = text.split("\n");

  QFontMetrics fm(m_infoFont);
  int w = 0;
  for (auto line: m_lines) {
    w = qMax(w, fm.width(line));
  }

  setFixedHeight(m_lines.count() * fm.height() + 10);
  setFixedWidth(w + 10);

  float x = 0;
  float y = 0;
  if (tile_to_canvas_pos(&x, &y, ptile)) {
    if (y - height() > 0) {
      y -= height();
    } else {
      y  += tileset_tile_height(tileset);
    }
    if (x + width() > parentWidget()->width()) {
      x = parentWidget()->width() - width();
    }
    move(x, y);
  }
  // qCDebug(FC) << "TileInfo" << size() << pos();
}

void TileInfo::paintEvent(QPaintEvent */*event*/)
{
  QPainter painter;

  painter.begin(this);
  painter.setBrush(palette().color(QPalette::Background));
  // painter.setPen(palette().color(QPalette::Background));
  painter.drawRect(0, 0, width(), height());

  QFontMetrics fm(m_infoFont);
  int h = fm.height();
  int pos = h;
  // qCDebug(FC) << "TileInfo: painting" << isVisible();
  painter.setFont(m_infoFont);
  for (auto line: m_lines) {
    painter.drawText(5, pos, line);
    pos += h;
  }
  painter.end();
}
