#include "chatwindow.h"
#include "application.h"
#include <QScrollBar>


#include "chatline_common.h"
#include "mapview_common.h"
#include "game.h"
#include "climap.h"

using namespace KV;


ChatWindow::ChatWindow(QWidget* parent)
  : QTextBrowser(parent)
{
  connect(Application::instance(), &Application::chatMessage,
          this, &ChatWindow::receive);
  connect(this, &ChatWindow::anchorClicked,
          this, &ChatWindow::handleAnchorClick);
}


void ChatWindow::receive(const QString &message) {
  append(message);
  verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum());
}

void ChatWindow::handleAnchorClick(const QUrl &link) {
  auto parts = link.toString().split(",");
  auto link_type = static_cast<text_link_type>(parts[0].toInt());
  auto id = parts[1].toInt();

  tile* ptile = nullptr;
  switch (link_type) {
  case TLT_CITY: {
    city* pcity = game_city_by_number(id);
    if (pcity) {
      ptile = client_city_tile(pcity);
    } else {
      output_window_append(ftc_client, _("This city isn't known!"));
    }
  }
    break;
  case TLT_TILE:
    ptile = index_to_tile(&(wld.map), id);
    if (!ptile) {
      output_window_append(ftc_client,
                           _("This tile doesn't exist in this game!"));
    }
    break;
  case TLT_UNIT: {
    unit* punit = game_unit_by_number(id);
    if (punit) {
      ptile = unit_tile(punit);
    } else {
      output_window_append(ftc_client, _("This unit isn't known!"));
    }
  }
    break;
  }

  if (ptile) {
    center_tile_mapcanvas(ptile);
    link_mark_restore(link_type, id);
  }
}
