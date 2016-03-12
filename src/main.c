#include <pebble.h>
#include "main.h"

static Window *s_main_window;
static TextLayer *s_time_layer;

static GBitmap *s_logo_bitmap;
static BitmapLayer *s_logo_bitmap_layer;

static GFont s_time_font;


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void set_up_logo(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  window_set_background_color(window, GColorFromRGBA(155, 171, 182, 1));
  
  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO_PNG);  
  s_logo_bitmap_layer = bitmap_layer_create(GRect(0, 0, 180, 180));
  bitmap_layer_set_compositing_mode(s_logo_bitmap_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_logo_bitmap_layer, GAlignCenter);
  bitmap_layer_set_bitmap(s_logo_bitmap_layer, s_logo_bitmap);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_bitmap_layer));
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  set_up_logo(window);
  
  // Create GFont
  s_time_font = fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(18, 12), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_time_font);
  
  gbitmap_destroy(s_logo_bitmap);
  bitmap_layer_destroy(s_logo_bitmap_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}