extern "C" {
#include "mapview_g.h"
}
#include "logging.h"
#include "canvas.h"
#include "application.h"
#include "mainwindow.h"

void update_info_label() {
  // qCDebug(FC) << "update_info_label";
  KV::Application::UpdateGameInfo();
}

void update_unit_info_label(struct unit_list *punitlist) {
  // qCDebug(FC) << "update_unit_info_label";
  KV::Application::UpdateUnitInfo(punitlist);
}

void update_mouse_cursor(cursor_type ct) {
  // qCDebug(FC) << "update_mouse_cursor";
  KV::Application::UpdateCursor(ct);
}

void update_turn_done_button(bool do_restore) {
  // qCDebug(FC) << "update_turn_done_button";
  KV::Application::UpdateTurnDone(do_restore);
}

void update_city_descriptions() {
  // qCDebug(FC) << "TODO: update_city_descriptions";
  update_map_canvas_visible();
}

void set_indicator_icons(struct sprite *bulb, struct sprite *sol, struct sprite *flake, struct sprite *gov) {
  qCDebug(FC) << "TODO: set_indicator_icons";
}

void overview_size_changed() {
  // qCDebug(FC) << "overview_size_changed";
}

void update_overview_scroll_window_pos(int /*x*/, int /*y*/) {
}

void get_overview_area_dimensions(int *width, int *height) {
  *width = 1;
  *height = 1;
}

canvas* get_overview_window() {
  return nullptr;
}

void flush_mapcanvas(int canvas_x, int canvas_y, int pixel_width, int pixel_height) {
  qCDebug(FC) << "TODO: flush_mapcanvas";
}

void dirty_rect(int x, int y, int w, int h) {
  // qCDebug(FC) << "dirty_rect";
  KV::Application::DirtyRect(QRect(x, y, w, h));
}

void dirty_all() {
  // qCDebug(FC) << "dirty_all";
  KV::Application::DirtyAll();
}

void flush_dirty() {
  // qCDebug(FC) << "flush_dirty";
  KV::Application::FlushDirty();
}

void gui_flush() {
  // qCDebug(FC) << "TODO: gui_flush";
  KV::Application::FlushMapview();
}

void update_map_canvas_scrollbars() {
  // qCDebug(FC) << "TODO: update_map_canvas_scrollbars";
}

void update_map_canvas_scrollbars_size() {
  // qCDebug(FC) << "TODO: update_map_canvas_scrollbars_size";
}

void put_cross_overlay_tile(struct tile *ptile) {
  qCDebug(FC) << "TODO: put_cross_overlay_tile";
}

void draw_selection_rectangle(int canvas_x, int canvas_y, int w, int h) {
  qCDebug(FC) << "TODO: draw_selection_rectangle";
}

void tileset_changed() {
  qCDebug(FC) << "TODO: tileset_changed";
}

