#pragma once

#include <QTabWidget>
#include <QMap>

struct Clause;

namespace KV {

class PlayerDialog;

class PlayerWidget: public QTabWidget
{

public:

  PlayerWidget(QWidget* parent = nullptr);

private:

  void initMeeting(int counterpart);
  void cancelMeeting(int counterpart);
  void createClause(int counterpart, const Clause& clause);
  void removeClause(int counterpart, const Clause& clause);
  void acceptTreaty(int counterpart, bool other_accepted);
  void closeAllTreatyDialogs();

private:

  PlayerDialog* m_players;
  QMap<int, int> m_meetings;

};

}

