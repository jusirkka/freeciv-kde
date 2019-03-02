#include "startdialog.h"
#include "ui_startdialog.h"
#include "application.h"
#include <QPainter>

#include "game.h"
#include "tilespec.h"
#include "sprite.h"
#include "colors.h"

using namespace KV;

StartDialog::StartDialog(QWidget *parent)
  : QDialog(parent)
  , m_UI(new Ui::StartDialog)
{
  m_UI->setupUi(this);
  QStringList headers{
    _("Name"), _("Ready"), Q_("?player:Leader"),_("Flag"),
    _("Border"), _("Nation"), _("Team"), _("Host")
  };

  m_UI->playerTree->setColumnCount(headers.count());
  m_UI->playerTree->setHeaderLabels(headers);
  m_UI->playerTree->setContextMenuPolicy(Qt::CustomContextMenu);
  m_UI->playerTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);


  connect(m_UI->playerTree, &QTreeWidget::customContextMenuRequested,
          this, &StartDialog::popupTreeMenu);

  connect(Application::instance(), &Application::updateUsers,
          this, &StartDialog::populateTree);


  setWindowTitle(qAppName());
}

void StartDialog::populateTree() {

  if (conn_list_size(game.est_connections) == 0) return;

  m_UI->playerTree->clear();

  auto playerRoot = new QTreeWidgetItem();
  playerRoot->setText(0, Q_("?header:Players"));
  playerRoot->setData(0, Qt::UserRole, QVariant::fromValue(0));

  players_iterate(pplayer) {
    if (player_has_flag(pplayer, PLRF_SCENARIO_RESERVED)) continue;
    if (is_barbarian(pplayer)) continue;

    auto playerItem = new QTreeWidgetItem();
    playerItem->setData(0, Qt::UserRole, QVariant::fromValue(1));
    playerItem->setData(1, Qt::UserRole, QVariant::fromValue((void *) pplayer));

    // Column 0 = name
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
                          get_nation_flag_sprite(tileset, pplayer->nation)->pm);
    }
    // Column 4 = Border color
    if (player_has_color(tileset, pplayer)) {
      QPixmap pm(m_UI->playerTree->header()->sectionSizeHint(4), 16);
      pm.fill(Qt::transparent);
      QPainter p;
      p.begin(&pm);
      p.fillRect(pm.width() / 2 - 8, 0, 16, 16, Qt::black);
      p.fillRect(pm.width() / 2 - 7, 1, 14, 14, get_player_color(tileset, pplayer)->qcolor);
      p.end();
      playerItem->setData(4, Qt::DecorationRole, pm);
    }
    // Column 5 = Nation
    if (pplayer->nation != NO_NATION_SELECTED) {
      playerItem->setText(5, nation_adjective_for_player(pplayer));
    } else {
      playerItem->setText(5, _("Random"));
    }
    // Column 6 = team
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

  m_UI->playerTree->insertTopLevelItem(0, playerRoot);

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

  m_UI->playerTree->insertTopLevelItem(1, observerRoot);

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

  m_UI->playerTree->insertTopLevelItem(2, detachedRoot);

  m_UI->playerTree->expandAll();
  updateButtons();
}


void StartDialog::popupTreeMenu(const QPoint &p) {
}


void StartDialog::updateButtons() {
}

StartDialog::~StartDialog()
{
  delete m_UI;
}













