#include "startdialog.h"
#include "ui_startdialog.h"
#include "application.h"
#include "nationdialog.h"
#include <QPainter>
#include <QMenu>
#include <QAction>
#include "sprite.h"
#include "colors.h"
#include <KConfigGroup>
#include <KWindowConfig>
#include <KSharedConfig>
#include <QWindow>


#include "game.h"
#include "tilespec.h"
#include "chat.h"


#include "connectdlg_common.h"
#include "client_main.h"
#include "climisc.h"
#include "chatline_common.h"

using namespace KV;

StartDialog::StartDialog(QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::StartDialog)
  , m_nationDialog(new NationDialog(this))
{
  m_ui->setupUi(this);

  // Player tree
  QStringList headers{
    _("Name"), _("Ready"), Q_("?player:Leader"),_("Flag"),
    _("Border"), _("Nation"), _("Team"), _("Host")
  };

  m_ui->playerTree->setColumnCount(headers.count());
  m_ui->playerTree->setHeaderLabels(headers);
  m_ui->playerTree->setContextMenuPolicy(Qt::CustomContextMenu);
  m_ui->playerTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);


  connect(m_ui->playerTree, &QTreeWidget::customContextMenuRequested,
          this, &StartDialog::popupTreeMenu);

  connect(Application::instance(), &Application::playersChanged,
          this, &StartDialog::populateTree);

  connect(Application::instance(), &Application::popdownNationDialog,
          m_nationDialog, &NationDialog::accept);

  connect(Application::instance(), &Application::refreshNationDialog,
          m_nationDialog, &NationDialog::refresh);

  // Rules combo
  connect(m_ui->rulesCombo, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
          this, &StartDialog::rulesetChange);
  connect(Application::instance(), &Application::rulesetMessage,
          this, &StartDialog::setRulesets);

  // Players spin
  m_ui->playersSpin->setRange(1, MAX_NUM_PLAYERS);
  connect(m_ui->playersSpin, QOverload<int>::of(&QSpinBox::valueChanged),
          this, &StartDialog::maxPlayersChanged);

  // AI combo
  for (int i = 0; i < AI_LEVEL_COUNT; i++) {
    auto level = static_cast<ai_level>(i);
    if (is_settable_ai_level(level)) {
      QString name = ai_level_translated_name(level);
      m_ui->aiCombo->addItem(name, i);
    }
  }
  m_ui->aiCombo->setCurrentIndex(-1);
  connect(m_ui->aiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &StartDialog::aiLevelChanged);

  // Nation button
  connect(m_ui->nationButton, &QPushButton::clicked,
          this, [=] () {this->pickNation(client_player());});

  // Observe button
  connect(m_ui->observeButton, &QPushButton::clicked,
          this, &StartDialog::observe);

  updateButtons();
  setWindowTitle(QString("%1: pregame settings").arg(qAppName()));

  create(); // ensure there's a window created
  const KConfigGroup cnf(KSharedConfig::openConfig(), "StartDialog");
  KWindowConfig::restoreWindowSize(windowHandle(), cnf);
  resize(windowHandle()->size());
}

StartDialog::~StartDialog() {
  KConfigGroup cnf(KSharedConfig::openConfig(), "StartDialog");
  KWindowConfig::saveWindowSize(windowHandle(), cnf);
  delete m_ui;
}


void StartDialog::populateTree() {

  if (conn_list_size(game.est_connections) == 0) return;

  m_ui->playerTree->clear();

  auto playerRoot = new QTreeWidgetItem();
  playerRoot->setText(0, Q_("?header:Players"));
  playerRoot->setData(0, Qt::UserRole, QVariant::fromValue(0));

  players_iterate(pplayer) {
    if (player_has_flag(pplayer, PLRF_SCENARIO_RESERVED)) continue;
    if (is_barbarian(pplayer)) continue;

    auto playerItem = new QTreeWidgetItem();
    playerItem->setData(0, Qt::UserRole, QVariant::fromValue(1));
    playerItem->setData(1, Qt::UserRole, QVariant::fromValue((void *) pplayer));

    // Column 0 = Name
    if (is_ai(pplayer)) {
      auto ainame = QString(pplayer->username) + " <" +
          ai_level_translated_name(pplayer->ai_common.skill_level) + ">";
      playerItem->setIcon(0, Application::Icon("ai"));
      playerItem->setText(0, ainame);
    } else {
      playerItem->setIcon(0, Application::Icon("human"));
      playerItem->setText(0, pplayer->username);
    }
    // Column 1 = Readiness
    playerItem->setText(1, is_ai(pplayer) || pplayer->is_ready ? _("Yes") : _("No"));
    // Column 2 = Leader
    playerItem->setText(2, player_name(pplayer));
    // Column 3 = Flag
    if (pplayer->nation) {
      playerItem->setData(3, Qt::DecorationRole,
                          get_nation_flag_sprite(get_tileset(), pplayer->nation)->pm);
    }
    // Column 4 = Border color
    if (player_has_color(get_tileset(), pplayer)) {
      QPixmap pm(m_ui->playerTree->header()->sectionSizeHint(4), 16);
      pm.fill(Qt::transparent);
      QPainter p;
      p.begin(&pm);
      p.fillRect(pm.width() / 2 - 8, 0, 16, 16, Qt::black);
      p.fillRect(pm.width() / 2 - 7, 1, 14, 14, get_player_color(get_tileset(), pplayer)->qcolor);
      p.end();
      playerItem->setData(4, Qt::DecorationRole, pm);
    }
    // Column 5 = Nation
    if (pplayer->nation != NO_NATION_SELECTED) {
      playerItem->setText(5, nation_adjective_for_player(pplayer));
    } else {
      playerItem->setText(5, _("Random"));
    }
    // Column 6 = Team
    playerItem->setText(6, pplayer->team ? team_name_translation(pplayer->team) : "");
    // Column 7 = Host
    QString host;
    int conn_id = -1;
    conn_list_iterate(pplayer->connections, pconn) {
      if (pconn->playing == pplayer && !pconn->observer) {
        conn_id = pconn->id;
        host = pconn->addr;
        break;
      }
    } conn_list_iterate_end;
    playerItem->setText(7, host);

    // find any custom observers
    conn_list_iterate(pplayer->connections, pconn) {
      if (pconn->id == conn_id) {
        continue;
      }
      auto observerItem = new QTreeWidgetItem();
      observerItem->setText(0, pconn->username); // Name
      observerItem->setText(5, _("Observer")); // Nation
      observerItem->setText(7, pconn->addr); // Host
      playerItem->addChild(observerItem);
    } conn_list_iterate_end;

    playerRoot->addChild(playerItem);

  } players_iterate_end;

  m_ui->playerTree->insertTopLevelItem(0, playerRoot);

  // Global observers
  auto observerRoot = new QTreeWidgetItem();

  observerRoot->setText(0, _("Global observers"));
  observerRoot->setData(0, Qt::UserRole, QVariant::fromValue(0));

  conn_list_iterate(game.est_connections, pconn) {
    if (pconn->playing || !pconn->observer) continue;
    auto observerItem = new QTreeWidgetItem();
    observerItem->setText(0, pconn->username); // Name
    observerItem->setText(5, _("Observer")); // Nation
    observerItem->setText(7, pconn->addr); // Host
    observerRoot->addChild(observerItem);
  } conn_list_iterate_end;

  m_ui->playerTree->insertTopLevelItem(1, observerRoot);

  // Detached
  auto detachedRoot = new QTreeWidgetItem();
  detachedRoot->setText(0, _("Detached"));
  detachedRoot->setData(0, Qt::UserRole, QVariant::fromValue(0));

  conn_list_iterate(game.all_connections, pconn) {
    if (pconn->playing || pconn->observer) continue;
    auto detachedItem = new QTreeWidgetItem();
    detachedItem->setText(0, pconn->username); // Name
    detachedItem->setText(7, pconn->addr); // Host
    detachedRoot->addChild(detachedItem);
  } conn_list_iterate_end;

  m_ui->playerTree->insertTopLevelItem(2, detachedRoot);

  m_ui->playerTree->expandAll();
  updateButtons();
}

static QString serverCommand(const QString& s) {
  return QString("%1%2").arg(SERVER_COMMAND_PREFIX).arg(s);
}

void StartDialog::popupTreeMenu(const QPoint &p) {
  QTreeWidgetItem *item = m_ui->playerTree->itemAt(p);
  if (!item) return;
  if (item->data(0, Qt::UserRole).toInt() == 0) return; // label

  // consistency check
  auto selected = static_cast<player*>(item->data(1, Qt::UserRole).value<void*>());
  bool ok = false;
  players_iterate(pplayer) {
    if (selected && selected == pplayer) {
      ok = true;
      break;
    }
  } players_iterate_end;
  if (!ok) return;

  QString me = QString(client.conn.username).replace("\"", "");


  auto menu = new QMenu(this);


  auto name = QString("\"%1\"").arg(selected->name);
  if (me != selected->username) {
    auto a = new QAction(_("Observe"));
    connect(a, &QAction::triggered, this, [=]() {
      send_chat(serverCommand(QString("observe %1").arg(name)).toUtf8());
    });
    menu->addAction(a);

    if (can_client_control()) {
      a = new QAction(_("Remove player"));
      connect(a, &QAction::triggered, this, [=]() {
        send_chat(serverCommand(QString("remove %1").arg(name)).toUtf8());
      });
      menu->addAction(a);
    }

    a = new QAction(_("Take this player"));
    connect(a, &QAction::triggered, this, [=]() {
      send_chat(serverCommand(QString("take %1").arg(name)).toUtf8());
      if (is_ai(selected)) {
        send_chat(serverCommand("away").toUtf8());
      }
    });
    menu->addAction(a);
  }

  if (can_conn_edit_players_nation(&client.conn, selected)) {
    auto a = new QAction(_("Pick nation"));
    connect(a, &QAction::triggered, this, [=]() {
      m_nationDialog->init(selected);
      m_nationDialog->show();
    });
    menu->addAction(a);
  }

  if (is_ai(selected) && can_client_control()) {
    // Set AI difficulty submenu
    auto submenu = new QMenu;
    submenu->setTitle(_("Set difficulty"));
    menu->addMenu(submenu);

    for (int i = 0; i < AI_LEVEL_COUNT; i++) {
      auto level = static_cast<ai_level>(i);
      if (is_settable_ai_level(level)) {
        auto a = new QAction(ai_level_translated_name(level));
        connect(a, &QAction::triggered, this, [=] () {
          send_chat(serverCommand(QString("%1 %2")
                                  .arg(ai_level_cmd(level))
                                  .arg(name)).toUtf8());
        });
        submenu->addAction(a);
      }
    }
  }

  // Put to Team X submenu
  if (game.info.is_new_game) {
    auto submenu = new QMenu;
    submenu->setTitle(_("Put on team"));
    menu->addMenu(submenu);
    int count = selected->team ? player_list_size(team_members(selected->team)) : 0;
    bool need_empty_team = count != 1;
    team_slots_iterate(tslot) {
      if (!team_slot_is_used(tslot)) {
        if (!need_empty_team) {
          continue;
        }
        need_empty_team = false;
      }
      auto a = new QAction(team_slot_name_translation(tslot));
      connect(a, &QAction::triggered, this, [=] () {
        send_chat(serverCommand(QString("team %1 \"%2\"")
                                .arg(name)
                                .arg(team_slot_rule_name(tslot)))
                  .toUtf8());
      });
      submenu->addAction(a);
    } team_slots_iterate_end;
  }

  if (can_client_control()) {
    auto a = new QAction(_("Aitoggle player"));
    connect(a, &QAction::triggered, this, [=] () {
      send_chat(serverCommand(QString("aitoggle %1").arg(name)).toUtf8());
    });
    menu->addAction(a);
  }

  menu->popup(m_ui->playerTree->mapToGlobal(p));
}


void StartDialog::updateButtons() {

  // Player spin
  int i = 0;
  players_iterate(pplayer) {
    i++;
  } players_iterate_end;

  m_ui->playersSpin->blockSignals(true);
  m_ui->playersSpin->setValue(i);
  m_ui->playersSpin->blockSignals(false);

  // Observe button
  if (client_is_observer() || client_is_global_observer()) {
    m_ui->observeButton->setText(_("Don't Observe"));
  } else {
    m_ui->observeButton->setText(_("Observe"));
  }

  bool sensitive;
  QString text;

  // Ok button
  if (can_client_control()) {
    sensitive = true;
    if (client_player()->is_ready) {
      text = _("Not ready");
    } else {
      int num_unready = 0;

      players_iterate(pplayer) {
        if (is_human(pplayer) && !pplayer->is_ready) {
          num_unready++;
        }
      } players_iterate_end;

      if (num_unready > 1) {
        text = _("Ready");
      } else {
        /* We are the last unready player so clicking here will
         * immediately start the game. */
        text = ("Start");
      }
    }
  } else {
    text = _("Start");
    if (can_client_access_hack() && client.conn.observer == TRUE) {
      sensitive = true;
      players_iterate(plr) {
        if (is_human(plr)) {
          /* There's human controlled player(s) in game, so it's their
           * job to start the game. */
          sensitive = false;
          break;
        }
      } players_iterate_end;
    } else {
      sensitive = false;
    }
  }
  m_ui->startButton->setEnabled(sensitive);
  m_ui->startButton->setText(text);

  /* Nation button */
  sensitive = game.info.is_new_game && can_client_control();
  m_ui->nationButton->setEnabled(sensitive);

  // AI combo
  ai_level level = server_ai_level();
  if (ai_level_is_valid(level)) {
    m_ui->aiCombo->setCurrentIndex(m_ui->aiCombo->findData(level));
  } else {
    m_ui->aiCombo->setCurrentIndex(-1);
  }
}

void StartDialog::rulesetChange(const QString& rules) {
  set_ruleset(rules.toUtf8());
}

void StartDialog::setRulesets(const QStringList &sets) {
  m_ui->rulesCombo->blockSignals(true);
  QString c(m_ui->rulesCombo->currentText());
  m_ui->rulesCombo->clear();
  bool ok = false;
  for (auto s: sets) {
    m_ui->rulesCombo->addItem(s);
    if (s == c) {
      m_ui->rulesCombo->setCurrentText(s);
      ok = true;
    }
  }
  if (!ok) {
    m_ui->rulesCombo->setCurrentText("default");
  }
  m_ui->rulesCombo->blockSignals(false);
}

void StartDialog::maxPlayersChanged(int numPlayers) {
  option_int_set(optset_option_by_name(server_optset, "aifill"), numPlayers);
}

void StartDialog::pickNation(const player* p) {
  m_nationDialog->init(p);
  m_nationDialog->show();
}




void StartDialog::aiLevelChanged(int) {

  QVariant v = m_ui->aiCombo->currentData();
  if (v.isValid()) {
    ai_level k = static_cast<ai_level>(v.toInt());
    /* Suppress changes provoked by server rather than local user */
    if (server_ai_level() != k) {
      send_chat(serverCommand(ai_level_cmd(k)).toUtf8());
    }
  }

}


void StartDialog::observe() {
  if (client_is_observer() || client_is_global_observer()) {
    if (game.info.is_new_game) {
      send_chat(serverCommand("take -").toUtf8());
    } else {
      send_chat(serverCommand("detach").toUtf8());
    }
    m_ui->observeButton->setText(_("Don't Observe"));
  } else {
    send_chat(serverCommand("observe").toUtf8());
    m_ui->observeButton->setText(_("Observe"));
  }
}










