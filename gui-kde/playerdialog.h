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

  static const int InvalidPlayer = -1;

  void initMeeting(int counterpart, int initiator);
  void cancelMeeting(int counterpart, int canceler);
  void createClause(int counterpart, const Clause& clause);
  void removeClause(int counterpart, const Clause& clause);
  void acceptTreaty(int counterpart, bool other_accepted);
  void closeAllTreatyDialogs();

private:

  struct Meeting {
    int initiator;
    int index;
  };
  using MeetingMap = QMap<int, Meeting>;

  PlayerWidget* m_players;
  MeetingMap m_meetings;
  QTabWidget* m_tabs;

};

}

