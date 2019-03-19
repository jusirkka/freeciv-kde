#pragma once

#include <QFrame>
#include <QLabel>

struct city;

namespace KV {

class CityInfoWidget : public QFrame
{
  Q_OBJECT
public:
  CityInfoWidget(QWidget* parent = nullptr);
public slots:

  void changeCity(city* c);

private:

  QLabel* m_foodLabel;
  QLabel* m_goldLabel;
  QLabel* m_granaryLabel;
  QLabel* m_corruptionLabel;
  QLabel* m_pollutionLabel;
  QLabel* m_statusLabel;

  QString m_template_2;
  QString m_template_3;

  city* m_city;
};

}

