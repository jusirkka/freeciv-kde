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

  void removeClause(const Clause& clause);
  void createClause(const Clause& clause);
  void awayResolution(bool accept);

  int away() const;

private slots:

  void on_homeClauseButton_clicked();
  void on_awayClauseButton_clicked();
  void on_acceptButton_clicked();
  void on_cancelButton_clicked();
  void on_homeGoldLine_returnPressed();
  void on_awayGoldLine_returnPressed();

  void removeItem(QListWidgetItem*);
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

