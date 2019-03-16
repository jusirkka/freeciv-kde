#pragma once

#include <QDialog>

#include "diptreaty.h"

namespace Ui {
class TreatyDialog;
}

struct player;
class QListWidgetItem;

namespace KV {

class TreatyDialog : public QDialog
{
  Q_OBJECT

public:
  explicit TreatyDialog(player* away, QWidget *parent = nullptr);
  ~TreatyDialog();

private:

  void on_homeClauseButton_clicked();
  void on_awayClauseButton_clicked();
  void on_acceptButton_clicked();
  void on_cancelButton_clicked();
  void on_homeGoldLine_textChanged();
  void on_awayGoldLine_textChanged();

  void removeClause(QListWidgetItem*);
  void popupClauses(player* giver, player* taker);
  void allAdvances(player* giver, player* taker);
  void updateTreaty();

private:

  Ui::TreatyDialog *m_ui;
  player* m_away;
  bool m_awayAccepts = false;
  QVector<Clause> m_clauses;
};

}

