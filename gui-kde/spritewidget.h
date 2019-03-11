#ifndef SPRITEWIDGET_H
#define SPRITEWIDGET_H

#include <QPushButton>

class QPixmap;
struct sprite;

namespace KV {
class SpriteWidget : public QPushButton
{
  Q_OBJECT
public:

  explicit SpriteWidget(QWidget *parent = nullptr);

  void setSprite(const sprite *s);
  void setId(int id);

signals:

  void buttonClicked(Qt::MouseButton button, int id);
  void wheelRolled(int delta, int id);

protected:

  void mouseReleaseEvent(QMouseEvent *ev) override;
  void paintEvent(QPaintEvent *ev) override;
  void wheelEvent(QWheelEvent *ev) override;

private:

  QPixmap m_pix;
  int m_id;

};

}

#endif // SPRITEWIDGET_H
