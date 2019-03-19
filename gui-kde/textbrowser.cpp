#include "textbrowser.h"
#include <QUrl>

#include "mapview_common.h"
#include "game.h"
#include "climap.h"


using namespace KV;

TextBrowser::TextBrowser(QWidget *parent)
  : QTextBrowser(parent)
{
  setOpenLinks(false);
  connect(this, &TextBrowser::anchorClicked,
          this, &TextBrowser::handleAnchorClick);
}


void TextBrowser::handleAnchorClick(const QUrl &link) {
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
      handleError(_("This city isn't known!"));
    }
  }
    break;
  case TLT_TILE:
    ptile = index_to_tile(&(wld.map), id);
    if (!ptile) {
      handleError(_("This tile doesn't exist in this game!"));
    }
    break;
  case TLT_UNIT: {
    unit* punit = game_unit_by_number(id);
    if (punit) {
      ptile = unit_tile(punit);
    } else {
      handleError(_("This unit isn't known!"));
    }
  }
    break;
  }

  if (ptile) {
    center_tile_mapcanvas(ptile);
    link_mark_restore(link_type, id);
  }
}
