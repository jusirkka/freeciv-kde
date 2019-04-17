#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "fc_config.h"
#include "shared.h"
#include "featured_text.h"
#include "audio.h"
#include "game.h"
#include "unit.h"

#include "client_main.h"
#include "tilespec.h"
#include "clinet.h"
#include "mapview_common.h"
#include "overview_common.h"
#include "citydlg_common.h"

#include "colors.h"
#include "sprite.h"

#pragma GCC diagnostic pop

#include <QApplication>
#include <QCommandLineParser>
#include <QTextBlock>
#include <QTextCursor>
#include <QTimer>
#include <QSocketNotifier>
#include <QIcon>
#include <KAboutData>
#include <KLocalizedString>
#include <QPainter>


#include "application.h"
#include "logging.h"
#include "themesmanager.h"
#include "mainwindow.h"
#include "themesmanager.h"
#include "state.h"


Q_LOGGING_CATEGORY(FC, "Freeciv")
const char* const gui_character_encoding = "UTF-8";
const bool gui_use_transliteration = false;
const char *client_string = "gui-kde";


using namespace KV;

void Application::Init() {}

void Application::Main(int argc, char **argv) {

  QApplication app(argc, argv);
  app.setOrganizationName("Kvanttiapina");
  app.setApplicationName("freeciv-kde");

  KLocalizedString::setApplicationDomain("freeciv");

  KAboutData aboutData(
        // The program name used internally. (componentName)
        qAppName(),
        // A displayable program name string. (displayName)
        qAppName(),
        // The program version string. (version)
        QStringLiteral("0.0.9"),
        // Short description of what the app does. (shortDescription)
        i18n("KDE Freeciv client"),
        // The license this code is released under
        KAboutLicense::GPL,
        // Copyright Statement (copyrightStatement = QString())
        i18n("Jukak Sirkka (c) 2019"),
        // Optional text shown in the About box.
        // Can contain any information desired. (otherText)
        i18n("Now... Go Give 'em Hell!"),
        // The program homepage string. (homePageAddress = QString())
        QStringLiteral("https://github.com/jusirkka/freeciv-kde"),
        // The bug report email address
        // (bugsEmailAddress = QLatin1String("submit@bugs.kde.org")
        QStringLiteral("https://github.com/jusirkka/freeciv-kde/issues"));

  aboutData.addAuthor(i18n("Jukka Sirkka"),
                      i18n("Codemonkey"),
                      QStringLiteral("jukka.sirkka@iki.fi"),
                      QStringLiteral("https://github.com/jusirkka"));

  KAboutData::setApplicationData(aboutData);

  QCommandLineParser parser;
  aboutData.setupCommandLine(&parser);
  parser.process(app);
  aboutData.processCommandLine(&parser);

  qCDebug(FC) << "Application::Main";

  // set application icon
  tileset_init(get_tileset());
  tileset_load_tiles(get_tileset());
  qApp->setWindowIcon(QIcon(get_icon_sprite(get_tileset(), ICON_FREECIV)->pm));

  // set system theme
  ThemesManager::ClearTheme();


  set_client_state(C_S_DISCONNECTED);
  init_mapcanvas_and_overview();
  calculate_overview_dimensions();

  // mainwindow ctor refers to application instance:
  // cannot be created in the instance ctor, so created here instead
  instance()->m_mainWindow = new MainWindow;
  instance()->m_mainWindow->show();
  app.exec();
  qCDebug(FC) << "Mainwindow closed";
}

void Application::Exit() {
  // TODO
  // free_mapcanvas_and_overview();
  tileset_free_tiles(get_tileset());
}

void Application::Beep()
{
  QApplication::beep();
  QApplication::alert(instance()->m_mainWindow->centralWidget());
}

void Application::VersionMessage(const char *version) {
  qCDebug(FC) << "TODO: Application::VersionMessage: not connected";
  instance()->versionMessage(QString(version));
}


QString Application::ApplyTags(const char *s, const text_tag_list *tags) {
  if (!tags) {
    return QString(s);
  }

  QMultiMap <int, QString> mm;
  QByteArray bytes = s;

  int start;
  int stop;
  text_tag_list_iterate(tags, ptag) {
    if ((text_tag_stop_offset(ptag) == FT_OFFSET_UNSET)) {
      stop = bytes.count();
    } else {
      stop = text_tag_stop_offset(ptag);
    }

    if ((text_tag_start_offset(ptag) == FT_OFFSET_UNSET)) {
      start = 0;
    } else {
      start = text_tag_start_offset(ptag);
    }

    switch (text_tag_type(ptag)) {
    case TTT_BOLD:
      mm.insert(stop, "</b>");
      mm.insert(start, "<b>");
      break;
    case TTT_ITALIC:
      mm.insert(stop, "</i>");
      mm.insert(start, "<i>");
      break;
    case TTT_STRIKE:
      mm.insert(stop, "</s>");
      mm.insert(start, "<s>");
      break;
    case TTT_UNDERLINE:
      mm.insert(stop, "</u>");
      mm.insert(start, "<u>");
      break;
    case TTT_COLOR: {
      auto fg = text_tag_color_foreground(ptag);
      auto bg = text_tag_color_background(ptag);
      if (fg) {
        mm.insert(stop, "</span>");
        mm.insert(start, QString("<span style=color:%1>").arg(fg));
      }
      if (bg) {
        mm.insert(stop, "</span>");
        mm.insert(start, QString("<span style=background-color:%1;>").arg(bg));
      }
      break;
    }
    case TTT_LINK: {
      color* pcolor = nullptr;

      switch (text_tag_link_type(ptag)) {
      case TLT_CITY:
        pcolor = get_color(get_tileset(), COLOR_MAPVIEW_CITY_LINK);
        break;
      case TLT_TILE:
        pcolor = get_color(get_tileset(), COLOR_MAPVIEW_TILE_LINK);
        break;
      case TLT_UNIT:
        pcolor = get_color(get_tileset(), COLOR_MAPVIEW_UNIT_LINK);
        break;
      }

      if (!pcolor) {
        break; /* Not a valid link type case. */
      }
      mm.insert(stop, "</a></font>");
      mm.insert(start, QString("<font color=\"%1\"><a href=%2,%3>")
                .arg(pcolor->qcolor.name(QColor::HexRgb))
                .arg(QString::number(text_tag_link_type(ptag)))
                .arg(QString::number(text_tag_link_id(ptag))));
    }
    }
  } text_tag_list_iterate_end;


  QMapIterator<int, QString> i(mm);
  int p = 0;
  QByteArray res;
  while (i.hasNext()) {
    i.next();
    res += bytes.left(i.key() - p);
    res += i.value();
    bytes = bytes.right(bytes.size() - i.key() + p);
    p = i.key();
  }

  // qCDebug(FC) << res;
  if (res.isEmpty()) return s;
  return QString(res);
}

void Application::ChatMessage(const char *s, const text_tag_list *tags, int) {
  // qCDebug(FC) << "Application::ChatMessage";

  QString wakeup(gui_options.gui_qt_wakeup_text);
  /* Format wakeup string if needed */
  if (wakeup.contains("%1")) {
    wakeup = wakeup.arg(client.conn.username);
  }

  QString str(s);
  if (str.contains(client.conn.username)) {
    qApp->alert(instance()->m_mainWindow);
  }

  /* Play sound if we encountered wakeup string */
  if (str.contains(wakeup) && client_state() < C_S_RUNNING && !wakeup.isEmpty()) {
    qApp->alert(instance()->m_mainWindow);
    audio_play_sound("e_player_wake", nullptr);
  }

  instance()->chatMessage(ApplyTags(s, tags));
}

QFont Application::Font(enum client_font /*font*/) {
  static bool once = true;
  if (once) {
    qCDebug(FC) << "TODO: Application::Font";
    once = false;
  }
  return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

QIcon Application::Icon(const QString& name) {
  QIcon icon;

  /* Try custom icon from theme */
  auto path = "themes/gui-qt/" + ThemesManager::Current() + "/" +
      name + ".png";
  icon.addFile(fileinfoname(get_data_dirs(), path.toLocal8Bit().data()));

  /* Try icon from icons dir */
  if (icon.isNull()) {
    path = "themes/gui-qt/icons/" + name + ".png";
    icon.addFile(fileinfoname(get_data_dirs(), path.toLocal8Bit().data()));
  }

  return QIcon(icon);
}

QPixmap Application::SaneMargins(const QPixmap &pix, const QSize& frame) {
  QImage img = pix.toImage().convertToFormat(QImage::Format_ARGB32);

  int xmax = 0;
  int xmin = img.width();
  int ymin = img.height();
  int ymax = 0;
  for (int y = 0; y < img.height(); y++) {
    QRgb row[img.width()];
    bool rowFilled = false;

    /* Copy to a location with guaranteed QRgb suitable alignment.
       * That fixes clang compiler warning. */
    memcpy(row, img.scanLine(y), img.width() * sizeof(QRgb));

    for (int x = 0; x < img.width(); ++x) {
      if (qAlpha(row[x])) {
        rowFilled = true;
        xmax = qMax(xmax, x);
        if (xmin > x) {
          xmin = x;
          x = xmax;
        }
      }
    }
    if (rowFilled) {
      ymin = qMin(ymin, y);
      ymax = y;
    }
  }

  QRect cr(xmin, ymin, xmax - xmin + 1, ymax - ymin + 1);
  if (!cr.isValid()) return pix;
  int x = (frame.width() - cr.width()) / 2;
  int y = (frame.height() - cr.height()) / 2;
  // qCDebug(FC) << cr << pix.rect() << frame << x << y;
  QPixmap res(frame);
  res.fill(Qt::transparent);
  QPainter p;
  p.begin(&res);
  p.drawPixmap(x, y, pix.copy(cr));
  p.end();
  return res;
}

MainWindow* Application::Mainwin() {
  return instance()->m_mainWindow;
}

void Application::SetRulesets(int num_rulesets, char **rulesets) {
  QStringList rules;
  for (int i = 0; i < num_rulesets; i++) {
    rules << rulesets[i];
  }
  qCDebug(FC) << "Application::SetRulesets" << rules;
  instance()->rulesetMessage(rules);
}

void Application::AddServerSource(int sock) {
  qCDebug(FC) << "Application::AddServerSource";
  instance()->addServerSource(sock);
}

void Application::RemoveServerSource() {
  qCDebug(FC) << "Application::RemoveServerSource";
  instance()->removeServerSource();
}

void Application::UpdateUsers(void *) {
  qCDebug(FC) << "Application::UpdateUsers";
  instance()->updateUsers();
}

void Application::UpdateCursor(cursor_type ct) {
  instance()->updateCursor(ct);
}

void Application::FlushDirty() {
  instance()->flushDirty();
}

void Application::DirtyAll() {
  instance()->dirtyAll();
}

void Application::DirtyRect(const QRect& r) {
  instance()->dirtyRect(r);
}

void Application::UpdateUnitInfo(unit_list *punits) {
  instance()->updateUnitInfo(punits);
}

void Application::UpdateGameInfo() {
  instance()->updateGameInfo();
}

void Application::UpdateTurnTimeout() {
  instance()->updateTurnTimeout();
}

void Application::ToggleTurnDone(bool on) {
  instance()->toggleTurnDone(on);
}

void Application::UpdateTurnDone(bool on) {
  instance()->updateTurnDone(on);
}

void Application::CreateLineAtMousePos() {
  instance()->createLineAtMousePos();
}

void Application::UnitSelectDialog(tile *ptile) {
  instance()->unitSelectDialog(ptile);
}

void Application::FlushMapview() {
  instance()->flushMapview();
}

void Application::UpdateMessages() {
  instance()->updateMessages();
}

void Application::UpdateReport(const QStringList &report) {
  instance()->updateReport(report);
}

void Application::PopupPlayers() {
  instance()->popupPlayers();
}

void Application::UpdatePlayers() {
  instance()->updatePlayers();
}

void Application::InitMeeting(int counterpart, int initiator) {
  instance()->initMeeting(counterpart, initiator);
}

void Application::CancelMeeting(int counterpart, int canceler) {
  instance()->cancelMeeting(counterpart, canceler);
}

void Application::CreateClause(int counterpart, const Clause& clause) {
  instance()->createClause(counterpart, clause);
}

void Application::RemoveClause(int counterpart, const Clause& clause) {
  instance()->removeClause(counterpart, clause);
}

void Application::AcceptTreaty(int counterpart, bool resolution) {
  instance()->acceptTreaty(counterpart, resolution);
}

void Application::CloseAllTreatyDialogs() {
  instance()->closeAllTreatyDialogs();
}

void Application::PopupCityReport() {
  instance()->popupCityReport();
}

void Application::UpdateCityReport() {
  instance()->updateCityReport();
}

void Application::UpdateCity(city* c) {
  instance()->updateCity(c);
}

void Application::RefreshCityDialog(city* c, bool popup) {
  instance()->refreshCityDialog(c, popup);
}

void Application::PopdownCityDialog(city* c) {
  instance()->popdownCityDialog(c);
}

void Application::UpdateUnitSelector() {
  instance()->updateUnitSelector();
}

void Application::PopupScienceReport() {
  instance()->popupScienceReport();
}

void Application::UpdateScienceReport() {
  instance()->updateScienceReport();
}

void Application::UpdateActions() {
  instance()->updateActions();
}

void Application::PopdownNationDialog() {
  instance()->popdownNationDialog();
}

void Application::RefreshNationDialog(bool nationsetChanged) {
  instance()->refreshNationDialog(nationsetChanged);
}

void Application::UpdateUnitReport() {
  instance()->updateUnitReport();
}

void Application::PopupUnitReport() {
  instance()->popupUnitReport();
}

void Application::UpdateEconomyReport() {
  instance()->updateEconomyReport();
}

void Application::PopupEconomyReport() {
  instance()->popupEconomyReport();
}

void Application::PopupHelpDialog(const QString &topic, help_page_type section) {
  instance()->popupHelpDialog(topic, section);
}

void Application::PopdownHelpDialog() {
  instance()->popdownHelpDialog();
}

void Application::UpdateOption(const option *opt) {
  instance()->updateOption(opt);
}

void Application::AddOption(option *opt) {
  instance()->addOption(opt);
}

void Application::DelOption(const option *opt) {
  instance()->delOption(opt);
}

void Application::AddIdleCallback(void callback(void *), void *data) {
  // qCDebug(FC) << "AddIdleCallback";
  instance()->addIdleCallback(callback, data);
}

void Application::StateChange(client_pages page) {
  qCDebug(FC) << "Application::StateChange" << client_pages_name(page);
  instance()->stateChange(page);
}

client_pages Application::CurrentState() {
  qCDebug(FC) << "Application::CurrentState";
  return instance()->m_mainWindow->state();
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
    // qCDebug(FC) << "Processing idle task";
    auto cb = m_tasks.takeFirst();
    cb.callback(cb.data);
  }
}

Application::Application()
  : m_mainWindow(nullptr)
  , m_timer(new QTimer)
  , m_notifier(nullptr)
  , m_completionList()
{
  m_timer->setSingleShot(true);
  connect(m_timer, &QTimer::timeout, this, &Application::timerRestart);
  m_timer->start();
}

void Application::timerRestart() {
  m_timer->start(real_timer_callback() * 1000);
}

void Application::addServerSource(int sock) {
  if (m_notifier) removeServerSource();
  if (QCoreApplication::instance() == nullptr) return;
  m_notifier = new QSocketNotifier(sock, QSocketNotifier::Read, this);
  connect(m_notifier, &QSocketNotifier::activated, this, &Application::serverInput);
}

void Application::removeServerSource() {
  m_notifier->deleteLater();
  m_notifier = nullptr;
}

void Application::serverInput(int sock) {
  input_from_server(sock);
}

void Application::updateUsers() {
  QString s;
  QSet<QString> fresh;
  conn_list_iterate(game.est_connections, pconn) {
    if (pconn->playing) {
      fresh << pconn->playing->name;
      fresh << pconn->playing->username;
    } else {
      fresh << pconn->username;
    }
  } conn_list_iterate_end;

  players_iterate (pplayer){
    fresh << pplayer->name;
  } players_iterate_end;

  if (fresh != QSet<QString>::fromList(m_completionList)) {
    m_completionList = fresh.values();
    emit completionListChanged(m_completionList);
  }

  emit playersChanged();
}

Application::~Application() {
  m_timer->stop();
  delete m_timer;
}
