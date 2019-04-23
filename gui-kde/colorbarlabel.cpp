#include "colorbarlabel.h"
#include <QPainter>

using namespace KV;

ColorBarLabel::ColorBarLabel(QWidget *parent)
  : QLabel(parent)
{
  m_colors << QColor(Qt::green)
           << QColor(Qt::yellow)
           << QColor(Qt::red)
           << QColor(Qt::gray);
  m_numbers << 1 << 1 << 1 << 1;
  m_len = 4;
  setFixedHeight(fontMetrics().height() / 2);
}


void ColorBarLabel::setNumbers(const NumbersVector &ns) {
  m_numbers = ns;
  m_len = 0;
  for (int n: ns) {
    m_len += n;
  }
  updateColors();
}

void ColorBarLabel::updateColors() {
  QPixmap pix(size());
  QPainter p;
  p.begin(&pix);
  p.fillRect(rect(), Qt::black);
  if (m_len != 0) {
    int x_p = 0;
    for (int i = 0; i < m_numbers.size(); i++) {
      int w = width() * m_numbers[i] / m_len;
      QRect r(x_p, 0, w, height());
      p.fillRect(r, m_colors[i % m_colors.size()]);
      x_p += w;
    }
  }
  p.end();
  setPixmap(pix);
}

void ColorBarLabel::resizeEvent(QResizeEvent */*event*/) {
  updateColors();
}
