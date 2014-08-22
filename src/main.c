#include "pebble.h"

static Window *window;
static GRect bounds;
static Layer *path_layer;

// This defines graphics path information to be loaded as a path later
static const GPathInfo HOUSE_PATH_POINTS = {
  // This is the amount of points
  11,
  // A path can be concave, but it should not twist on itself
  // The points should be defined in clockwise order due to the rendering
  // implementation. Counter-clockwise will work in older firmwares, but
  // it is not officially supported
  (GPoint []) {
    {-40, 0},
    {0, -40},
    {40, 0},
    {28, 0},
    {28, 40},
    {10, 40},
    {10, 16},
    {-10, 16},
    {-10, 40},
    {-28, 40},
    {-28, 0}
  }
};

// This is an example of a path that looks like a compound path
// If you rotate it however, you will see it is a single shape
static const GPathInfo INFINITY_RECT_PATH_POINTS = {
  16,
  (GPoint []) {
    {-50, 0},
    {-50, -60},
    {10, -60},
    {10, -20},
    {-10, -20},
    {-10, -40},
    {-30, -40},
    {-30, -20},
    {50, -20},
    {50, 40},
    {-10, 40},
    {-10, 0},
    {10, 0},
    {10, 20},
    {30, 20},
    {30, 0}
  }
};
/*
    -15  -5   5    15
-25 X0Y0 X1Y0 X2Y0 X3Y0   -25
-15 X0Y1 X1Y1 X2Y1 X3Y1   -15
-5  X0Y2 X1Y2 X2Y2 X3Y2   -5
5   X0Y3 X1Y3 X2Y3 X3Y3   5
15  X0Y4 X1Y4 X2Y4 X3Y4   15
25  X0Y5 X1Y5 X2Y5 X3Y5   25
    -15  -5   5    15
*/
#define X0 -12
#define X1 -4
#define X2 4
#define X3 12
#define Y0 -20
#define Y1 -12
#define Y2 -4
#define Y3 4
#define Y4 12
#define Y5 20

#define REFRESH_TIME  250
  
static const GPathInfo PATH_POINTS_ZERO = {
  11,
  (GPoint []) {
    {X0, Y0},
    {X3, Y0},
    {X3, Y5},
    {X0, Y5},
    {X0, Y1},
    {X1, Y1}, 
    {X1, Y4},
    {X2, Y4},
    {X2, Y1},
    {X0, Y1},
    {X0, Y0}
  }
};

static const GPathInfo PATH_POINTS_ONE = {
  4,
  (GPoint []) {
    {X1, Y0},
    {X2, Y0},
    {X2, Y5},
    {X1, Y5}
  }
};


static const GPathInfo PATH_POINTS_FONTS[] = {
  // 0
  {
    11,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y5},
      {X0, Y5},
      {X0, Y1},
      {X1, Y1}, 
      {X1, Y4},
      {X2, Y4},
      {X2, Y1},
      {X0, Y1},
      {X0, Y0}
    }
  },
  // 1
  {
    4,
    (GPoint []) {
      {X1, Y0},
      {X2, Y0},
      {X2, Y5},
      {X1, Y5}
    }
  },
  // 2
  {
    12,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y3},
      {X1, Y3},
      {X1, Y4},
      {X3, Y4},
      {X3, Y5},
      {X0, Y5},
      {X0, Y2},
      {X2, Y2},
      {X2, Y1},
      {X0, Y1}
    }
  },
  // 3
  {
    12,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y5},
      {X0, Y5},
      {X0, Y4},
      {X2, Y4},
      {X2, Y3},
      {X1, Y3},
      {X1, Y2},
      {X2, Y2},
      {X2, Y1},
      {X0, Y1}
    }
  },
  // 4
  {
    10,
    (GPoint []) {
      {X0, Y0},
      {X1, Y0},
      {X1, Y2},
      {X2, Y2},
      {X2, Y0},
      {X3, Y0},
      {X3, Y5},
      {X2, Y5},
      {X2, Y3},
      {X0, Y3}
    }
  },
  // 5
  {
    12,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y1},
      {X1, Y1},
      {X1, Y2},
      {X3, Y2},
      {X3, Y5},
      {X0, Y5},
      {X0, Y4},
      {X2, Y4},
      {X2, Y3},
      {X0, Y3}
    }
  },
  // 6
  {
    12,
    (GPoint []) {
      {X1, Y2},
      {X3, Y2},
      {X3, Y5},
      {X0, Y5},
      {X0, Y0},
      {X3, Y0},
      {X3, Y1},
      {X1, Y1},
      {X1, Y4},
      {X2, Y4},
      {X2, Y3},
      {X1, Y3}
    }
  },
  // 7
  {
    6,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y5},
      {X2, Y5},
      {X2, Y1},
      {X0, Y1},
    }
  },
  // 8
  {
    18,
    (GPoint []) {
      {X3, Y3},
      {X3, Y0},
      {X0, Y0},
      {X0, Y2},
      {X1, Y3},
      {X2, Y3},
      {X2, Y4},
      {X1, Y4},
      {X1, Y3},
      {X0, Y2},
      {X0, Y5},
      {X3, Y5},
      {X3, Y3},
      {X2, Y2},
      {X1, Y2},
      {X1, Y1},
      {X2, Y1},
      {X2, Y2}
    }
  },
/*
  {
    16,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y5},
      {X0, Y5},
      {X0, Y3},
      {X1, Y3},
      {X1, Y4},
      {X2, Y4},
      {X2, Y3},
      {X0, Y3},
      {X0, Y2},
      {X2, Y2},
      {X2, Y1},
      {X1, Y1},
      {X1, Y2},
      {X0, Y2}
    }
  },
*/
  // 9
  {
    12,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y5},
      {X0, Y5},
      {X0, Y4},
      {X2, Y4},
      {X2, Y1},
      {X1, Y1},
      {X1, Y2},
      {X2, Y2},
      {X2, Y3},
      {X0, Y3}
    }
  },
/*
  {
    14,
    (GPoint []) {
      {X0, Y0},
      {X3, Y0},
      {X3, Y5},
      {X0, Y5},
      {X0, Y4},
      {X2, Y4},
      {X2, Y3},
      {X0, Y3},
      {X0, Y1},
      {X1, Y1},
      {X1, Y2},
      {X2, Y2},
      {X2, Y1},
      {X0, Y1}
    }
  },
*/
  // :
  {
    8,
    (GPoint []) {
      {X1, Y1},
      {X2, Y1},
      {X2, Y2},
      {X1, Y2},
      {X1, Y3},
      {X2, Y3},
      {X2, Y4},
      {X1, Y4}
    }
  }
  
};  
  
static int g_nHour;
static int g_nMinute;

static GPath *house_path;

static GPath *infinity_path;

static GPath *path_zero;
static GPath *path_one;
static GPath *path_two;
static GPath *path_three;
static GPath *path_four;
static GPath *path_five;
static GPath *path_six;
static GPath *path_seven;
static GPath *path_eight;
static GPath *path_nine;
static GPath *path_colon;

#define NUM_GRAPHIC_PATHS 13

static GPath *graphic_paths[NUM_GRAPHIC_PATHS];

static GPath *current_path = NULL;

static GPath *paths[5] = {0, 0, 0, 0, 0};
static GPoint aapoints[5][20];

static int current_path_index = 0;

static int path_angle = 0;

static bool outline_mode = false;

static char time_string[40];
static TextLayer *text_layer_time;

static AppTimer *g_apptimer;

static void SetDirty(void *data) {
  layer_mark_dirty(path_layer);
  g_apptimer = app_timer_register(REFRESH_TIME, SetDirty, NULL);
}

GPath * CreateDigit(int pos, const GPathInfo *ppathinfo) {
  int posPixel = (pos - 2) * 28;
  GPathInfo pathinfo;
  pathinfo.num_points = ppathinfo->num_points;
  pathinfo.points = aapoints[pos];
  int i;
  for (i=0; i<(int)pathinfo.num_points; i++) {
    pathinfo.points[i].x = ppathinfo->points[i].x + posPixel;
    pathinfo.points[i].y = ppathinfo->points[i].y;
  }
  GPath *path = gpath_create(&pathinfo);
  return path;
}



// This is the layer update callback which is called on render updates
static void path_layer_update_callback(Layer *me, GContext *ctx) {
  (void)me;

  AccelData accel = (AccelData){.x=0, .y=0, .z=0};
  accel_service_peek(&accel);
  
  int path_angle_new;
  int diff_angle;
  if (-2010 < accel.z && accel.z < 2010) {
    path_angle_new = atan2_lookup(accel.y, accel.x) * 360 / 0x10000;
    path_angle_new = (270 - path_angle_new) % 360;
    
    diff_angle = path_angle_new - path_angle;
    if (-180 < diff_angle && diff_angle < 180) {
      path_angle = (path_angle + path_angle_new) / 2 % 360;
    } else {
      path_angle = (path_angle + path_angle_new + 360) / 2 %360;
    }
  } else {
    path_angle = 0;
  }
  
  // You can rotate the path before rendering
  //gpath_rotate_to(current_path, (TRIG_MAX_ANGLE / 360) * path_angle);

  // There are two ways you can draw a GPath: outline or filled
  // In this example, only one or the other is drawn, but you can draw
  // multiple instances of the same path filled or outline.
  if (outline_mode) {
    // draw outline uses the stroke color
    graphics_context_set_stroke_color(ctx, GColorWhite);
    gpath_draw_outline(ctx, current_path);
  } else {
    // draw filled uses the fill color
    //graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);
    //gpath_draw_filled(ctx, current_path);

    int codes[5];
    codes[0] = g_nHour / 10;
    codes[1] = g_nHour % 10;
    codes[2] = 10; // :
    codes[3] = g_nMinute / 10;
    codes[4] = g_nMinute % 10;
/*
    //if (paths[0]!=0)
    //  gpath_destroy(paths[0]);
    paths[0] = CreateDigit(0, &PATH_POINTS_FONTS[6]);
    //if (paths[1]!=0)
    //  gpath_destroy(paths[1]);
    paths[1] = CreateDigit(1, &PATH_POINTS_FONTS[7]);
    //if (paths[2]!=0)
    //  gpath_destroy(paths[2]);
    paths[2] = CreateDigit(2, &PATH_POINTS_FONTS[8]);
    //if (paths[3]!=0)
    //  gpath_destroy(paths[3]);
    paths[3] = CreateDigit(3, &PATH_POINTS_FONTS[9]);
    //if (paths[4]!=0)
    //  gpath_destroy(paths[4]);
    paths[4] = CreateDigit(4, &PATH_POINTS_FONTS[10]);
*/
    int i;
    for (i=0; i<5; i++) {
      /*
      if (i==2)
        graphics_context_set_stroke_color(ctx, GColorBlack);
      else
        graphics_context_set_stroke_color(ctx, GColorWhite);
      */
      paths[i] = CreateDigit(i, &PATH_POINTS_FONTS[codes[i]]);
      gpath_move_to(paths[i], GPoint(bounds.size.w/2, bounds.size.h/2));
      gpath_rotate_to(paths[i], (TRIG_MAX_ANGLE / 360) * path_angle);
      gpath_draw_filled(ctx, paths[i]);
      //gpath_draw_outline(ctx, paths[i]);
      gpath_destroy(paths[i]);
    }
/*
    gpath_move_to(graphic_paths[0], GPoint(bounds.size.w/2, bounds.size.h/2));
    gpath_rotate_to(graphic_paths[0], (TRIG_MAX_ANGLE / 360) * path_angle);
    gpath_draw_filled(ctx, graphic_paths[0]);
*/
  }
}

static int path_angle_add(int angle) {
  return path_angle = (path_angle + angle) % 360;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Rotate the path counter-clockwise
  path_angle_add(-10);
  layer_mark_dirty(path_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Rotate the path clockwise
  path_angle_add(10);
  layer_mark_dirty(path_layer);
}

static void select_raw_down_handler(ClickRecognizerRef recognizer, void *context) {
  // Show the outline of the path when select is held down
  outline_mode = true;
  layer_mark_dirty(path_layer);
}
static void select_raw_up_handler(ClickRecognizerRef recognizer, void *context) {
  // Show the path filled
  outline_mode = false;
  // Cycle to the next path
  current_path_index = (current_path_index+1) % NUM_GRAPHIC_PATHS;
  current_path = graphic_paths[current_path_index];
  layer_mark_dirty(path_layer);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  AccelData accel = (AccelData){.x=0, .y=0, .z=0};
  accel_service_peek(&accel);
//  if (-950 < accel.z && accel.z < 950) {
    /*
    path_angle = atan2_lookup(accel.y, accel.x) * 360 / 0x10000;
    path_angle = (270 - path_angle) % 360;
    */
    //path_angle = (atan2_lookup(-accel.y, -accel.x) * 360 / 0x10000 - 90) % 360;
//  } else {
//    path_angle = 0;
//  }
  
  g_nHour = tick_time->tm_hour;
  g_nMinute = tick_time->tm_min;
  //layer_mark_dirty(path_layer);
/*  
  snprintf(time_string, sizeof(time_string), "%02d:%02d:%02d %d",
           tick_time->tm_hour,
           tick_time->tm_min,
           tick_time->tm_sec,
           path_angle);
*/
  snprintf(time_string, sizeof(time_string), "%d, %d, %d : %d",
          accel.x, accel.y, accel.z, (int)atan2_lookup(accel.y, accel.x));
  text_layer_set_text(text_layer_time, time_string);
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_raw_down_handler, select_raw_up_handler, NULL);
}

static void init() {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  accel_data_service_subscribe(0, NULL);
  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  
  //window_set_click_config_provider(window, config_provider);

  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_frame(window_layer);

  path_layer = layer_create(bounds);
  layer_set_update_proc(path_layer, path_layer_update_callback);
  layer_add_child(window_layer, path_layer);

  // Pass the corresponding GPathInfo to initialize a GPath
  house_path = gpath_create(&HOUSE_PATH_POINTS);
  infinity_path = gpath_create(&INFINITY_RECT_PATH_POINTS);
  path_zero = gpath_create(&PATH_POINTS_ZERO);
  path_one = gpath_create(&PATH_POINTS_ONE);
  
  // This demo allows you to cycle paths in an array
  // Try adding more GPaths to cycle through
  // You'll need to define another GPathInfo
  // Remember to update NUM_GRAPHIC_PATHS accordingly
  //graphic_paths[0] = house_path;
  //graphic_paths[1] = infinity_path;
  graphic_paths[0] = path_zero;
  graphic_paths[1] = path_one;
  graphic_paths[2] = path_one;
  graphic_paths[3] = path_one;
  graphic_paths[4] = path_one;
  graphic_paths[5] = path_one;
  graphic_paths[6] = path_one;
  graphic_paths[7] = path_one;
  graphic_paths[8] = path_one;
  graphic_paths[9] = path_one;
  graphic_paths[10] = path_one;
  graphic_paths[11] = path_one;
  graphic_paths[12] = path_one;
  
  current_path = graphic_paths[2];

  // Move all paths to the center of the screen
  for (int i = 0; i < NUM_GRAPHIC_PATHS; i++) {
    gpath_move_to(graphic_paths[i], GPoint(bounds.size.w/2, bounds.size.h/2));
  }
  text_layer_time = text_layer_create((GRect){.origin={0, 0}, .size={160, 20}});
  layer_add_child(window_layer, text_layer_get_layer(text_layer_time));
  g_apptimer = app_timer_register(REFRESH_TIME, SetDirty, NULL);
}

static void deinit() {
  app_timer_cancel(g_apptimer);
  text_layer_destroy(text_layer_time);
  gpath_destroy(house_path);
  gpath_destroy(infinity_path);

  layer_destroy(path_layer);
  tick_timer_service_unsubscribe();
  accel_data_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
