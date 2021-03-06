#include "state.h"
#include "mainwindow.h"
#include "logging.h"
#include "networkdialog.h"
#include "startdialog.h"
#include "messagebox.h"
#include "mapview.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QStateMachine>
#include "themesmanager.h"
#include "serveroptionsdialog.h"
#include "chatlineedit.h"
#include <QTimer>

#include "tilespec.h"
#include "version.h"
#include "fc_config.h"
#include "clinet.h"
#include "client_main.h"
#include "connectdlg_common.h"


using namespace KV::State;

/**
 * Intro
 */

Intro::Intro(KV::MainWindow* parent)
  : Base(parent, PAGE_MAIN)
{}

Intro::~Intro() {}

QWidget* Intro::createIntroWidget() {
  QPixmap intro_pm(tileset_main_intro_filename(get_tileset()));
  QPainter painter(&intro_pm);
  painter.setPen(Qt::white);
  QFontMetrics fm(QApplication::font());

  char msgbuf[128];
  const char* rev_ver = fc_git_revision();

  if (!rev_ver) {
    /* TRANS: "version 2.6.0, Qt client" */
    fc_snprintf(msgbuf, sizeof(msgbuf), _("%s%s, KDE client"),
                word_version(), VERSION_STRING);
  } else {
    fc_snprintf(msgbuf, sizeof(msgbuf), "%s%s",
                word_version(), VERSION_STRING);
    painter.drawText(10,
                     intro_pm.height() - fm.descent() - fm.height() * 2,
                     msgbuf);

    /* TRANS: "commit: [modified] <git commit id>" */
    fc_snprintf(msgbuf, sizeof(msgbuf), _("commit: %s"), rev_ver);
    painter.drawText(10,
                     intro_pm.height() - fm.descent() - fm.height(),
                     msgbuf);

    strncpy(msgbuf, _("KDE client"), sizeof(msgbuf) - 1);
  }

  painter.drawText(intro_pm.width() - fm.width(msgbuf)-10,
                   intro_pm.height() - fm.descent(), msgbuf);

  auto intro_label = new QLabel;
  intro_label->setPixmap(intro_pm);
  intro_label->setScaledContents(false);

  auto intro_layout = new QVBoxLayout;
  intro_layout->addWidget(intro_label, 0, Qt::AlignHCenter);

  auto w = new QWidget;
  w->setLayout(intro_layout);
  return w;
}

void Intro::onEntry(QEvent* event) {
  auto v = qobject_cast<KV::MapView*>(m_parent->centralWidget());
  if (v) {
    // a game is already active - leave intro state
    emit playing();
    return;
  }
  m_parent->setCentralWidget(createIntroWidget());
  QState::onEntry(event);
}

/**
  * Network
  */

Network::Network(MainWindow *parent)
  : Base(parent, PAGE_NETWORK)
  , m_networkDialog(new KV::NetworkDialog(parent))
{
   connect(m_networkDialog, &QDialog::accepted, this, &Network::connectToServer);
   connect(m_networkDialog, &QDialog::rejected, this, &Network::rejected);
}

void Network::connectToServer() {
  char errbuf [512];

  sz_strlcpy(user_name, m_networkDialog->user().toLocal8Bit().data());
  sz_strlcpy(server_host, m_networkDialog->server().toLocal8Bit().data());
  server_port = m_networkDialog->port();

  if (connect_to_server(user_name, server_host, server_port,
                        errbuf, sizeof(errbuf)) != -1) {
    emit accepted("");
  } else {
    KV::MessageBox fail(m_parent->centralWidget(), errbuf, "Connection failed");
    fail.setStandardButtons(QMessageBox::Ok);
    fail.exec();
    emit rejected();
  }
}

Network::~Network() {
  delete m_networkDialog;
}

void Network::onEntry(QEvent* event) {
  if (event->type() == QEvent::StateMachineSignal) {
    auto ev = static_cast<QStateMachine::SignalEvent*>(event);
    if (ev->sender() == m_parent->action("connectToGame")) {
      m_networkDialog->init();
      m_networkDialog->show();
    } else if (ev->signalIndex() ==
               ev->sender()->metaObject()->indexOfSignal("startNewGame(QString)")) {
      if (!is_server_running()) {
        client_start_server();
      }
      emit accepted(ev->arguments().first().toString());
    }
  }
  QState::onEntry(event);
}


void Network::onExit(QEvent* event) {
  m_networkDialog->final();
  QState::onExit(event);
}


/*
 * Start
 */

Start::Start(MainWindow *parent)
  : Base(parent, PAGE_START)
  , m_startDialog(new KV::StartDialog(parent))
{
  connect(m_startDialog, &QDialog::accepted, this, &Start::playerReady);
  connect(m_startDialog, &QDialog::rejected, this, &Start::disconnectFromServer);
  connect(m_startDialog, &StartDialog::configLocal,
          m_parent->action("localOptions"), &QAction::triggered);
  connect(m_startDialog, &StartDialog::configServer,
          m_parent->action("serverOptions"), &QAction::triggered);
}


Start::~Start() {
  delete m_startDialog;
}

void Start::onEntry(QEvent* event) {
  // ensure server option validity
  m_parent->m_serverOptions->reset();
  m_startDialog->show();
  if (event->type() == QEvent::StateMachineSignal) {
    auto ev = static_cast<QStateMachine::SignalEvent*>(event);
    if (ev->signalIndex() ==
        ev->sender()->metaObject()->indexOfSignal("accepted(QString)")) {
      QString loadFile = ev->arguments().first().toString();
      if (!loadFile.isEmpty()) {
        // give the server some time to boot - otherwise we get disconnected
        QTimer::singleShot(500, this, [loadFile] () {
          Chat::sendServerCommand(QString("load %1").arg(loadFile));
        });
      }
    }
  }
  QState::onEntry(event);
}

void Start::disconnectFromServer() {
  disconnect_from_server();
  emit rejected();
}

void Start::playerReady() {
  if (can_client_control()) {
    dsend_packet_player_ready(&client.conn,
                              player_number(client_player()),
                              !client_player()->is_ready);
  } else {
    dsend_packet_player_ready(&client.conn, 0, TRUE);
  }

  emit accepted();
}

/*
 * Game
 */

Game::Game(MainWindow *parent)
  : Base(parent, PAGE_GAME)
{
}

KV::MapView* Game::createMapWidget() {
  auto view = new MapView(m_parent);
  return view;
}

Game::~Game() {}

void Game::onEntry(QEvent* event) {
  m_parent->setMapView(createMapWidget());
  m_parent->enableGameMenus(true);
  if (m_parent->m_fallbackGeom.isValid()) {
    // go fullscreen
    m_parent->action("fullScreen")->setChecked(true);
  }
  QState::onEntry(event);
}

void Game::onExit(QEvent* event) {
  m_parent->enableGameMenus(false);
  m_parent->setCentralWidget(new QWidget);
  QState::onExit(event);
}





