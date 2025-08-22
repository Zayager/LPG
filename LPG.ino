#include <Wire.h>
#include <SPI.h>
#include <PCA9557.h>
#include <dimmable_light.h>
#include <lvgl.h>
#include "ui.h"
#include "gfx_conf.h"

// Конфигурация диммера
const int syncPin = 17;
// #define outputPin 18
// #define zerocross 17
const int thyristorPin = 18;
DimmableLight light(thyristorPin);
const int period = 50;
int outVal = 0;
int outVal1 = 0;
// Конфигурация LVGL и дисплея
static lv_disp_draw_buf_t draw_buf;
static lv_color_t disp_draw_buf1[screenWidth * screenHeight / 10];
static lv_color_t disp_draw_buf2[screenWidth * screenHeight / 10];
static lv_disp_drv_t disp_drv;
PCA9557 Out;
// Функции для дисплея и сенсора (оставить без изменений)
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );

   tft.pushImageDMA(area->x1, area->y1, w, h,(lgfx::rgb565_t*)&color_p->full);

   lv_disp_flush_ready( disp );

}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
   uint16_t touchX, touchY;
   bool touched = tft.getTouch( &touchX, &touchY);
   if( !touched )
   {
      data->state = LV_INDEV_STATE_REL;
   }
   else
   {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;
   }
}

lv_obj_t *ui_LabelTimer;

void setup() {
  Serial.begin(9600);

   // Инициализация диммера
  DimmableLight::setSyncPin(syncPin);
  DimmableLight::begin();
    
      //GPIO init
#if defined (CrowPanel_70)
  pinMode(38, OUTPUT);
  digitalWrite(38, LOW);
  // pinMode(17, OUTPUT);
  // digitalWrite(17, LOW);
  // pinMode(18, OUTPUT);
  // digitalWrite(18, LOW);
  pinMode(42, OUTPUT);
  digitalWrite(42, LOW);

  // touch timing init
  Wire.begin(19, 20);
  Out.reset();
  Out.setMode(IO_OUTPUT);
  Out.setState(IO0, IO_LOW);
  Out.setState(IO1, IO_LOW);
  // delay(20);
  Out.setState(IO0, IO_HIGH);
  // delay(100);
  Out.setMode(IO1, IO_INPUT);


#endif

  //Display Prepare
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  // delay(200);

  lv_init();

  // delay(100);

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf1, disp_draw_buf2, screenWidth * screenHeight/10);
  /* Initialize the display */
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.full_refresh = 1;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
   indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  tft.fillScreen(TFT_BLACK);
  ui_init();
 
  ui_LabelTimer = lv_label_create(lv_scr_act()); // Создать метку на активном экране
  lv_label_set_text(ui_LabelTimer, "00:00");     // Установить начальный текст
  lv_obj_set_pos(ui_LabelTimer, 365, 325);        // Позиция (x, y)
  lv_obj_set_size(ui_LabelTimer, 200, 50);       // Размер (ширина, высота)
  lv_obj_set_style_text_font(ui_LabelTimer, &lv_font_montserrat_24, LV_PART_MAIN); // Стиль шрифта
}


void updateTimerLabel() {
    if (timerActive) {
        unsigned long remainingMs = timeoutMinutes * 60000UL - (millis() - timerStartTime);
        if (remainingMs > 0) {
            int minutes = remainingMs / 60000;
            int seconds = (remainingMs % 60000) / 1000;
            char timeStr[20];
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", minutes, seconds);
            lv_label_set_text(ui_LabelTimer, timeStr);
        } else {
            lv_label_set_text(ui_LabelTimer, "00:00");
        }
    } else {
        lv_label_set_text(ui_LabelTimer, "00:00");
    }
}


void loop() {
    lv_timer_handler();
    
    if (rab == 1) {
        char buf[32];
        lv_roller_get_selected_str(ui_Roller2, buf, sizeof(buf));
        outVal1 = atoi(buf);
        outVal = map(outVal1, 10, 100, 120, 255);
        light.setBrightness(outVal);
        rab = 0;
    }
    
    // Кнопка ручного режима
    if (baran == 1) {
        rab = 1;
        light.setBrightness(120);    // СЛАЙДЕР ЗЕЛЕНЫЙ              
        baran = 0;               
    }
    if (baranoff == 1) {
        light.setBrightness(0);  // СЛАЙДЕР КРАСНЫЙ  
        baranoff = 0;               
    }









    if (timerActive) {
        unsigned long elapsed = millis() - timerStartTime;
        unsigned long timeoutMs = timeoutMinutes * 60000UL;
        unsigned long t=0;
        if (powerton== 1){
          light.setBrightness(120);
          powerton=0;
        }
        if (powerton== 2){
          light.setBrightness(0);
          powerton=0;
        }
        if (elapsed >= timeoutMs) {
          
            light.setBrightness(0);
            timerActive = false;
        }
        	if (millis()-t > 1000){
            t=millis();
            updateTimerLabel();
          }
    }


    
    //  delay(5);
}
