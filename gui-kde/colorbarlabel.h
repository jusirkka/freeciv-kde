#pragma once

#include <QLabel>

namespace KV {

class ColorBarLabel: public QLabel
{
  Q_OBJECT
public:
  using NumbersVector = QVector<int>;
  explicit ColorBarLabel(QWidget *parent = nullptr);
  void setNumbers(const NumbersVector& n);

  void resizeEvent(QResizeEvent *event) override;

private:

  void updateColors();

private:

  using ColorVector = QVector<QColor>;

  ColorVector m_colors;
  NumbersVector m_numbers;
  int m_len;
};

}
