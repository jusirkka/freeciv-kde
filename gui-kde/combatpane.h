#pragma once

#include <QWidget>
#include <QLabel>
#include "ioutputpane.h"

class QTableWidget;

struct tile;
struct city;

namespace KV {

class Hostile;

city* nearestCity(const tile *t);

class CombatPane : public IOutputPane
{
  Q_OBJECT

public:

  CombatPane();

  QWidget *outputWidget(QWidget *parent) override;
  QVector<QWidget*> toolBarWidgets() const override;
  QString displayName() const override;

  int priorityInStatusBar() const override;

  void clearContents() override;
  void refreshContents() override;
  void configureOutput() override;
  void visibilityChanged(bool visible) override;

  bool canConfigure() const override;
  bool canRefresh() const override;

private slots:

  void addCombat(const Hostile& att, const Hostile& def);

private:

  QTableWidget *m_combats;
  int m_maxLines;

};

}

