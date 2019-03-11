#include "govmenu.h"
#include "sprite.h"
#include "messagebox.h"
#include "application.h"
#include "logging.h"

#include "client_main.h"
#include "game.h"
#include "tilespec.h"
#include "government.h"
#include "packhand.h"


using namespace KV;

GovMenu::GovMenu(QWidget *parent)
  :QMenu(_("Government"), parent)
{
  setAttribute(Qt::WA_TranslucentBackground);

  auto revolGov = game.government_during_revolution;

  auto action = addAction(_("Revolution..."));
  auto sprite = get_government_sprite(tileset, revolGov);
  if (sprite != nullptr) {
    action->setIcon(QIcon(sprite->pm));
  }
  connect(action, &QAction::triggered, this, [=] () {
    popupRevolutionDialog();
  });

  addSeparator();

  int cnt = government_count();
  for (int i = 0; i < cnt; ++i) {
    auto gov = government_by_number(i);
    if (gov == revolGov) continue;
    action = addAction(government_name_translation(gov));
    action->setData(i);
    sprite = get_government_sprite(tileset, gov);
    if (sprite != nullptr) {
      action->setIcon(QIcon(sprite->pm));
    }
    connect(action, &QAction::triggered, this, [=] () {
      popupRevolutionDialog(government_by_number(i));
    });
  }
  updateGov();

  connect(Application::instance(), &Application::updateGameInfo,
          this, &GovMenu::updateGov);

}

void GovMenu::popupRevolutionDialog(government *g) const
{
  bool revo = true;
  if (client.conn.playing->revolution_finishes < 0) {
    KV::StandardMessageBox ask(nullptr,
                               _("You say you wanna revolution?"),
                               _("Revolution!"));
    revo = ask.exec() == QMessageBox::Ok;
  }

  if (revo) {
    if (!g) {
      start_revolution();
    } else {
      set_government_choice(g);
    }
  }
}

QAction* GovMenu::findAction(int i) const {
  for (auto a: actions()) {
    bool ok;
    int d = a->data().toInt(&ok);
    if (!ok) continue;
    if (d == i) return a;
  }
  return nullptr;
}

void GovMenu::updateGov()
{
  auto cnt = government_count();

  actions()[0]->setEnabled(!client_is_observer());

  for (int i = 0; i < cnt; ++i) {
    auto a = findAction(i);
    if (!a) continue;
    auto gov = government_by_number(i);
    a->setEnabled(can_change_to_government(client.conn.playing, gov));
  }
}


