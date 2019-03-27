#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "client_main.h"
#include "gui_interface.h"
#include "colors.h"
#include "canvas.h"
#include "game.h"
extern "C" {
#include "citydlg_g.h"
}

#include "themesmanager.h"
#include "application.h"
#include "spritefactory.h"
#include "logging.h"

#include <QPainter>
#include <QFontMetrics>
#include <QApplication>

#pragma GCC diagnostic pop

static void setup_gui_funcs() {

  auto funcs = get_gui_funcs();

  funcs->ui_init = KV::Application::Init;
  funcs->ui_main = KV::Application::Main;
  funcs->ui_exit = KV::Application::Exit;
  funcs->sound_bell = KV::Application::Beep;

  //   Returns gui type of the client
  funcs->get_gui_type = [] () {return GUI_QT;};

  funcs->insert_client_build_info = [] (char *, size_t) {
    qCDebug(FC) << "insert_client_build_info: dummy";
  };
  funcs->adjust_default_options = [] () {
    qCDebug(FC) << "TODO: adjust_default_options";
  };

  funcs->version_message = KV::Application::VersionMessage;
  funcs->real_output_window_append = KV::Application::ChatMessage;

  funcs->is_view_supported = [] (enum ts_type t) {
    if (t == TS_ISOMETRIC) return true;
    if (t == TS_OVERHEAD) return true;
    return false;
  };
  funcs->tileset_type_set = [] (enum ts_type) {
    qCDebug(FC) << "TODO: tileset_type_set";
  };
  funcs->free_intro_radar_sprites = [] () {
    qCDebug(FC) << "free_intro_radar_sprites: dummy";
  };
  funcs->load_gfxfile = KV::SpriteFactory::Load;
  funcs->create_sprite = KV::SpriteFactory::Create;
  funcs->get_sprite_dimensions = KV::SpriteFactory::Dimensions;
  funcs->crop_sprite = KV::SpriteFactory::Crop;
  funcs->free_sprite = KV::SpriteFactory::Free;

  funcs->color_alloc = [] (int r, int g, int b) {
    auto pcolor = new color;
    pcolor->qcolor.setRgb(r, g, b);
    return pcolor;
  };
  funcs->color_free = [] (struct color *pcolor) {
    delete pcolor;
  };

  funcs->canvas_create = [] (int width, int height) {
    // qCDebug(FC) << "canvas_create";
    struct canvas *store = new canvas;
    store->map_pixmap = QPixmap(width, height);
    return store;
  };
  funcs->canvas_free = [] (struct canvas *store) {
    // qCDebug(FC) << "canvas_free";
    delete store;
  };
  funcs->canvas_set_zoom = [] (canvas*, float) {
    // qCDebug(FC) << "TODO: canvas_set_zoom";
  };
  funcs->has_zoom_support = [] () {
    return false;
  };
  funcs->canvas_copy = [] (
      struct canvas *dest, struct canvas *src,
      int src_x, int src_y, int dest_x, int dest_y, int width,
      int height) {
    // qCDebug(FC) << "canvas_copy";

    QRectF source_rect(src_x, src_y, width, height);
    QRectF dest_rect(dest_x, dest_y, width, height);

    if (!width || !height) {
      return;
    }

    QPainter p;
    p.begin(&dest->map_pixmap);
    p.drawPixmap(dest_rect, src->map_pixmap, source_rect);
    p.end();
  };

  funcs->canvas_put_sprite = [] (      
      struct canvas *pcanvas,
      int canvas_x, int canvas_y,
      struct sprite *sprite,
      int offset_x, int offset_y, int width, int height) {

    // qCDebug(FC) << "canvas_put_sprite";
    QPainter p;
    p.begin(&pcanvas->map_pixmap);
    p.drawPixmap(canvas_x, canvas_y, sprite->pm, offset_x, offset_y, width, height);
    p.end();
  };
  funcs->canvas_put_sprite_full = [] (
      struct canvas *pcanvas,
      int canvas_x, int canvas_y,
      struct sprite *sprite) {

    // qCDebug(FC) << "canvas_put_sprite_full";
    int width, height;
    get_sprite_dimensions(sprite, &width, &height);
    canvas_put_sprite(pcanvas, canvas_x, canvas_y, sprite,
                      0, 0, width, height);
  };
  funcs->canvas_put_sprite_fogged = [] (
      struct canvas *pcanvas,
      int canvas_x, int canvas_y,
      struct sprite *psprite,
      bool /*fog*/, int /*fog_x*/, int /*fog_y*/) {

    // qCDebug(FC) << "canvas_put_sprite_fogged";
    QPainter p;
    p.begin(&pcanvas->map_pixmap);
    p.setCompositionMode(QPainter::CompositionMode_Difference);
    p.setOpacity(0.5);
    p.drawPixmap(canvas_x, canvas_y, psprite->pm);
    p.end();
  };
  funcs->canvas_put_rectangle = [] (
      struct canvas *pcanvas,
      struct color *pcolor,
      int canvas_x, int canvas_y,
      int width, int height) {

    // qCDebug(FC) << "canvas_put_rectangle";
    QBrush brush(pcolor->qcolor);
    QPen pen(pcolor->qcolor);
    QPainter p;
    p.begin(&pcanvas->map_pixmap);
    p.setPen(pen);
    p.setBrush(brush);
    if (width == 1 && height == 1) {
      p.drawPoint(canvas_x, canvas_y);
    } else if (width == 1) {
      p.drawLine(canvas_x, canvas_y, canvas_x, canvas_y + height -1);
    } else if (height == 1) {
      p.drawLine(canvas_x, canvas_y, canvas_x + width - 1, canvas_y);
    } else {
      p.drawRect(canvas_x, canvas_y, width, height);
    }
    p.end();
  };
  funcs->canvas_fill_sprite_area = [] (
      struct canvas *pcanvas,
      struct sprite *psprite, struct color *pcolor,
      int canvas_x, int canvas_y) {

    // qCDebug(FC) << "canvas_fill_sprite_area";
    int width, height;
    get_sprite_dimensions(psprite, &width, &height);
    canvas_put_rectangle(pcanvas, pcolor, canvas_x, canvas_y, width, height);
  };
  funcs->canvas_put_line = [] (
      struct canvas *pcanvas, struct color *pcolor,
      enum line_type ltype, int start_x, int start_y,
      int dx, int dy) {

    // qCDebug(FC) << "canvas_put_line";
    QPen pen;
    pen.setColor(pcolor->qcolor);
    pen.setWidth(1);
    if (ltype == LINE_BORDER) {
      pen.setStyle(Qt::DashLine);
      pen.setDashOffset(4);
    } else if (ltype == LINE_TILE_FRAME || ltype == LINE_GOTO) {
      pen.setWidth(2);
    }
    QPainter p;
    p.begin(&pcanvas->map_pixmap);
    p.setPen(pen);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawLine(start_x, start_y, start_x + dx, start_y + dy);
    p.end();
  };
  funcs->canvas_put_curved_line = [] (
      struct canvas *pcanvas, struct color *pcolor,
      enum line_type ltype, int start_x, int start_y,
      int dx, int dy) {

    // qCDebug(FC) << "canvas_put_curved_line";
    QPen pen;
    pen.setColor(pcolor->qcolor);
    QPainter p;
    QPainterPath path;
    pen.setWidth(1);
    if (ltype == LINE_BORDER) {
      pen.setStyle(Qt::DashLine);
      pen.setDashOffset(4);
      pen.setWidth(2);
    } else if (ltype == LINE_TILE_FRAME || ltype == LINE_GOTO) {
      pen.setWidth(2);
    }
    p.begin(&pcanvas->map_pixmap);
    p.setRenderHints(QPainter::Antialiasing);
    p.setPen(pen);
    path.moveTo(start_x, start_y);
    path.cubicTo(start_x + dx / 2, start_y, start_x, start_y + dy / 2,
                 start_x + dx, start_y + dy);
    p.drawPath(path);
    p.end();
  };
  funcs->get_text_size = [] (
      int *width, int *height,
      enum client_font font, const char *text) {

    // qCDebug(FC) << "get_text_size";
    auto afont = KV::Application::Font(font);
    auto fm = new QFontMetrics(afont);
    if (width) {
      *width = fm->width(QString::fromUtf8(text));
    }
    if (height) {
      *height = fm->height();
    }
    delete fm;
  };
  funcs->canvas_put_text = [] (
      struct canvas *pcanvas, int canvas_x, int canvas_y,
      enum client_font font, struct color *pcolor,
      const char *text) {

    // qCDebug(FC) << "canvas_put_text";
    QColor color(pcolor->qcolor);

    auto afont = KV::Application::Font(font);
    QPen pen;
    pen.setColor(color);
    auto fm = new QFontMetrics(afont);

    QPainter p;
    p.begin(&pcanvas->map_pixmap);
    p.setPen(pen);
    p.setFont(afont);
    p.drawText(canvas_x, canvas_y + fm->ascent(), QString::fromUtf8(text));
    p.end();
    delete fm;
  };

  funcs->set_rulesets = KV::Application::SetRulesets;
  funcs->options_extra_init = [] () {
    qCDebug(FC) << "TODO: options_extra_init";
  };
  funcs->server_connect = [] () {
    qCDebug(FC) << "TODO: server_connect";
  };
  funcs->add_net_input = KV::Application::AddServerSource;
  funcs->remove_net_input = KV::Application::RemoveServerSource;
  funcs->real_conn_list_dialog_update = KV::Application::UpdateUsers;
  funcs->close_connection_dialog = [] () {
    qCDebug(FC) << "TODO: close_connection_dialog";
  };
  funcs->add_idle_callback = KV::Application::AddIdleCallback;

  funcs->real_set_client_page = KV::Application::StateChange;
  funcs->get_current_client_page = KV::Application::CurrentState;

  funcs->set_unit_icon = [] (int idx, struct unit* punit) {
    qCDebug(FC) << "TODO: set_unit_icon" << idx << punit->id;
  };
  funcs->set_unit_icons_more_arrow = [] (bool onoff) {
    qCDebug(FC) << "TODO: set_unit_icons_more_arrow" << onoff;
  };
  funcs->real_focus_units_changed = [] () {
    qCDebug(FC) << "TODO: real_focus_units_changed";
  };
  funcs->gui_update_font = [] (
      const char *font_name,
      const char *font_value) {
    qCDebug(FC) << "TODO: gui_update_font" << font_name << font_value;
  };

  funcs->editgui_refresh = [] () {
    // qCDebug(FC) << "TODO: editgui_refresh";
  };
  funcs->editgui_notify_object_created = [] (int tag, int id) {
    // qCDebug(FC) << "TODO: editgui_notify_object_created" << tag << id;
  };
  funcs->editgui_notify_object_changed = [] (
      int objtype, int object_id, bool removal) {
    // qCDebug(FC) << "TODO: editgui_notify_object_changed" << objtype << object_id << removal;
  };
  funcs->editgui_popup_properties = [] (const struct tile_list* /*tiles*/, int objtype) {
    // qCDebug(FC) << "TODO: editgui_popup_properties" << objtype;
  };
  funcs->editgui_tileset_changed = [] () {
    // qCDebug(FC) << "TODO: editgui_tileset_changed";
  };
  funcs->editgui_popdown_all = [] () {
    // qCDebug(FC) << "TODO: editgui_popdown_all";
  };

  funcs->popup_combat_info = [] (
      int attacker_unit_id, int defender_unit_id,
      int attacker_hp, int defender_hp,
      bool make_att_veteran, bool make_def_veteran) {
    qCDebug(FC) << "TODO: popup_combat_info"
                << attacker_unit_id
                << defender_unit_id
                << attacker_hp
                << defender_hp
                << make_att_veteran
                << make_def_veteran;
  };
  funcs->update_timeout_label = [] () {
    // qCDebug(FC) << "update_timeout_label";
    KV::Application::UpdateTurnTimeout();
  };
  funcs->real_city_dialog_popup = [] (struct city* pcity) {
    qCDebug(FC) << "real_city_dialog_popup";
    KV::Application::RefreshCityDialog(pcity, true);
  };
  funcs->real_city_dialog_refresh = [] (struct city* pcity) {
    qCDebug(FC) << "real_city_dialog_refresh";
    KV::Application::RefreshCityDialog(pcity, false);
  };
  funcs->popdown_city_dialog = [] (struct city* pcity) {
    qCDebug(FC) << "popdown_city_dialog";
    KV::Application::PopdownCityDialog(pcity);
  };
  funcs->popdown_all_city_dialogs = [] () {
    qCDebug(FC) << "popdown_all_city_dialogs";
    KV::Application::PopdownCityDialog(nullptr);
  };
  funcs->handmade_scenario_warning = [] () {
    qCDebug(FC) << "TODO: handmade_scenario_warning";
    return false;
  };
  funcs->refresh_unit_city_dialogs = [] (struct unit *punit) {
    real_city_dialog_refresh(game_city_by_number(punit->homecity));
    real_city_dialog_refresh(tile_city(punit->tile));
  };
  funcs->city_dialog_is_open = [] (struct city* /*pcity*/) {
    qCDebug(FC) << "TODO: city_dialog_is_open";
    return false;
  };;

  funcs->request_transport = [] (struct unit* /*pcargo*/, struct tile* /*ptile*/) {
    qCDebug(FC) << "TODO: request_transport";
    return false;
  };

  funcs->gui_load_theme = KV::ThemesManager::LoadTheme;
  funcs->gui_clear_theme = KV::ThemesManager::ClearTheme;
  funcs->get_gui_specific_themes_directories = KV::ThemesManager::GetPaths;
  funcs->get_useable_themes_in_directory = KV::ThemesManager::GetThemes;
}


int main(int argc, char **argv)
{
  setup_gui_funcs();

//  QApplication app(argc, argv);
//  app.setOrganizationName("Kvanttiapina");
//  app.setApplicationName("freeciv-kde");

  return client_main(argc, argv);
}
