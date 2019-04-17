#pragma once

#include <QObject>
#include <QVector>

extern "C" {
#include "canvas_g.h"
#include "pages_g.h"
#include "helpdlg_g.h"
}
#include "tilespec.h"

class QTimer;
class QWidget;
class QFont;
class QIcon;
class QSocketNotifier;

struct text_tag_list;
struct Clause;

namespace KV {

class MainWindow;


class Application: public QObject {

  Q_OBJECT

  friend class StartDialog;
  friend class ChatWindow;
  friend class ChatLineEdit;
  friend class MapView;
  friend class MinimapView;
  friend class MainWindow;
  friend class UnitInfo;
  friend class GameInfo;
  friend class EndTurnRect;
  friend class GovMenu;
  friend class BrowserWidget;
  friend class ReportWidget;
  friend class PlayerWidget;
  friend class PlayerDialog;
  friend class CityModel;
  friend class UnitSelector;
  friend class ScienceDialog;
  friend class UnitReport;
  friend class EconomyReport;
  friend class HelpDialog;
  friend class NationDialog;
  friend class OptionModel;

public:

  ~Application();

  static void Init();

  /**
    * @brief The main loop for the UI. This is called from main(),
    * and when it exits the client will exit.
    */
  static void Main(int argc, char *argv[]);

  static void Exit();

  /**
   * @brief Make a bell noise (beep). This provides low-level sound alerts even
   * if there is no real sound support.
   */
  static void Beep();

  static MainWindow* Mainwin();
  static void VersionMessage(const char* version);
  static void ChatMessage(const char *astring,
                          const text_tag_list* tags,
                          int conn_id);
  static void SetRulesets(int num_rulesets, char **rulesets);
  static void FlushDirty();
  static void DirtyAll();
  static void DirtyRect(const QRect& r);
  static void UpdateCursor(cursor_type ct);


  static QFont Font(enum client_font font);
  static QIcon Icon(const QString& name);

  static QPixmap SaneMargins(const QPixmap& pix, const QSize& frame);

  static void AddServerSource(int sock);
  static void RemoveServerSource();
  static void UpdateUsers(void*);
  static void AddIdleCallback(void (callback)(void *), void *data);
  static void StateChange(client_pages page);
  static client_pages CurrentState();
  static void UpdateUnitInfo(unit_list* punits);
  static void UpdateUnitSelector();
  static void UpdateGameInfo();
  static void UpdateTurnTimeout();
  static void ToggleTurnDone(bool on);
  static void UpdateTurnDone(bool on);
  static void CreateLineAtMousePos();
  static void UnitSelectDialog(tile *ptile);
  static void FlushMapview();
  static QString ApplyTags(const char *s, const text_tag_list *tags);
  static void UpdateMessages();
  static void UpdateReport(const QStringList& report);
  static void PopupPlayers();
  static void UpdatePlayers();
  static void InitMeeting(int counterpart, int initiator);
  static void CancelMeeting(int counterpart, int canceler);
  static void CreateClause(int counterpart, const Clause& clause);
  static void RemoveClause(int counterpart, const Clause& clause);
  static void AcceptTreaty(int counterpart, bool resolution);
  static void CloseAllTreatyDialogs();
  static void PopupCityReport();
  static void UpdateCityReport();
  static void UpdateCity(city*);
  static void RefreshCityDialog(city*, bool popup);
  static void PopdownCityDialog(city*);
  static void PopupScienceReport();
  static void UpdateScienceReport();
  static void UpdateActions();
  static void PopdownNationDialog();
  static void RefreshNationDialog(bool);
  static void PopupUnitReport();
  static void UpdateUnitReport();
  static void PopupEconomyReport();
  static void UpdateEconomyReport();
  static void PopupHelpDialog(const QString& topic, help_page_type section);
  static void PopdownHelpDialog();
  static void UpdateOption(const option* opt);
  static void AddOption(option* opt);
  static void DelOption(const option* opt);




signals:

  void versionMessage(QString);
  void chatMessage(const QString&);
  void rulesetMessage(const QStringList&);
  void playersChanged();
  void completionListChanged(const QStringList&);
  void flushDirty();
  void dirtyAll();
  void dirtyRect(const QRect& r);
  void updateCursor(cursor_type ct);
  void stateChange(client_pages page);
  void updateUnitInfo(void* punits);
  void updateGameInfo();
  void updateTurnTimeout();
  void toggleTurnDone(bool on);
  void updateTurnDone(bool on);
  void createLineAtMousePos();
  void unitSelectDialog(tile *ptile);
  void flushMapview();
  void updateMessages();
  void updateReport(const QStringList& report);
  void popupPlayers();
  void updatePlayers();
  void initMeeting(int counterpart, int initiator);
  void cancelMeeting(int counterpart, int canceler);
  void createClause(int counterpart, const Clause& clause);
  void removeClause(int counterpart, const Clause& clause);
  void acceptTreaty(int counterpart, bool resolution);
  void closeAllTreatyDialogs();
  void popupCityReport();
  void updateCityReport();
  void updateCity(city*);
  void refreshCityDialog(city*, bool popup);
  void popdownCityDialog(city*);
  void updateUnitSelector();
  void popupScienceReport();
  void updateScienceReport();
  void updateActions();
  void popdownNationDialog();
  void refreshNationDialog(bool);
  void popupUnitReport();
  void updateUnitReport();
  void popupEconomyReport();
  void updateEconomyReport();
  void popupHelpDialog(const QString& topic, help_page_type section);
  void popdownHelpDialog();
  void updateOption(const void* opt);
  void addOption(void* opt);
  void delOption(const void* opt);



private:

  static Application* instance();
  Application();
  Application(const Application&);
  Application& operator=(const Application&);

  void addIdleCallback(void (callback)(void *), void *data);
  void addServerSource(int);
  void removeServerSource();
  void updateUsers();

private slots:

  void timerRestart();
  void processTasks();
  void serverInput(int sock);


private:

  class Callback {
  public:
    Callback()
      : callback(nullptr)
      , data(nullptr) {}
    Callback(void (cb)(void *), void *d)
      : callback(cb)
      , data(d) {}

    void (*callback) (void *data);
    void *data;
  };

  MainWindow* m_mainWindow;
  QTimer* m_timer;
  QVector<Callback> m_tasks;
  QSocketNotifier* m_notifier;
  QStringList m_completionList;


};


}
