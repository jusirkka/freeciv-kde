#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QVector>

extern "C" {
#include "canvas_g.h"
#include "pages_g.h"
}
class QTimer;
class QWidget;
class QFont;
class QIcon;
class QSocketNotifier;

struct text_tag_list;

namespace KV {

class MainWindow;


class Application: public QObject {

  Q_OBJECT

  friend class StartDialog;
  friend class ChatWindow;
  friend class ChatLineEdit;

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

  static void VersionMessage(const char* version);
  static void ChatMessage(const char *astring,
                          const text_tag_list* tags,
                          int conn_id);
  static void SetRulesets(int num_rulesets, char **rulesets);

  static QFont Font(enum client_font font);
  static QIcon Icon(const QString& name);

  static void AddServerSource(int sock);
  static void RemoveServerSource();
  static void UpdateUsers(void*);
  static void AddIdleCallback(void (callback)(void *), void *data);
  static void StateChange(client_pages page);
  static client_pages CurrentState();

signals:

  void versionMessage(QString);
  void chatMessage(const QString&);
  void rulesetMessage(const QStringList&);
  void playersChanged();
  void completionListChanged(const QStringList&);

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

#endif // APPLICATION_H
