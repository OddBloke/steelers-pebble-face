#include <pebble.h>
#include "main.h"

// Matching values to those in 'Settings'
typedef enum {
  AppKeyAnimations = 0
} AppKey;

const int DROPPING_ANGLE = 16384;  // Rotate by 270 degrees
const int UPWARDS_ANGLE = 24576;  // Rotate by 135 degrees

static Window *s_main_window;
static TextLayer *s_time_layer;
#if defined(PBL_ROUND)
  static TextLayer *s_date_layer;
  static TextLayer *s_day_of_week_layer;
#endif

static GBitmap *s_logo_bitmap;
static BitmapLayer *s_logo_bitmap_layer;

static GBitmap *s_football_bitmap;
static RotBitmapLayer *s_football_bitmap_layer;
static GRect s_football_end_position;

static bool show_animations;


static void update_time_and_date() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  #if defined(PBL_ROUND)
    static char s_date_buffer[3];
    strftime(s_date_buffer, sizeof(s_date_buffer), "%d", tick_time);
    text_layer_set_text(s_date_layer, s_date_buffer);
  
    static char s_day_of_week_buffer[10];
    strftime(s_day_of_week_buffer, sizeof(s_day_of_week_buffer), "%A", tick_time);
    text_layer_set_text(s_day_of_week_layer, s_day_of_week_buffer);
  #endif
}

static void set_up_logo(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  window_set_background_color(window, PBL_IF_ROUND_ELSE(GColorFromRGBA(155, 171, 182, 1),
                                                        GColorWhite));
  
  s_logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO_PNG);  
  s_logo_bitmap_layer = bitmap_layer_create(GRect(0, 0, 180, 180));
  bitmap_layer_set_compositing_mode(s_logo_bitmap_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_logo_bitmap_layer, GAlignCenter);
  bitmap_layer_set_bitmap(s_logo_bitmap_layer, s_logo_bitmap);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(s_logo_bitmap_layer));
}

static void put_football_in_starting_position() {
  rot_bitmap_layer_set_angle(s_football_bitmap_layer, UPWARDS_ANGLE);
  GRect r = layer_get_frame((Layer *) s_football_bitmap_layer);
  #if defined(PBL_ROUND)
    r.origin.x = -15;
    r.origin.y = 145;
  #else
    r.origin.x = -35;
    r.origin.y = 155;
  #endif
  layer_set_frame((Layer *) s_football_bitmap_layer, r);
}

static void set_up_football(Layer *window_layer) {
  s_football_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOOTBALL_PNG);
  s_football_bitmap_layer = rot_bitmap_layer_create(s_football_bitmap);
  rot_bitmap_set_compositing_mode(s_football_bitmap_layer, GCompOpSet);
  
  put_football_in_starting_position();
  
  GRect bounds = layer_get_bounds(window_layer);
  GRect r = layer_get_frame((Layer *) s_football_bitmap_layer);
  r.origin.x = (bounds.size.w / 2) - 25;
  r.origin.y = 60;
  s_football_end_position = r;
    
  layer_add_child(window_layer, (Layer *) s_football_bitmap_layer);
}

static void set_up_clock(Layer *window_layer) {
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(2, PBL_IF_ROUND_ELSE(23, 17), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

#if defined(PBL_ROUND)
static void set_up_date(Layer *window_layer) {
  s_date_layer = text_layer_create(GRect(113, 77, 30, 30));
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorMelon);
  text_layer_set_text(s_date_layer, "00");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void set_up_day_of_week(Layer *window_layer) {
  s_day_of_week_layer = text_layer_create(GRect(22, 95, 100, 30));
  
  text_layer_set_background_color(s_day_of_week_layer, GColorClear);
  text_layer_set_text_color(s_day_of_week_layer, GColorBlack);
  text_layer_set_text(s_day_of_week_layer, "Wednesday");
  text_layer_set_font(s_day_of_week_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_day_of_week_layer, GTextAlignmentLeft);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_day_of_week_layer));
}
#endif

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  
  set_up_logo(window);
  set_up_football(window_layer);
  set_up_clock(window_layer);
  #if defined(PBL_ROUND)
    set_up_date(window_layer);
    set_up_day_of_week(window_layer);
  #endif
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  #if defined(PBL_ROUND)
    text_layer_destroy(s_date_layer);
    text_layer_destroy(s_day_of_week_layer);
  #endif
  
  gbitmap_destroy(s_logo_bitmap);
  bitmap_layer_destroy(s_logo_bitmap_layer);
  
  gbitmap_destroy(s_football_bitmap);
  rot_bitmap_layer_destroy(s_football_bitmap_layer);
}

static void football_reached_ground_handler(Animation *animation, bool finished, void *context) {
  put_football_in_starting_position();
}

static void animate_football_to_ground() {
  GRect r = layer_get_frame((Layer *) s_football_bitmap_layer);
  r.origin.x += 5;
  
  rot_bitmap_layer_set_angle(s_football_bitmap_layer, DROPPING_ANGLE);
  layer_set_frame((Layer *) s_football_bitmap_layer, r);
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(
    (Layer *) s_football_bitmap_layer, &r, &GRect(r.origin.x, 180, r.size.w, r.size.h));
  
  // Get the Animation
  Animation *anim = property_animation_get_animation(prop_anim);
  
  // Configure the Animation's curve, delay, and duration
  animation_set_curve(anim, AnimationCurveLinear);
  animation_set_delay(anim, 0);
  animation_set_duration(anim, 750);
  
  animation_set_handlers(anim, (AnimationHandlers) {
    .stopped = football_reached_ground_handler
  }, NULL);

  // Play the animation
  animation_schedule(anim);
}

static void football_reached_clock_handler(Animation *animation, bool finished, void *context) {
  update_time_and_date();
  animate_football_to_ground();
}

static void animate_football_to_clock() {
  GRect r = layer_get_frame((Layer *) s_football_bitmap_layer);
  PropertyAnimation *prop_anim = property_animation_create_layer_frame(
    (Layer *) s_football_bitmap_layer,
    &r,
    &s_football_end_position);
  // Get the Animation
  Animation *anim = property_animation_get_animation(prop_anim);
  
  // Configure the Animation's curve, delay, and duration
  animation_set_curve(anim, AnimationCurveEaseIn);
  animation_set_delay(anim, 0);
  animation_set_duration(anim, 750);
  
  animation_set_handlers(anim, (AnimationHandlers) {
    .stopped = football_reached_clock_handler
  }, NULL);
  
  // Play the animation
  animation_schedule(anim);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (show_animations) {
    animate_football_to_clock();
  } else {
    update_time_and_date();
  }
}

static void persist_configuration() {
  persist_write_bool(AppKeyAnimations, show_animations);
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {
  Tuple *animations_t = dict_find(iter, AppKeyAnimations);
  if(animations_t) {
    show_animations = animations_t->value->int32 == 1;
  }
  persist_configuration();
};

static void fetch_and_set_config_options() {
  if (persist_exists(AppKeyAnimations)) {
    show_animations = persist_read_bool(AppKeyAnimations);
  } else {
    show_animations = true;
  }
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
  update_time_and_date();
  
  fetch_and_set_config_options();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Largest expected inbox and outbox message sizes
  const uint32_t inbox_size = 54;
  const uint32_t outbox_size = 0;

  // Open AppMessage
  app_message_open(inbox_size, outbox_size);
  // Register to be notified about inbox received events
  app_message_register_inbox_received(inbox_received_callback);
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