#ifndef STARTDIALOG_H
#define STARTDIALOG_H

#include <QDialog>

class QPoint;

namespace Ui {
class StartDialog;
}

struct player;

namespace KV {

class NationDialog;

class StartDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StartDialog(QWidget* parent);
  ~StartDialog();

private slots:

  void popupTreeMenu(const QPoint& p);
  void populateTree();
  void rulesetChange(const QString& rules);
  void setRulesets(const QStringList& sets);
  void maxPlayersChanged(int numPlayers);
  void aiLevelChanged(int idx);
  void pickNation(const player* p);
  void observe();

private:

  void updateButtons();

signals:

  void configLocal(bool);
  void configServer(bool);

private:

  Ui::StartDialog* m_ui;
  NationDialog* m_nationDialog;

};
}

#endif // STARTDIALOG_H
