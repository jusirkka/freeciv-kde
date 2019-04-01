#pragma once

#include <QDialog>
#include <QMap>

struct Clause;

class QTabWidget;

namespace KV {

class PlayerWidget;

class PlayerDialog: public QDialog
{

public:

  PlayerDialog(QWidget* parent = nullptr);
  ~PlayerDialog();

private:

  void initMeeting(int counterpart);
  void cancelMeeting(int counterpart);
  void createClause(int counterpart, const Clause& clause);
  void removeClause(int counterpart, const Clause& clause);
  void acceptTreaty(int counterpart, bool other_accepted);
  void closeAllTreatyDialogs();

private:

  PlayerWidget* m_players;
  QMap<int, int> m_meetings;
  QTabWidget* m_tabs;

};

}

