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
struct text_tag_list;

namespace KV {

class MainWindow;


class Application: public QObject {

  Q_OBJECT

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

  static void AddServerSource(int sock);
  static void RemoveServerSource();
  static void UpdateUsers(void*);
  static void AddIdleCallback(void (callback)(void *), void *data);
  static void StateChange(enum client_pages page);

signals:

  void versionMessage(QString);
  void chatMessage(QString, const text_tag_list*);
  void rulesetMessage(QStringList);
  void addServerSource(int);
  void removeServerSource();
  void updateUsers();

private:

  static Application* instance();
  Application();
  Application(const Application&);
  Application& operator=(const Application&);

  void addIdleCallback(void (callback)(void *), void *data);

private slots:

  void timerRestart();
  void processTasks();


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


};


}

#endif // APPLICATION_H
