#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "benchmark/lv_demo_benchmark.h"
#include "lv_demo_widgets/lv_demo_widgets.h"
#include "lv_input/lv_demo_keypad_encoder.h"

#define LVGL_TICK_PERIOD 5

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];


#if USE_LV_LOG != 0
/* Serial debugging */
static void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * fn_name, const char * dsc)
{

  Serial.printf("\n%s@%d->%s", file, line, dsc);
  delay(20);
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{

  lv_tick_inc(LVGL_TICK_PERIOD);
}

/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
  static int32_t last_diff = 0;
  int32_t diff = 0; /* Dummy - no movement */
  int btn_state = LV_INDEV_STATE_REL; /* Dummy - no press */

  data->enc_diff = diff - last_diff;;
  data->state = btn_state;

  last_diff = diff;

  return false;
}

//
//
//
uint32_t wioGet5WaySwitch()
{
uint32_t k=0;

  if (digitalRead(WIO_5S_UP) == LOW) {
    k=4; Serial.println("5 Way Up");
    }
  else if (digitalRead(WIO_5S_DOWN) == LOW) {
    k=5; Serial.println("5 Way Down");
    }
  else if (digitalRead(WIO_5S_LEFT) == LOW) {
    k=2; Serial.println("5 Way Left");
    }
  else if (digitalRead(WIO_5S_RIGHT) == LOW) {
    k=3; Serial.println("5 Way Right");
    }
  else if (digitalRead(WIO_5S_PRESS) == LOW) {
    k=1; Serial.println("5 Way Press");
    }
  return k;  
}
//
//
//
static int32_t encoder_diff;
static lv_indev_state_t encoder_state;
static bool encoder_wio(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;
    uint32_t act_key = wioGet5WaySwitch();
    if(act_key != 0) {
        switch(act_key) {
        case 1:
            act_key = LV_KEY_ENTER;
            encoder_state = LV_INDEV_STATE_PR;	
            break;
        case 2:
            act_key = LV_KEY_LEFT;
            encoder_diff = -1;
            encoder_state = LV_INDEV_STATE_REL;
            break;
        case 3:
            act_key = LV_KEY_RIGHT;
            encoder_state = LV_INDEV_STATE_REL;
            encoder_diff = 1;
            break;
        }
        last_key = act_key;
    }
    else {
        encoder_diff = 0;
        encoder_state = LV_INDEV_STATE_REL;
    }
    data->key = last_key;
    data->enc_diff = encoder_diff;
    data->state = encoder_state;
    /*Return `false` because we are not buffering and no more data to read*/
    return false;
}

static lv_group_t* g; //An Object Group
static lv_indev_t* encoder_indev; //The input device
void setup() {

  Serial.begin(115200); /* prepare for possible serial debug */

  //setup 5-way switch--------------
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  //setup buttons------------------
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);
  

  lv_init();

#if USE_LV_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin(); /* TFT init */
  tft.setRotation(1); /* Landscape orientation */

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the touch pad*/
  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_ENCODER;
  //indev_drv.read_cb = read_encoder;
  indev_drv.read_cb = encoder_wio;
  encoder_indev = lv_indev_drv_register(&indev_drv);

  //Create Group for encoder
  //g = lv_group_create();
  //lv_indev_set_group(encoder_indev, g);
  
  /*stress test*/
  //lv_demo_benchmark();
  //lv_demo_widgets();
  lv_demo_keypad_encoder();
}


void loop() {
  lv_tick_handler();
  lv_task_handler(); /* let the GUI do its work */
  delay(5);
}