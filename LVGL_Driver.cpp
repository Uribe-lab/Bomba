/*****************************************************************************
  | File        :   LVGL_Driver.c
  
  | help        : 
    The provided LVGL library file must be installed first
******************************************************************************/
#include "LVGL_Driver.h"

#include <TAMC_GT911.h>

// Pines oficiales del táctil para la placa ES3C28P
#define TOUCH_SDA  16
#define TOUCH_SCL  15
#define TOUCH_INT  17
#define TOUCH_RST  18
#define TOUCH_I2C_ADDR  0x38  // Dirección de fábrica del FT6336G

// Dimensiones de la lectura táctil (deben coincidir con la pantalla)
#define TOUCH_WIDTH  240
#define TOUCH_HEIGHT 320

TAMC_GT911 ts = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ LVGL_BUF_LEN ];
static lv_color_t buf2[ LVGL_BUF_LEN ];
// static lv_color_t* buf1 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);
// static lv_color_t* buf2 = (lv_color_t*) heap_caps_malloc(LVGL_BUF_LEN, MALLOC_CAP_SPIRAM);
    


/* Serial debugging */
void Lvgl_print(const char * buf)
{
    // Serial.printf(buf);
    // Serial.flush();
}

/*  Display flushing 
    Displays LVGL content on the LCD
    This function implements associating LVGL data to the LCD screen
*/
void Lvgl_Display_LCD( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
  LCD_addWindow(area->x1, area->y1, area->x2, area->y2, ( uint16_t *)&color_p->full);
  lv_disp_flush_ready( disp_drv );
}


/*Read the touchpad*/
void Lvgl_Touchpad_Read( lv_indev_drv_t * indev_drv, lv_indev_data_t * data )
{
  // Pedir datos al chip táctil FT6336G
  Wire.beginTransmission(TOUCH_I2C_ADDR);
  Wire.write(0x02); // Registro que guarda si hay dedos tocando
  
  if (Wire.endTransmission() == 0) {
    Wire.requestFrom(TOUCH_I2C_ADDR, 5);
    if (Wire.available() >= 5) {
      uint8_t touch_count = Wire.read() & 0x0F;
      uint8_t x_msb = Wire.read();
      uint8_t x_lsb = Wire.read();
      uint8_t y_msb = Wire.read();
      uint8_t y_lsb = Wire.read();
      
      if (touch_count > 0 && touch_count <= 5) {
        // Unir bytes para formar las coordenadas de píxeles
        uint16_t x = ((x_msb & 0x0F) << 8) | x_lsb;
        uint16_t y = ((y_msb & 0x0F) << 8) | y_lsb;
        
        // Mapeo directo para el diseño vertical de tu placa
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR; // Informa que está Presionado
        return;
      }
    }
  }
  data->state = LV_INDEV_STATE_REL; // Informa que está Liberado
}







void example_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}


void Lvgl_Init(void)
{
  // Reset físico y seguro del sensor táctil real FT6336G
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(10);
  digitalWrite(TOUCH_RST, HIGH);
  delay(50);

  // Arrancar el bus I2C en los pines correctos: 16 (SDA) y 15 (SCL)
  Wire.begin(TOUCH_SDA, TOUCH_SCL);

  lv_init();
  lv_disp_draw_buf_init( &draw_buf, buf1, buf2, LVGL_BUF_LEN);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  disp_drv.hor_res = LVGL_WIDTH;
  disp_drv.ver_res = LVGL_HEIGHT;
  disp_drv.flush_cb = Lvgl_Display_LCD;
  disp_drv.full_refresh = 1;                    
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = Lvgl_Touchpad_Read;
  lv_indev_drv_register( &indev_drv );

  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &example_increase_lvgl_tick,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t lvgl_tick_timer = NULL;
  esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);
}






void Lvgl_Loop(void)
{
  lv_timer_handler(); /* let the GUI do its work */
}
