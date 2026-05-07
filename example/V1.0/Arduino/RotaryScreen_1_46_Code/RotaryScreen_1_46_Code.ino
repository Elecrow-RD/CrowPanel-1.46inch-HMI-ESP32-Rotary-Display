// #define LGFX_AUTOBUFFERmy_disp_flush
#include "RotaryScreen_1_46.h"

TwoWire IIC = TwoWire(1); 
static TwoWire* wi = &Wire;

cst816t touch = cst816t(Wire, 13, 5);

/*RTOS Task*/
TaskHandle_t ledTestTaskHandle = NULL;
TaskHandle_t encTaskHandle = NULL;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf = NULL;
static lv_color_t *buf1 = NULL;

lv_obj_t *current_screen = NULL;

//#define DEBUG_PRINT
/* screen index flag*/
int screen1_index = 1;

void IRAM_ATTR buttonISR() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > debounceTime) {
    debounceTime = 40;
    if (digitalRead(SWITCH_PIN)) {
      pressFlag = false;
    } else {
      pressFlag = true;
      lastPressTime = interruptTime;
      clickCount++;
    }
  }
  lastInterruptTime = interruptTime;
} 

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  if (gfx.getStartCount() > 0) {
    gfx.endWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  
  //if ( gfx.getTouch( &touchX, &touchY ) ) {
  if(touch.available())
  {
    data->state = LV_INDEV_STATE_PR;
    if(touch.x == 0 && touch.y == 0)
      return ;
    data->point.x = touch.x;

    data->point.y = touch.y;
    
#ifdef DEBUG_PRINT
    Serial.print( "Data x " );
    Serial.println( data->point.x );
    Serial.print( "Data y " );
    Serial.println( data->point.y );
#endif
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void initBacklight() {
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(SCREEN_BACKLIGHT_PIN, pwmChannel);
  ledcWrite(pwmChannel, (50 * 255) / 100);
}

void performClickAction()
{
  current_screen = lv_scr_act();
  if(current_screen == ui_Screen1)
  {
    if(current_screen == ui_Screen1)
    {
      if(screen1_index == 0)
      {
        _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 200 ,0, &ui_Screen2_screen_init);
      }
      else if(screen1_index == 1)
      {
        _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 200 ,0, &ui_Screen3_screen_init);
      }
      else if(screen1_index == 2)
      {
        _ui_screen_change(&ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 200 ,0, &ui_Screen4_screen_init);
      }
    }
  }
}

void performDoubleClickAction()
{
  current_screen = lv_scr_act();
  if(current_screen == ui_Screen2 || current_screen == ui_Screen3 || current_screen == ui_Screen4)
  {

    _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 200 ,0, &ui_Screen1_screen_init);
      
  }
}

// Arc Event Callbacks for Slider Changes
static void volume_arc_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_target(e);
    int value = lv_arc_get_value(arc);
    
    // Update text label
    char volText[8];
    if (value == 100) {
        snprintf(volText, sizeof(volText), "%d%%", value);
    } else {
        snprintf(volText, sizeof(volText), " %d%%", value);
    }
    lv_label_set_text(ui_VolNum, volText);
}

static void bulb_arc_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_target(e);
    int value = lv_arc_get_value(arc);
    
    // Update text label
    char bulbText[8];
    if (value == 100) {
        snprintf(bulbText, sizeof(bulbText), "%d%%", value);
    } else {
        snprintf(bulbText, sizeof(bulbText), " %d%%", value);
    }
    lv_label_set_text(ui_BulbNum, bulbText);
    
    // Update LED brightness on pin 43
    int pwm_value = (value * 255) / 100;
    ledcWrite(breathPwmChannel, pwm_value);
}

static void light_arc_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_target(e);
    int value = lv_arc_get_value(arc);
    
    // Update text label
    char lightText[8];
    if (value == 100) {
        snprintf(lightText, sizeof(lightText), "%d%%", value);
    } else {
        snprintf(lightText, sizeof(lightText), " %d%%", value);
    }
    lv_label_set_text(ui_LightNum, lightText);
    
    // Update screen brightness PWM
    int pwm_value = (value * 255) / 100;
    ledcWrite(pwmChannel, pwm_value);
}

void encTask(void *pvParameters) {
  while (1) {
    // Read the current state of CLK
    currentStateCLK = digitalRead(ENCODER_A_PIN);

    // If last and current state of CLK are different, then pulse occurred
    // React to only 1 state change to avoid double count
    if (currentStateCLK != lastStateCLK) {// && currentStateCLK == 1) {

      current_screen = lv_scr_act();

      // If the DT state is different than the CLK state then
      // the encoder is rotating CCW so decrement
      if (digitalRead(ENCODER_B_PIN) != currentStateCLK) {
        if (abs(last_counter - counter) > 200) {
          continue;
        }
        position_tmp = 1;

        if (current_screen == ui_Screen2) {
          int currentVol = lv_arc_get_value(ui_VolumeArc);
          Serial.printf(" ++ currentVol = %d\n", currentVol);
          int newVol = (currentVol + 5) > 100 ? 100 : currentVol + 5;
          Serial.printf(" ++ END currentVol = %d\n", newVol);
          lv_arc_set_value(ui_VolumeArc, newVol);

          char volumeText[8];
          if (newVol == 100) {
            snprintf(volumeText, sizeof(volumeText), "%d%%", newVol);
            lv_label_set_text(ui_VolNum, volumeText);
          } else {
            snprintf(volumeText, sizeof(volumeText), " %d%%", newVol);
            lv_label_set_text(ui_VolNum, volumeText);
          }

        } else if (current_screen == ui_Screen3) {
          int currentBulb = lv_arc_get_value(ui_BulbArc);
          Serial.printf(" ++ currentBulb = %d\n", currentBulb);
          int newBulb = (currentBulb + 5) > 100 ? 100 : currentBulb + 5;
          Serial.printf(" ++ END currentBulb = %d\n", newBulb);
          lv_arc_set_value(ui_BulbArc, newBulb);

          char BulbText[8];
          if (newBulb == 100) {
            snprintf(BulbText, sizeof(BulbText), "%d%%", newBulb);
            lv_label_set_text(ui_BulbNum, BulbText);
          } else {
            snprintf(BulbText, sizeof(BulbText), " %d%%", newBulb);
            lv_label_set_text(ui_BulbNum, BulbText);
          }
          // Update LED brightness on pin 43
          int pwm_value = (newBulb * 255) / 100;
          ledcWrite(breathPwmChannel, pwm_value);

        } else if (current_screen == ui_Screen4) {
          int currentLight = lv_arc_get_value(ui_LightArc);
          Serial.printf(" ++ currentLight = %d\n", currentLight);
          int newLight = (currentLight + 5) > 100 ? 100 : currentLight + 5;
          Serial.printf(" ++ END currentLight = %d\n", newLight);
          lv_arc_set_value(ui_LightArc, newLight);
          
          char LightText[8];
          if (newLight == 100) {
            snprintf(LightText, sizeof(LightText), "%d%%", newLight);
            lv_label_set_text(ui_LightNum, LightText);
          } else {
            snprintf(LightText, sizeof(LightText), " %d%%", newLight);
            lv_label_set_text(ui_LightNum, LightText);
          }
          int pwm_value = (newLight * 255) / 100;
          ledcSetup(pwmChannel, pwmFreq, pwmResolution);
          ledcAttachPin(SCREEN_BACKLIGHT_PIN, pwmChannel);
          ledcWrite(pwmChannel, pwm_value);
        }

        counter++;
        currentDir = "CCW";
      } else {
        if (one_test == false) 
        {
          one_test = true;
          continue;
        }

        if (current_screen == ui_Screen2) {
          int currentVol = lv_arc_get_value(ui_VolumeArc);
          Serial.printf(" -- currentVol = %d\n", currentVol);
          int newVol = (currentVol - 5) < 0 ? 0 : currentVol - 5;
          Serial.printf(" -- END currentVol = %d\n", newVol);
          lv_arc_set_value(ui_VolumeArc, newVol);

          char volumeText[8];
          if (newVol == 100) {
            snprintf(volumeText, sizeof(volumeText), "%d%%", newVol);
            lv_label_set_text(ui_VolNum, volumeText);
          } else {
            snprintf(volumeText, sizeof(volumeText), " %d%%", newVol);
            lv_label_set_text(ui_VolNum, volumeText);
          }

        } else if (current_screen == ui_Screen3) {
          int currentBulb = lv_arc_get_value(ui_BulbArc);
          Serial.printf(" -- currentBulb = %d\n", currentBulb);
          int newBulb = (currentBulb - 5) < 0 ? 0 : currentBulb - 5;
          Serial.printf(" -- END currentBulb = %d\n", newBulb);
          lv_arc_set_value(ui_BulbArc, newBulb);

          char BulbText[8];
          if (newBulb == 100) {
            snprintf(BulbText, sizeof(BulbText), "%d%%", newBulb);
            lv_label_set_text(ui_BulbNum, BulbText);
          } else {
            snprintf(BulbText, sizeof(BulbText), " %d%%", newBulb);
            lv_label_set_text(ui_BulbNum, BulbText);
          }
          // Update LED brightness on pin 43
          int pwm_value = (newBulb * 255) / 100;
          ledcWrite(breathPwmChannel, pwm_value);

        } else if (current_screen == ui_Screen4) {
          int currentLight = lv_arc_get_value(ui_LightArc);
          Serial.printf(" -- currentLight = %d\n", currentLight);
          int newLight = (currentLight - 5) < 0 ? 0 : currentLight - 5;
          Serial.printf(" -- END currentLight = %d\n", newLight);
          lv_arc_set_value(ui_LightArc, newLight);

          char LightText[8];
          if (newLight == 100) {
            snprintf(LightText, sizeof(LightText), "%d%%", newLight);
            lv_label_set_text(ui_LightNum, LightText);
          } else {
            snprintf(LightText, sizeof(LightText), " %d%%", newLight);
            lv_label_set_text(ui_LightNum, LightText);
          }
          int pwm_value = (newLight * 255) / 100;
          ledcSetup(pwmChannel, pwmFreq, pwmResolution);
          ledcAttachPin(SCREEN_BACKLIGHT_PIN, pwmChannel);
          ledcWrite(pwmChannel, pwm_value);
        }

        position_tmp = 0;
        counter--;
        currentDir = "CW";
      }

      Serial.print("Direction: ");
      Serial.print(currentDir);
      Serial.print(" | Counter: ");
      Serial.println(counter);
      last_counter = counter;

      processEncoder();

    }
    
    // Remember last CLK state
    lastStateCLK = currentStateCLK;

    if (clickCount == 1 && millis() - lastPressTime > doubleClickTime) {
      Serial.println("click ");
      Serial.printf("clickCount Value:");
      Serial.println(clickCount);
      performClickAction();
      debounceTime = 40;
      clickCount = 0; 
    }
    else if (clickCount == 2) {
      Serial.println("double click");
      Serial.printf("clickCount Value:");
      Serial.println(clickCount);
      performDoubleClickAction();
      debounceTime = 160;
      clickCount = 0; 
    }

    if (clickCount > 2) {
      clickCount = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void processEncoder()
{
  current_screen = lv_scr_act();

  if(current_screen == ui_Screen1)
  {
    if(position_tmp == 1)
    {
      if(screen1_index < 2)
      {
        screen1_index++;
      }

      Serial.printf("cur_index : %d\n", screen1_index);
    }
    else if(position_tmp == 0)
    {
      if(screen1_index > 0)
      {
        screen1_index--;
      }

      Serial.printf("cur_index : %d\n", screen1_index);
    }
    updataScreen(screen1_index);
    position_tmp = -1;
  }
}

void updataScreen(int index)
{
  if(index < 0)
  {
    index = 0;
  }
  else if (index > 2)
  {
    index = 2;
  }

  Serial.printf("cur_index :%d\n", screen1_index);

  lv_obj_add_flag(ui_volumeBlue,  LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_BulbBlue,    LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_BulbWhite,   LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_LightBlue,   LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_LightWhite,  LV_OBJ_FLAG_HIDDEN);

  switch(index)
  {
    case 0:
      //volume
      lv_obj_clear_flag(ui_volumeBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_volumeTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_volumeBlue, 0);
      lv_obj_set_x(ui_volumeTextBlue, 0);
      lv_obj_set_x(ui_volumeWhite, 0);
      lv_obj_set_x(ui_volumeTextWhite, 0);

      //bulb
      lv_obj_clear_flag(ui_BulbWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_BulbTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_BulbBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_BulbTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_BulbWhite, 110);
      lv_obj_set_x(ui_BulbTextWhite, 110);
      lv_obj_set_x(ui_BulbBlue, 110);
      lv_obj_set_x(ui_BulbTextBlue, 110);

      //light
      lv_obj_add_flag(ui_LightWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_LightTextWhite, LV_OBJ_FLAG_HIDDEN);

      break;
    case 1:
      //volume
      lv_obj_clear_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_volumeTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_volumeBlue, -110);
      lv_obj_set_x(ui_volumeTextBlue, -110);
      lv_obj_set_x(ui_volumeWhite, -110);
      lv_obj_set_x(ui_volumeTextWhite, -110);
      //bulb
      lv_obj_clear_flag(ui_BulbBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_BulbTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_BulbWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_BulbTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_BulbWhite, 0);
      lv_obj_set_x(ui_BulbTextWhite, 0);
      lv_obj_set_x(ui_BulbBlue, 0);
      lv_obj_set_x(ui_BulbTextBlue, 0);

      //light
      lv_obj_clear_flag(ui_LightWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_LightTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_LightBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_LightTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_LightBlue, 110);
      lv_obj_set_x(ui_LightTextBlue, 110);
      lv_obj_set_x(ui_LightWhite, 110);
      lv_obj_set_x(ui_LightTextWhite, 110);
      break;
    case 2:
      //volume
      lv_obj_add_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextBlue, LV_OBJ_FLAG_HIDDEN);

      //bulb
      lv_obj_clear_flag(ui_BulbWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_BulbTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_BulbBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_BulbTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_BulbWhite, -110);
      lv_obj_set_x(ui_BulbTextWhite, -110);
      lv_obj_set_x(ui_BulbBlue, -110);
      lv_obj_set_x(ui_BulbTextBlue, -110);

      //light
      lv_obj_clear_flag(ui_LightBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_LightTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_LightWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_LightTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_LightBlue, 0);
      lv_obj_set_x(ui_LightTextBlue, 0);
      lv_obj_set_x(ui_LightWhite, 0);
      lv_obj_set_x(ui_LightTextWhite, 0); 

      break;
  }
}

void ledTestTask(void *pvParameters) {
  while (1) {
    led.clear();
    led.show();
    //Five circles of white flowing water lights
    while ((isLed) && (ledCount++ < 5)) {
      for (int i = 0; isLed && i < 8; i++) {
        led.setPixelColor(i, led.Color(255, 255, 255));
        led.show();
        vTaskDelay(pdMS_TO_TICKS(250));
        led.clear();
        led.show();
      }
    }
    ledCount = 0;

    //All the lights flash in rapid succession in various colors simultaneously.
    for (int i = 0; isLed && i < 5; i++) {
      led.setPixelColor(0, led.Color(255, 0, 0));
      led.setPixelColor(1, led.Color(0, 255, 0));
      led.setPixelColor(2, led.Color(0, 0, 255));
      led.setPixelColor(3, led.Color(255, 255, 0));
      led.setPixelColor(4, led.Color(130, 0, 255));
      led.setPixelColor(5, led.Color(0, 130, 255));
      led.setPixelColor(6, led.Color(255, 130, 0));
      led.setPixelColor(7, led.Color(255, 0, 130));
      
      led.show();
      vTaskDelay(pdMS_TO_TICKS(100));
      led.clear();
      led.show();
      vTaskDelay(pdMS_TO_TICKS(100));
    }

    //Colorful flowing lights in 5 circles
    while (isLed && ledCount < 5) {
      for (int i = 0; isLed && i < 8; i++) {
        led.clear();
        switch (i) {
          case 0: led.setPixelColor(i, led.Color(255, 0, 0));   break;
          case 1: led.setPixelColor(i, led.Color(0, 255, 0));   break;
          case 2: led.setPixelColor(i, led.Color(0, 0, 255));   break;
          case 3: led.setPixelColor(i, led.Color(255, 255, 0)); break;
          case 4: led.setPixelColor(i, led.Color(130, 0, 255)); break;
          case 5: led.setPixelColor(i, led.Color(0, 130, 255)); break;
          case 6: led.setPixelColor(i, led.Color(255, 130, 0)); break;
          case 7: led.setPixelColor(i, led.Color(255, 0, 130)); break;
        }
        led.show();
        vTaskDelay(pdMS_TO_TICKS(250));
      }
      ledCount++;
    }
    ledCount = 0;

    //All the lights flash in a slow, colored pattern simultaneously.
    for (int i = 0; isLed && i < 5; i++) {
      led.setPixelColor(0, led.Color(255, 0, 0));
      led.setPixelColor(1, led.Color(0, 255, 0));
      led.setPixelColor(2, led.Color(0, 0, 255));
      led.setPixelColor(3, led.Color(255, 255, 0));
      led.setPixelColor(4, led.Color(130, 0, 255));
      led.setPixelColor(5, led.Color(0, 130, 255));
      led.setPixelColor(6, led.Color(255, 130, 0));
      led.setPixelColor(7, led.Color(255, 0, 130));
      led.show();
      vTaskDelay(pdMS_TO_TICKS(250));
      led.clear();
      led.show();
      vTaskDelay(pdMS_TO_TICKS(250));
    }

    //Colorful breathing light, breathing 5 times
    led.setPixelColor(0, led.Color(255, 0, 0));
    led.setPixelColor(1, led.Color(0, 255, 0));
    led.setPixelColor(2, led.Color(0, 0, 255));
    led.setPixelColor(3, led.Color(255, 255, 0));
    led.setPixelColor(4, led.Color(130, 0, 255));
    led.setPixelColor(5, led.Color(0, 130, 255));
    led.setPixelColor(6, led.Color(255, 130, 0));
    led.setPixelColor(7, led.Color(255, 0, 130));
    while ((isLed) && (ledCount++ < 10)) {
      for (ledBrightness = 0; isLed && ledBrightness <= 25; ledBrightness++) {
        led.setBrightness(ledBrightness);
        led.setPixelColor(0, led.Color(255, 0, 0));
        led.setPixelColor(1, led.Color(0, 255, 0));
        led.setPixelColor(2, led.Color(0, 0, 255));
        led.setPixelColor(3, led.Color(255, 255, 0));
        led.setPixelColor(4, led.Color(130, 0, 255));
        led.setPixelColor(5, led.Color(0, 130, 255));
        led.setPixelColor(6, led.Color(255, 130, 0));
        led.setPixelColor(7, led.Color(255, 0, 130));
        led.show();
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      for (; isLed && ledBrightness >= 0; ledBrightness--) {
        led.setBrightness(ledBrightness);
        led.show();
        vTaskDelay(pdMS_TO_TICKS(50));
      }
      ledCount++;
    }

    ledCount = 0;
    ledBrightness = 0;
    led.setBrightness(25);
    led.clear();
    led.show();

    //****************led stop****************
    if (!isLed) {
      ledCount = 0;
      ledBrightness = 0;
      led.setBrightness(25);
      led.clear();
      led.show();
      vTaskSuspend(NULL);
    }

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);

  pinMode(14, OUTPUT);        // The following operation is to reset the screen and make it work
  digitalWrite(14, HIGH);
  delay(10);                  // VDD goes high at start, pause
  digitalWrite(14, LOW);      // Bring reset low
  delay(10);                  // Wait 10 ms
  digitalWrite(14, HIGH);     // Bring out of reset

  wi->setPins(6,7);           // Touchscreen I2C pins
  wi->begin();
  IIC.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  pinMode(POWER_LIGHT_PIN, OUTPUT);        
  digitalWrite(POWER_LIGHT_PIN, LOW);      // The power indicator light is on

  // pinMode(4 , OUTPUT);        
  // pinMode(12, OUTPUT);            

  pinMode(1, OUTPUT);   
  digitalWrite(1, HIGH);      // Pin 1 and pin 2 need to be turned on simultaneously in order to turn on the screen's power
  pinMode(2, OUTPUT);    
  digitalWrite(2, HIGH); 

  pinMode(17, OUTPUT);        // Power pins for RGB lights
  digitalWrite(17, HIGH);

  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), buttonISR, FALLING);

  //delay(100);
  gfx.init();
  gfx.initDMA();  
  gfx.startWrite();
  gfx.setColor(0, 0, 0);
  gfx.setTextSize(2);
  gfx.fillScreen(TFT_BLACK);
  
  touch.begin(mode_touch);

  lv_init();

  size_t buffer_size = sizeof(lv_color_t) * screenWidth * screenHeight;
  buf = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);  //MALLOC_CAP_DEFAULT,MALLOC_CAP_DMA,MALLOC_CAP_INTERNAL
  buf1 = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  if (!buf)
    Serial.println("Failed to allocate for LVGL buf!");
  if (!buf1)
    Serial.println("Failed to allocate for LVGL buf1!");
  lv_disp_draw_buf_init(&draw_buf, buf, buf1, screenWidth * screenHeight);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
  delay(100);

  ui_init();

  // Configure arc ranges (0-100 for all)
  lv_arc_set_range(ui_VolumeArc,  0, 100);
  lv_arc_set_range(ui_BulbArc  ,  0, 100);
  lv_arc_set_range(ui_LightArc ,  0, 100);

  // Set initial values
  lv_arc_set_value(ui_VolumeArc, 50);
  lv_arc_set_value(ui_BulbArc  , 50);
  lv_arc_set_value(ui_LightArc , 50);

  // Bind arc event callbacks for slider value changes
  lv_obj_add_event_cb(ui_VolumeArc, volume_arc_event_cb , LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(ui_BulbArc  , bulb_arc_event_cb   , LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(ui_LightArc , light_arc_event_cb  , LV_EVENT_VALUE_CHANGED, NULL);

  // Initialize Bulb LED PWM (Pin 43)
  ledcSetup(breathPwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(BULB_LED_PIN, breathPwmChannel);
  ledcWrite(breathPwmChannel, 50); // Initial brightness ~20%

  led.begin();
  led.setBrightness(25);  
  led.clear();
  led.show();

  delay(200);
  initBacklight();

  xTaskCreatePinnedToCore(ledTestTask, "LED Test", 2048, NULL, 1, &ledTestTaskHandle, 0);
  xTaskCreatePinnedToCore(encTask, "ENC", 2048, NULL, 1, &encTaskHandle, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_timer_handler(); /* let the GUI do its work */

  vTaskDelay(pdMS_TO_TICKS(5));
}
