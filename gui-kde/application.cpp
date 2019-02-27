#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "fc_config.h"
#include "sprite.h"


// client
#include "client_main.h"
#include "tilespec.h"


#pragma GCC diagnostic pop

#include <QApplication>
#include <QCommandLineParser>
#include <QTextBlock>
#include <QTextCursor>
#include <QTimer>
#include "application.h"
#include "logging.h"
#include "themesmanager.h"
#include "mainwindow.h"


Q_LOGGING_CATEGORY(FC, "Freeciv")
const char* const gui_character_encoding = "UTF-8";
const bool gui_use_transliteration = false;
const char *client_string = "gui-kde";


using namespace KV;

void Application::Init() {/*noop*/}

void Application::Main(int argc, char *argv[]) {

  QApplication app(argc, argv);
  app.setOrganizationName("Kvanttiapina");
  app.setApplicationName("freeciv-kde");

  QCommandLineParser parser;
  parser.setApplicationDescription("Freeciv KDE Client");
  parser.addHelpOption();
  parser.addVersionOption();

  parser.process(app);

  qRegisterMetaType<QTextCursor>("QTextCursor");
  qRegisterMetaType<QTextBlock>("QTextBlock");

  qSetMessagePattern("[%{category} "
                     "%{if-debug}D%{endif}"
                     "%{if-info}I%{endif}"
                     "%{if-warning}W%{endif}"
                     "%{if-critical}C%{endif}"
                     "%{if-fatal}F%{endif}]"
                     "[%{file}:%{line}] - %{message}");
  QLoggingCategory::setFilterRules(QStringLiteral("Freeciv.debug=true"));


  // set application icon
  tileset_init(tileset);
  tileset_load_tiles(tileset);
  app.setWindowIcon(QIcon(get_icon_sprite(tileset, ICON_FREECIV)->pm));

  // set system theme
  ThemesManager::ClearTheme();


  set_client_state(C_S_DISCONNECTED);

  instance()->m_mainWindow->show();

  app.exec();


}

void Application::Exit() {
  // TODO
  // free_mapcanvas_and_overview();
  tileset_free_tiles(tileset);
}

void Application::Beep()
{
  QApplication::beep();
  QApplication::alert(instance()->m_mainWindow->centralWidget());
}

QWidget* Application::View() {
  return instance()->m_mainWindow->centralWidget();
}

void Application::VersionMessage(const char *version) {
  instance()->versionMessage(QString(version));
}

void Application::ChatMessage(const char *astring,
                              const struct text_tag_list *tags, int)
{
  // instance()->chatMessage(QString(astring), tags);
}

QFont Application::Font(enum client_font /*font*/) {
  qCDebug(FC) << "TODO: Font";
  return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

void Application::SetRulesets(int num_rulesets, char **rulesets) {
  QStringList rules;
  for (int i = 0; i < num_rulesets; i++) {
    rules << rulesets[i];
  }
  instance()->rulesetMessage(rules);
}

void Application::AddServerSource(int sock) {
  instance()->addServerSource(sock);
}

void Application::RemoveServerSource() {
  instance()->removeServerSource();
}

void Application::UpdateUsers(void *) {
  instance()->updateUsers();
}

void Application::AddIdleCallback(void callback(void *), void *data) {
  qCDebug(FC) << "AddIdleCallback";
  instance()->addIdleCallback(callback, data);
}

void Application::StateChange(client_pages page) {
  qCDebug(FC) << "TODO: StateChange";
}

Application* Application::instance() {
    static Application* app = new Application();
    return app;
}

void Application::addIdleCallback(void callback(void *), void *data) {
  Callback cb(callback, data);
  m_tasks << cb;
  QTimer::singleShot(0, this, &Application::processTasks);
}

void Application::processTasks() {
  while (!m_tasks.isEmpty()) {
    auto cb = m_tasks.takeFirst();
    cb.callback(cb.data);
  }
}

Application::Application()
  : m_mainWindow(new MainWindow)
  , m_timer(new QTimer)
{
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout, this, &Application::timerRestart);
  m_timer->start();
}

void Application::timerRestart() {
  m_timer->start(real_timer_callback() * 1000);
}

Application::~Application() {
  delete m_mainWindow;
  m_timer->stop();
  delete m_timer;
}
