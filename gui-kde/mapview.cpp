extern "C" {
#include "mapview_g.h"
}

void update_info_label() {
}

void update_unit_info_label(struct unit_list *punitlist) {
}

void update_mouse_cursor(enum cursor_type new_cursor_type) {
}

void update_turn_done_button(bool do_restore) {
}

void update_city_descriptions() {
}

void set_indicator_icons(struct sprite *bulb, struct sprite *sol, struct sprite *flake, struct sprite *gov) {
}

void overview_size_changed() {
}

void update_overview_scroll_window_pos(int x, int y) {
}

void get_overview_area_dimensions(int *width, int *height) {
}

struct canvas * get_overview_window() {
}

void flush_mapcanvas(int canvas_x, int canvas_y, int pixel_width, int pixel_height) {
}

void dirty_rect(int canvas_x, int canvas_y, int pixel_width, int pixel_height) {
}

void dirty_all() {
}

void flush_dirty() {
}

void gui_flush() {
}

void update_map_canvas_scrollbars() {
}

void update_map_canvas_scrollbars_size() {
}

void put_cross_overlay_tile(struct tile *ptile) {
}

void draw_selection_rectangle(int canvas_x, int canvas_y, int w, int h) {
}

void tileset_changed() {
}

