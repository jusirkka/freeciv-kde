#ifndef ENDTURNRECT_H
#define ENDTURNRECT_H

#include <QWidget>
#include "fc_types.h"

class QPushButton;

namespace KV {

class SpriteWidget;

class EndTurnRect : public QWidget
{
  Q_OBJECT

public:
  explicit EndTurnRect(QWidget *parent = nullptr);

private slots:

  void updateArea();

  void changeTaxRateWheel(int delta, int id);
  void changeTaxRateClick(Qt::MouseButton button, int id);

  void setTurnButtonEnabled(bool enable);
  void setTurnButtonHighlight(bool highlight);

private:

  void changeTaxRate(output_type_id type, int delta);

  private:

  QPushButton *m_done;
  SpriteWidget *m_governmentIndicator;
  SpriteWidget *m_nuclearIndicator;
  SpriteWidget *m_pollutionIndicator;
  SpriteWidget *m_researchIndicator;
  SpriteWidget *m_taxIndicators[10];
  int m_fontSize;

};

}


#endif // ENDTURNRECT_H
