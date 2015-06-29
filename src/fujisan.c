#include <pebble.h>
#define KEY_SUNRISE_TIME 0
#define KEY_SUNSET_TIME 1

static Window *window;
static TextLayer *s_watch_layer;
static Layer *s_circle_window_layer;
static Layer *s_fuji_layer;
static Layer *s_fuji_background_layer;
static BitmapLayer *s_wall_bitmap_layer;
static GFont s_beaver_font;

static GBitmap *s_wall_bitmap;
static GPath *s_fuji_path_ptr = NULL;
static GPath *s_fuji_snow_path_ptr = NULL;
static GPath *s_fuji_background_path_ptr = NULL;
static long s_sunrise = 0;
static long s_sunset = 0;

static const GPathInfo FUJI_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint []) {
    {4, 124},
    {70, 50},
    {100, 50},
    {140, 124}
  }
};
static const GPathInfo FUJI_SNOW_PATH_INFO = {
  .num_points = 9,
  .points = (GPoint []) {
    { 70, 50},
    {100, 50},
    {108, 65},
    {100, 72},
    { 93, 65},
    { 84, 72},
    { 76, 65},
    { 64, 72},
    { 57, 65}
  }
};
static const GPathInfo FUJI_BACKGROUND_PATH_INFO = {
  .num_points = 4,
  .points = (GPoint[]) {{2, 2}, {142, 2}, {142, 166}, {2, 166}}
};


static void update_time() {
  time_t tmp = time(NULL);
  struct tm *tick_time = localtime(&tmp);

  static char buffer[] = "00:00";

  if (clock_is_24h_style() == true) {
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // for screenshot
  //text_layer_set_text(s_watch_layer, "06:10");

  text_layer_set_text(s_watch_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void setup_paths(void) {
  s_fuji_path_ptr = gpath_create(&FUJI_PATH_INFO);
  s_fuji_snow_path_ptr = gpath_create(&FUJI_SNOW_PATH_INFO);
  s_fuji_background_path_ptr = gpath_create(&FUJI_BACKGROUND_PATH_INFO);
}

static void setup_bitmaps(void) {
  s_wall_bitmap = gbitmap_create_with_resource(RESOURCE_ID_WALL_COLOR_IMAGE);
}

static void fuji_background_layer_update_proc(Layer *layer, GContext *ctx) {
  time_t current_time = time(NULL);
  struct tm *day_start = localtime(&current_time);
  struct tm *day_end = localtime(&current_time);

  day_start->tm_hour = 0;
  day_start->tm_min = 0;
  day_start->tm_sec = 0;
  long day_start_time = mktime(day_start);

  day_end->tm_hour = 23;
  day_end->tm_min = 59;
  day_end->tm_sec = 59;
  long day_end_time = mktime(day_end);

  GColor fill_color = GColorWhite;

  if (s_sunrise != 0) {
    if (day_start_time <= current_time && current_time < (s_sunrise - 3600)) {
      fill_color = GColorDarkGray;
    }
    if ((s_sunrise - 3600) <= current_time && current_time <= s_sunrise) {
      fill_color = GColorPastelYellow;
    }
  }
  if (s_sunset != 0) {
    if (s_sunset <= current_time && current_time <= (s_sunset + 3600)) {
      fill_color = GColorSunsetOrange;
    }
    if ((s_sunset + 3600) < current_time && current_time <= day_end_time) {
      fill_color = GColorDarkGray;
    }
  }

  graphics_context_set_fill_color(ctx, fill_color);
  gpath_draw_filled(ctx, s_fuji_background_path_ptr);
}

static void fuji_layer_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_stroke_width(ctx, 3);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  gpath_draw_filled(ctx, s_fuji_path_ptr);

  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, s_fuji_snow_path_ptr);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  gpath_draw_outline(ctx, s_fuji_path_ptr);
  gpath_draw_outline(ctx, s_fuji_snow_path_ptr);
}

static void circle_window_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint circle_center = GPoint((bounds.size.w / 2), (bounds.size.w / 2)); // same value

  graphics_context_set_stroke_width(ctx, 13);
  graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(GColorDarkGreen, GColorBlack));
  graphics_draw_circle(ctx, circle_center, 54);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_fuji_background_layer = layer_create(bounds);
  layer_set_update_proc(s_fuji_background_layer, fuji_background_layer_update_proc);
  layer_add_child(window_layer, s_fuji_background_layer);

  s_fuji_layer = layer_create(bounds);
  layer_set_update_proc(s_fuji_layer, fuji_layer_update_proc);
  layer_add_child(window_layer, s_fuji_layer);

  s_wall_bitmap_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_wall_bitmap_layer, s_wall_bitmap);
  bitmap_layer_set_compositing_mode(s_wall_bitmap_layer, GCompOpSet);
  layer_add_child(window_layer,  bitmap_layer_get_layer(s_wall_bitmap_layer));

  s_circle_window_layer = layer_create(bounds);
  layer_set_update_proc(s_circle_window_layer, circle_window_layer_update_proc);
  layer_add_child(window_layer, s_circle_window_layer);

  s_watch_layer = text_layer_create((GRect) { .origin = { 0, 130 }, .size = { bounds.size.w, 30 } });
  s_beaver_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BEAVER_30));
  text_layer_set_font(s_watch_layer, s_beaver_font);
  text_layer_set_text(s_watch_layer, "00:00");
  text_layer_set_text_alignment(s_watch_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_watch_layer, GColorClear);

  layer_add_child(window_layer, text_layer_get_layer(s_watch_layer));

  update_time();
}

static void window_unload(Window *window) {
  gpath_destroy(s_fuji_path_ptr);
  gpath_destroy(s_fuji_snow_path_ptr);
  gpath_destroy(s_fuji_background_path_ptr);
  gbitmap_destroy(s_wall_bitmap);

  fonts_unload_custom_font(s_beaver_font);
  text_layer_destroy(s_watch_layer);

  layer_destroy(s_circle_window_layer);
  layer_destroy(s_fuji_layer);
  layer_destroy(s_fuji_background_layer);
  bitmap_layer_destroy(s_wall_bitmap_layer);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);

  while (t != NULL) {
    switch (t->key) {
    case KEY_SUNRISE_TIME:
      s_sunrise = (long) t->value->int32;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Sunrise at %ld", s_sunrise);
      break;
    case KEY_SUNSET_TIME:
      s_sunset = (long) t->value->int32;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Sunset at %ld", s_sunset);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    t = dict_read_next(iterator);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init(void) {
  setup_paths();
  setup_bitmaps();

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // AppMessage Callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
