extern "C" {
#include "mapview_g.h"
}
#include "logging.h"
#include "canvas.h"

void update_info_label() {
  qCDebug(FC) << "update_info_label";
}

void update_unit_info_label(struct unit_list *punitlist) {
  qCDebug(FC) << "update_unit_info_label";
}

void update_mouse_cursor(enum cursor_type new_cursor_type) {
  qCDebug(FC) << "update_mouse_cursor";
}

void update_turn_done_button(bool do_restore) {
  qCDebug(FC) << "update_turn_done_button";
}

void update_city_descriptions() {
  qCDebug(FC) << "update_city_descriptions";
}

void set_indicator_icons(struct sprite *bulb, struct sprite *sol, struct sprite *flake, struct sprite *gov) {
  qCDebug(FC) << "set_indicator_icons";
}

void overview_size_changed() {
  qCDebug(FC) << "overview_size_changed";
}

void update_overview_scroll_window_pos(int x, int y) {
  qCDebug(FC) << "update_overview_scroll_window_pos";
}

void get_overview_area_dimensions(int *width, int *height) {
  qCDebug(FC) << "get_overview_area_dimensions";
}

struct canvas * get_overview_window() {
  qCDebug(FC) << "get_overview_window";
  return new struct canvas;
}

void flush_mapcanvas(int canvas_x, int canvas_y, int pixel_width, int pixel_height) {
  qCDebug(FC) << "flush_mapcanvas";
}

void dirty_rect(int canvas_x, int canvas_y, int pixel_width, int pixel_height) {
  qCDebug(FC) << "dirty_rect";
}

void dirty_all() {
  qCDebug(FC) << "dirty_all";
}

void flush_dirty() {
  qCDebug(FC) << "flush_dirty";
}

void gui_flush() {
  qCDebug(FC) << "gui_flush";
}

void update_map_canvas_scrollbars() {
  qCDebug(FC) << "update_map_canvas_scrollbars";
}

void update_map_canvas_scrollbars_size() {
  qCDebug(FC) << "update_map_canvas_scrollbars_size";
}

void put_cross_overlay_tile(struct tile *ptile) {
  qCDebug(FC) << "put_cross_overlay_tile";
}

void draw_selection_rectangle(int canvas_x, int canvas_y, int w, int h) {
  qCDebug(FC) << "draw_selection_rectangle";
}

void tileset_changed() {
  qCDebug(FC) << "tileset_changed";
}

