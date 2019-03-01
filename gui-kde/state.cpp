#include "state.h"
#include "mainwindow.h"
#include "logging.h"
#include "network.h"
#include "mapwidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>

#include "tilespec.h"
#include "version.h"
#include "fc_config.h"


using namespace KV::State;

#define IN_PROGRESS 1

/**
 * Intro
 */

Intro::Intro(KV::MainWindow* parent)
  : Base(parent, PAGE_MAIN)
{}

Intro::~Intro() {}

QWidget* Intro::createIntroWidget() {
  QPixmap intro_pm(tileset_main_intro_filename(tileset));
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

#if IN_PROGRESS
  QLabel *beta_label = new QLabel("warning: development in progress");
  QPalette warn_color;
  warn_color.setColor(QPalette::WindowText, Qt::red);
  beta_label->setPalette(warn_color);
  beta_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Maximum);
  beta_label->setAlignment(Qt::AlignCenter);
  intro_layout->addWidget(beta_label, 0, Qt::AlignHCenter);
#endif

  auto w = new QWidget;
  w->setLayout(intro_layout);
  return w;
}

void Intro::onEntry(QEvent* event) {
  auto v = qobject_cast<KV::MapWidget*>(m_parent->centralWidget());
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
  , m_networkDialog(new KV::Network(parent))
{
   connect(m_networkDialog, &QDialog::accepted, this, &Network::accepted);
   connect(m_networkDialog, &QDialog::rejected, this, &Network::rejected);
}


Network::~Network() {
  delete m_networkDialog;
}

void Network::onEntry(QEvent* event) {
  m_networkDialog->show();
  m_networkDialog->init();
  QState::onEntry(event);
}


void Network::onExit(QEvent* event) {
  m_networkDialog->final();
  QState::onExit(event);
}

/*
 * Game
 */

Game::Game(MainWindow *parent)
  : Base(parent, PAGE_GAME)
{}

KV::MapWidget* Game::createMapWidget() {
  return new MapWidget;
}

Game::~Game() {}

void Game::onEntry(QEvent* event) {
  m_parent->setCentralWidget(createMapWidget());
  QState::onEntry(event);
}


/*
 * Start
 */

Start::Start(MainWindow *parent)
  : Base(parent, PAGE_START)
{}


Start::~Start() {}

void Start::onEntry(QEvent* event) {
  QState::onEntry(event);
}























