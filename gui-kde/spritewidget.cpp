#include "spritewidget.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>

#include "sprite.h"

using namespace KV;

SpriteWidget::SpriteWidget(QWidget *parent)
  : QPushButton(parent)
  , m_id(-1)
{
  setFixedSize(m_pix.size());
}

void SpriteWidget::setSprite(const sprite *s)
{
  if (s) {
    m_pix = s->pm;
  }

  // We don't want to be resized.
  setFixedSize(m_pix.size());
  repaint();
}

/****************************************************************************
  Sets an user-defined identifier. It will be passed as argument to signals.
****************************************************************************/
void SpriteWidget::setId(int id)
{
  m_id = id;
}

/****************************************************************************
  Emits button_clicked().
****************************************************************************/
void SpriteWidget::mouseReleaseEvent(QMouseEvent *ev)
{
  QPushButton::mouseReleaseEvent(ev);
  emit buttonClicked(ev->button(), m_id);
  ev->accept();
}

void SpriteWidget::paintEvent(QPaintEvent */*ev*/) {
  if (!m_pix.isNull()) {
    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(0, 0, m_pix);
    painter.end();
  }
}

void SpriteWidget::wheelEvent(QWheelEvent *ev)
{
  int pixels = ev->pixelDelta().y();
  int degrees = ev->angleDelta().y() / 8;

  if (pixels != 0) {
    emit wheelRolled(pixels, m_id);
    ev->accept();
  } else if (degrees != 0) {
    emit wheelRolled(degrees / 15, m_id);
    ev->accept();
  } else {
    ev->ignore();
  }
}
