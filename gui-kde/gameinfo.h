#ifndef GAMEINFO_H
#define GAMEINFO_H

#include <QWidget>

class QLabel;

namespace KV {

class GameInfo: public QWidget
{
  Q_OBJECT

public:
  explicit GameInfo(QWidget* parent = nullptr);

private slots:

  void updateInfo();
  void updateTurnTime();
  void blink();
  void updateResearch();

private:

  void mousePressEvent(QMouseEvent *event) override;

private:

  QLabel *m_turn;
  QLabel *m_gold;
  QLabel *m_turnTime;
  QLabel *m_research;
  QTimer *m_researchTimer;
  bool m_blinkState;
};
}
#endif // GAMEINFO_H
