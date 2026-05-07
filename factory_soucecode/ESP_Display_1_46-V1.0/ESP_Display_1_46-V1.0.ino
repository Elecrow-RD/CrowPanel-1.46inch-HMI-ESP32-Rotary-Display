#define LGFX_AUTOBUFFERmy_disp_flush
#include <LovyanGFX.h>
#include <Arduino.h>
#include <cst816t.h>
#include <Wire.h>
#include <lvgl.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>


#include <demos/lv_demos.h>

//#include "ESP_Display_1_46.h"
#include "ui.h"

/* I2C Define */
//I2C SDA Pin
#define I2C_SDA_PIN     38
//I2C SCL Pin
#define I2C_SCL_PIN     39


/* LED Light Define */
#define POWER_LIGHT_PIN 40                                           
#define LED_PIN         48
#define LED_NUM         8  

uint8_t count = 0;

TwoWire IIC = TwoWire(1); 
static TwoWire* wi=&Wire;

cst816t touch = cst816t(Wire,13,5);
bool isTestingTouch = false;

/* WiFi */
String wifiId = "yanfa1";            //WiFi SSID to coonnect to
String wifiPwd = "1223334444yanfa";  //PASSWORD of the WiFi SSID to coonnect to
/* Uart */
String str_uart; 
/*BLE Set */
#define BLE_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLEServer *pBLEserver = NULL;
BLEService *pBLEservice = NULL;
BLECharacteristic *pBLEcharacteristic = NULL;
BLEAdvertising *pBLEadvertising = NULL;
bool isBLEon = false;
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("BLE Client connected");
  };

  void onDisconnect(BLEServer *pServer) {
    Serial.println("BLE Client disconnected - restarting advertising");
    pServer->startAdvertising(); 
  }
};
MyServerCallbacks *pServerCallbacks = NULL;

/*Test Flag*/
bool isQT = false;
bool isFinal = false;

uint16_t touchx = 0;
uint16_t touchy = 0;
/*RTOS Task*/
TaskHandle_t lcdTestTaskHandle = NULL;
TaskHandle_t ledTestTaskHandle = NULL;
TaskHandle_t encTaskHandle = NULL;
TaskHandle_t ioForTestTaskHandle = NULL;

/* ENCODER set*/
#define ENCODER_A_PIN 45
#define ENCODER_B_PIN 42
#define SWITCH_PIN 41

volatile int8_t position_tmp = -1;  
volatile int8_t currentA = 0;
volatile int8_t lastA = 0;

volatile unsigned long lastPressTime = 0;   
volatile bool pressFlag = false;      
volatile int clickCount = 0;          
unsigned long debounceTime = 30;   
const unsigned long doubleClickTime = 300; 

int last_counter = 0;
int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
bool one_test = false;
/* LED Light define */
#define POWER_LIGHT_PIN 40                                           
#define LED_PIN 48
#define LED_NUM 8  
Adafruit_NeoPixel led = Adafruit_NeoPixel(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);  
bool isLed = true;
uint8_t ledCount = 0;
int8_t ledBrightness = 0;
/*BackLight PWM Set*/
#define SCREEN_BACKLIGHT_PIN 46
const int pwmFreq = 5000;
const int pwmChannel = 0;
const int pwmResolution = 8;
/*LVGL set*/
static const uint32_t screenWidth = 360;
static const uint32_t screenHeight = 360;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf = NULL;
static lv_color_t *buf1 = NULL;

lv_obj_t *current_screen = NULL;


lv_obj_t *scr = NULL;

/* IIC Test */
#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIGHT, &IIC, OLED_RESET);
/* Touch */
uint16_t touchX, touchY;

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST77961 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST; 
      cfg.spi_mode = 0;      
      cfg.freq_write = 60000000;    
      cfg.freq_read = 20000000;         
      cfg.spi_3wire = true;      
      cfg.use_lock = true;       
      cfg.dma_channel = SPI_DMA_CH_AUTO;  
      cfg.pin_sclk = 10; 
      cfg.pin_mosi = 11;  
      cfg.pin_miso = -1; 
      cfg.pin_dc = 3;  
      _bus_instance.config(cfg);          
      _panel_instance.setBus(&_bus_instance); 
    }
    {                            
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 9;                 
      cfg.pin_rst = -1;       
      cfg.pin_busy = -1;         
      cfg.memory_width = 360; 
      cfg.memory_height = 360;  
      cfg.panel_width = 360;
      cfg.panel_height = 360; 
      cfg.offset_x = 0;   
      cfg.offset_y = 0;   
      cfg.offset_rotation = 0;  
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;  
      cfg.readable = false;  
      cfg.invert = false;    
      cfg.rgb_order = true;   
      cfg.dlen_16bit = false;   
      cfg.bus_shared = false;   
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);  
  }
};
LGFX gfx; 

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

char current_setup = 0;
void testTouch() {

  // bool isTouched;
  // uint16_t touchX, touchY;
  // uint8_t gesture;

  // isTouched = touch.getTouch(&touchX, &touchY, &gesture);
  // if (isTouched) {
  //   while (isTouched) {
  //     delay(10);
  //     isTouched = touch.getTouch(&touchX, &touchY, &gesture);
  //   }
  //   Serial.printf("(X:%d, Y:%d),gesture:0x%02X\n", touchX, touchY, gesture);
  // }
    if(touch.available())
    {
      if(touch.x == 0 && touch.y == 0)
        return ;
      touchx = touch.x;

      touchy = touch.y;
      // gfx.drawPixel(touchx,touchy,TFT_RED);
      Serial.printf("(X:%d, Y:%d)", touchx, touchy);
      Serial.println("");
      if(touchx > 0 && touchx < 50 && touchy > 160 && touchy < 200)
      {
        current_setup = 1;
        Serial.println("测试点1：oK");
      }
      else if(touchx > 320 && touchx < 360 && touchy > 160 && touchy < 200)
      {
        current_setup = 2;
        Serial.println("测试点2：oK");
      }
      else if(touchx > 160 && touchx < 200 && touchy > 0 && touchy < 40)
      {
        current_setup = 3;
        Serial.println("测试点3：oK");
      }
      else if(touchx > 160 && touchx < 200 && touchy > 320 && touchy < 360)
      {
        current_setup = 4;
        Serial.println("测试点4：oK");
      }
    }
    switch(current_setup)
    {
      case 0:
        gfx.fillRect(0, 160, 40, 40, TFT_RED);
        break;
      case 1:
        gfx.fillRect(0, 160, 40, 40, TFT_BLACK);
        gfx.fillRect(320, 160, 40, 40, TFT_RED);
        delay(10);
        break;
      case 2:
        gfx.fillRect(320, 160, 40, 40, TFT_BLACK);
        gfx.fillRect(160, 20, 40, 40, TFT_RED);
        delay(10);
        break;
      case 3:
        gfx.fillRect(160, 20, 40, 40, TFT_BLACK);
        gfx.fillRect(160, 320, 40, 40, TFT_RED);
        delay(10);
        break;
      case 4:
        gfx.fillRect(160, 320, 40, 40, TFT_BLACK);
        delay(10);
        break;
    }
  
}

void initBacklight() {
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(SCREEN_BACKLIGHT_PIN, pwmChannel);
  ledcWrite(pwmChannel, (50 * 255) / 100);
}

void bleInit() {
  BLEDevice::init("ESP32S3_1.46_BLE_Server");
  //set BLE Power Max
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  pBLEserver = BLEDevice::createServer();
  if (!pServerCallbacks) {
    pServerCallbacks = new MyServerCallbacks();
  }
  pBLEserver->setCallbacks(pServerCallbacks);

  pBLEservice = pBLEserver->createService(BLE_SERVICE_UUID);
  pBLEcharacteristic = pBLEservice->createCharacteristic(
    BLE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pBLEcharacteristic->setValue("Hello from BLE");

  pBLEadvertising = BLEDevice::getAdvertising();
  pBLEadvertising->addServiceUUID(BLE_SERVICE_UUID);
}
void bleON() {
  if (!isBLEon) {
    if (!pBLEservice || !pBLEadvertising) { bleInit(); }

    pBLEservice->start();
    pBLEadvertising->start();
    isBLEon = true;
    Serial.println("BLE started");
  } else {
    Serial.println("BLE already started");
  }
}
void bleOFF() {
  if (isBLEon) {
    pBLEservice->stop();
    pBLEadvertising->stop();
    isBLEon = false;
    Serial.println("BLE stopped.");
  } else {
    Serial.println("BLE already stopped");
  }
}

void wifiConnect_sta() {
  if (WiFi.isConnected()) {
    Serial.println("WiFi already connected");
  } else {
    uint8_t i = 0;

    WiFi.begin(wifiId.c_str(), wifiPwd.c_str());

    while ((i < 20) && (!WiFi.isConnected())) {
      delay(1000);
      Serial.print(".");
      i++;
    }
    if (i >= 20) {
      Serial.println("failed to connect wifi within 20s");
    } else {
      WiFi.setAutoReconnect(true);
      Serial.println("wifi connected");
    }
  }
}
void wifiDisconnect_sta() {
  uint8_t i = 0;
  WiFi.disconnect(true);
  WiFi.setAutoReconnect(false);
  while ((i < 5) && (WiFi.isConnected())) {
    delay(1000);
    Serial.print(".");
    i++;
  }
  if (i >= 5) {
    Serial.println("failed to disconnect wifi");
  } else {
    WiFi.setAutoReconnect(false);
    Serial.println("wifi disconnected");
  }
}
void wifiInfo() {
  Serial.println("----------WiFi Info----------");
  Serial.printf("wifiId:%s\nwifiPwd:%s\n", wifiId.c_str(), wifiPwd.c_str());
  // if(WiFi.isConnected()){
  if (WiFi.isConnected()) {
    Serial.println("wifi status:connected");
    Serial.print("ssid:");
    Serial.println(WiFi.SSID());
    Serial.print("rssi:");
    Serial.println(WiFi.RSSI());
    Serial.print("localIP:");
    Serial.println(WiFi.localIP());
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println("wifi status:disconnected");
  }
  Serial.println("----------WiFi Info----------");
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

void ioForTestTask(void *pvParameters) {
  while (1) {
    digitalWrite(4, HIGH);
    digitalWrite(12, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    digitalWrite(4, LOW);
    digitalWrite(12, LOW);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void temp_arc_set_value_cb(void *val)
{
    lv_arc_set_value(ui_TempArc, (int)val);
}

static void volume_arc_set_value_cb(void *val)
{
    lv_arc_set_value(ui_VolumeArc, (int)val);
}

static void light_arc_set_value_cb(void *val)
{
    lv_arc_set_value(ui_lightArc, (int)val);
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
          //lv_arc_set_value(ui_VolumeArc, newVol);
          lv_async_call(volume_arc_set_value_cb, (void*)newVol);
          char volText[8];
          if (newVol == 100) {
            snprintf(volText, sizeof(volText), "%d%%", newVol);
            lv_label_set_text(ui_VolNum, volText);
          } else {
            snprintf(volText, sizeof(volText), " %d%%", newVol);
            lv_label_set_text(ui_VolNum, volText);
          }
        } else if (current_screen == ui_Screen3) {
          int currentTemp = lv_arc_get_value(ui_TempArc);
          Serial.printf(" ++ currentVol = %d\n", currentTemp);
          int newTemp = (currentTemp + 5) > 200 ? 200 : currentTemp + 5;
          Serial.printf(" ++ END currentVol = %d\n", newTemp);
          //lv_arc_set_value(ui_TempArc, newTemp);
          lv_async_call(temp_arc_set_value_cb, (void*)newTemp);
          char TempText[8];
          if (newTemp >= 100 && newTemp <= 200) {
            snprintf(TempText, sizeof(TempText), "%d%°C", newTemp);
            lv_label_set_text(ui_TempNum, TempText);
          } else {
            snprintf(TempText, sizeof(TempText), " %d%°C", newTemp);
            lv_label_set_text(ui_TempNum, TempText);
          }
        } else if (current_screen == ui_Screen4) {
          int currentLight = lv_arc_get_value(ui_lightArc);
          Serial.printf(" ++ currentLight = %d\n", currentLight);
          int newLight = (currentLight + 5) > 100 ? 100 : currentLight + 5;
          Serial.printf(" ++ END currentLight = %d\n", newLight);
          //lv_arc_set_value(ui_lightArc, newLight);
          lv_async_call(light_arc_set_value_cb, (void*)newLight);
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
          //lv_arc_set_value(ui_VolumeArc, newVol);
          lv_async_call(volume_arc_set_value_cb, (void*)newVol);
          char volText[8];
          if (newVol == 100) {
            snprintf(volText, sizeof(volText), "%d%%", newVol);
            lv_label_set_text(ui_VolNum, volText);
          } else {
            snprintf(volText, sizeof(volText), " %d%%", newVol);
            lv_label_set_text(ui_VolNum, volText);
          }
        } else if (current_screen == ui_Screen3) {
          int currentTemp = lv_arc_get_value(ui_TempArc);
          Serial.printf(" -- currentVol = %d\n", currentTemp);
          int newTemp = (currentTemp - 5) < 0 ? 0 : currentTemp - 5;
          Serial.printf(" -- END currentVol = %d\n", newTemp);
          //lv_arc_set_value(ui_TempArc, newTemp);
          lv_async_call(temp_arc_set_value_cb, (void*)newTemp);
          char TempText[8];
          if (newTemp >= 100 && newTemp <= 200) {
            snprintf(TempText, sizeof(TempText), "%d%°C", newTemp);
            lv_label_set_text(ui_TempNum, TempText);
          } else {
            snprintf(TempText, sizeof(TempText), " %d%°C", newTemp);
            lv_label_set_text(ui_TempNum, TempText);
          }
        } else if (current_screen == ui_Screen4) {
          int currentLight = lv_arc_get_value(ui_lightArc);
          Serial.printf(" -- currentLight = %d\n", currentLight);
          int newLight = (currentLight - 5) < 0 ? 0 : currentLight - 5;
          Serial.printf(" -- END currentLight = %d\n", newLight);
          //lv_arc_set_value(ui_lightArc, newLight);
          lv_async_call(light_arc_set_value_cb, (void*)newLight);
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

  lv_obj_add_flag(ui_volumeBlue, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_tempBlue, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_tempWhite, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_lightBlue, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui_lightWhite, LV_OBJ_FLAG_HIDDEN);

  switch(index)
  {
    case 0:
    //voluem
      lv_obj_clear_flag(ui_volumeBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_volumeTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_volumeBlue, 0);
      lv_obj_set_x(ui_volumeTextBlue, 0);
      lv_obj_set_x(ui_volumeWhite, 0);
      lv_obj_set_x(ui_volumeTextWhite, 0);

    //temp
      lv_obj_clear_flag(ui_tempWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_tempTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_tempBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_tempTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_tempWhite, 110);
      lv_obj_set_x(ui_tempTextWhite, 110);
      lv_obj_set_x(ui_tempBlue, 110);
      lv_obj_set_x(ui_tempTextBlue, 110);

    //light
      lv_obj_add_flag(ui_lightWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_lightTextWhite, LV_OBJ_FLAG_HIDDEN);

      break;
    case 1:
    //voluem
      lv_obj_clear_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_volumeTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_volumeBlue, -110);
      lv_obj_set_x(ui_volumeTextBlue, -110);
      lv_obj_set_x(ui_volumeWhite, -110);
      lv_obj_set_x(ui_volumeTextWhite, -110);
    //temp
      lv_obj_clear_flag(ui_tempBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_tempTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_tempWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_tempTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_tempWhite, 0);
      lv_obj_set_x(ui_tempTextWhite, 0);
      lv_obj_set_x(ui_tempBlue, 0);
      lv_obj_set_x(ui_tempTextBlue, 0);

    //light
      lv_obj_clear_flag(ui_lightWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_lightTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_lightBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_lightTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_lightBlue, 110);
      lv_obj_set_x(ui_lightTextBlue, 110);
      lv_obj_set_x(ui_lightWhite, 110);
      lv_obj_set_x(ui_lightTextWhite, 110);

      break;
    case 2:
    //voluem
      lv_obj_add_flag(ui_volumeWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_volumeTextBlue, LV_OBJ_FLAG_HIDDEN);
    //temp
      lv_obj_clear_flag(ui_tempWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_tempTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_tempBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_tempTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_tempWhite, -110);
      lv_obj_set_x(ui_tempTextWhite, -110);
      lv_obj_set_x(ui_tempBlue, -110);
      lv_obj_set_x(ui_tempTextBlue, -110);

    //light
      lv_obj_clear_flag(ui_lightBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(ui_lightTextBlue, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_lightWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_lightTextWhite, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_x(ui_lightBlue, 0);
      lv_obj_set_x(ui_lightTextBlue, 0);
      lv_obj_set_x(ui_lightWhite, 0);
      lv_obj_set_x(ui_lightTextWhite, 0); 

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
          case 0: led.setPixelColor(i, led.Color(255, 0, 0)); break;
          case 1: led.setPixelColor(i, led.Color(0, 255, 0)); break;
          case 2: led.setPixelColor(i, led.Color(0, 0, 255)); break;
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

void lcdTestTask(void *pvParameters) {
  while (1) {
    if (isFinal) {
      gfx.fillScreen(TFT_RED);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if (isFinal) {
      gfx.fillScreen(TFT_GREEN);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if (isFinal) {
      gfx.fillScreen(TFT_BLUE);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if (isFinal) {
      gfx.fillScreen(TFT_WHITE);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if (isFinal) {
      gfx.fillScreen(gfx.color565(128, 128, 128));
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if (!isFinal) {
      vTaskSuspend(NULL);
    }
  }
}
  

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial);

  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
  delay(10);                  // VDD goes high at start, pause
  digitalWrite(14, LOW);  // Bring reset low
  delay(10);                  // Wait 10 ms
  digitalWrite(14, HIGH); // Bring out of reset

  wi->setPins(6,7);
  wi->begin();
  IIC.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  pinMode(40, OUTPUT);   
  digitalWrite(40, LOW); 
  pinMode(4, OUTPUT);           
  pinMode(12, OUTPUT);            

  pinMode(1, OUTPUT);   
  digitalWrite(1, HIGH); 
  pinMode(2, OUTPUT);    
  digitalWrite(2, HIGH); 
  pinMode(17, OUTPUT);
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

  led.begin();
  led.setBrightness(25);  
  led.clear();
  led.show();

  delay(200);
  initBacklight();

  xTaskCreatePinnedToCore(lcdTestTask, "LCD Test", 2048, NULL, 1, &lcdTestTaskHandle, 0);
  if (lcdTestTaskHandle != NULL) vTaskSuspend(lcdTestTaskHandle);
  xTaskCreatePinnedToCore(ledTestTask, "LED Test", 2048, NULL, 1, &ledTestTaskHandle, 0);
  xTaskCreatePinnedToCore(encTask, "ENC", 2048, NULL, 1, &encTaskHandle, 0);
  xTaskCreatePinnedToCore(ioForTestTask, "IO For Test", 1024, NULL, 1, &ioForTestTaskHandle, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isQT == false)
  {
      lv_timer_handler(); /* let the GUI do its work */
  }
  processStrFromUart();
  if (isTestingTouch) {
    testTouch();
  }
  vTaskDelay(pdMS_TO_TICKS(5));
}

void processStrFromUart() {
  if (Serial.available()) {  //检查UART0是否有数据
    str_uart = Serial.readStringUntil('\n');
    str_uart.trim();
    //Serial.printf("Serial(uart0):%s\n", str_uart.c_str());
    if (str_uart == "Q") {
      if (isQT == false) {
        Serial.println("test on");
        gfx.fillScreen(TFT_BLACK);
        gfx.drawString("QT test", 160, 180);
        isQT = true;
      }
    } else if (str_uart == "dL") {
      gfx.drawString("LED", 160, 180);
    } else if (str_uart == "L") {
      if (isLed == false) {
        isLed = true;
        if (ledTestTaskHandle != NULL) {
          vTaskResume(ledTestTaskHandle);
        }
      }
    } else if (str_uart == "l") {
      if (isLed == true) {
        isLed = false;
      }
    }
    else if (str_uart == "r") {
      ledcWrite(pwmChannel, 0);
      gfx.fillScreen(TFT_RED);
      ledcWrite(pwmChannel, 255);
    } else if (str_uart == "5") {
      gfx.fillScreen(TFT_GREEN);
    } else if (str_uart == "6") {
      gfx.fillScreen(TFT_BLUE);
    } else if (str_uart == "7") {
      gfx.fillScreen(TFT_WHITE);
    } else if (str_uart == "8") {
      gfx.fillScreen(gfx.color565(128, 128, 128));
    } else if (str_uart == "9") {
      gfx.fillScreen(TFT_BLACK);
    } else if (str_uart == "dG") {
      gfx.setTextColor(TFT_BLACK);
      gfx.fillScreen(TFT_WHITE);
      gfx.drawString("Backlight", 160, 180);
    } else if (str_uart == "dg") {
      gfx.setTextColor(TFT_WHITE);
      gfx.fillScreen(TFT_BLACK);
    } else if (str_uart == "0") {
      ledcWrite(pwmChannel, 0);
    } else if (str_uart == "1") {
      ledcWrite(pwmChannel, 63);
    } else if (str_uart == "2") {
      ledcWrite(pwmChannel, 126);
    } else if (str_uart == "3") {
      ledcWrite(pwmChannel, 189);
    } else if (str_uart == "4") {
      ledcWrite(pwmChannel, 255);
    } else if (str_uart.startsWith("wifiId:")) {
      int colonIndex = str_uart.indexOf(':');
      wifiId = str_uart.substring(colonIndex + 1);
      // static const char* wifiId = str_uart.substring(colonIndex + 1);
      Serial.printf("confirm wifiId:%s\n", wifiId.c_str());
    } else if (str_uart.startsWith("wifiPwd:")) {
      int colonIndex = str_uart.indexOf(':');
      wifiPwd = str_uart.substring(colonIndex + 1);
      Serial.printf("confirm wifiPwd:%s\n", wifiPwd.c_str());
    } else if (str_uart == "dW") {
      gfx.drawString("WiFi", 160, 180);
    } else if (str_uart == "W") {
      wifiConnect_sta();
    } else if (str_uart == "w") {
      wifiDisconnect_sta();
    } else if (str_uart == "I") {
      wifiInfo();
    } else if (str_uart == "dB") {
      gfx.drawString("BLE", 160, 180);
      bleON();
    } else if (str_uart == "b") {
      bleOFF();
    } else if (str_uart == "dT") {
      gfx.drawString("Touch", 160, 180);
    } else if (str_uart == "T") {
      isTestingTouch = true;
      gfx.fillScreen(TFT_BLACK);
    } else if (str_uart == "t") {
      isTestingTouch = false;
      current_setup = 0;
      gfx.fillScreen(TFT_BLACK);
    } else if (str_uart == "dO") {
      gfx.fillScreen(TFT_BLACK);
      gfx.drawString("IIC", 160, 180);
    } else if (str_uart == "O") {
      oled_test();
    } else if (str_uart == "o") {
      oled_stop();
    } else if (str_uart == "F") {
      vTaskSuspend(encTaskHandle);

      isFinal = true;
      if (lcdTestTaskHandle != NULL) {
        vTaskResume(lcdTestTaskHandle);
      }
      oled_test();
      if (isLed == false) {
        isLed = true; 
        if (ledTestTaskHandle != NULL) {
          vTaskResume(ledTestTaskHandle);
        }
      }
      bleON();
      wifiConnect_sta();
    } else if (str_uart == "f") {
      vTaskResume(encTaskHandle);

      isFinal = false;
      // isDht11 = false;
      oled_stop();
      // isLed = false;
      bleOFF();
      wifiDisconnect_sta();
    } else if (str_uart == "R") {
      isQT = false;
      gfx.fillScreen(TFT_BLACK);
      delay(200);
      ledcWrite(pwmChannel, 0);
      ESP.restart();
    }
  }
}

void testdrawstyles(void)
{
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(25,25);
  display.println(F("ELECROW"));
  display.display();

}

void oled_test()
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }
  else
  {
    Serial.println("OLED Begins");
  }

  testdrawstyles();
}

void oled_stop()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  Serial.println("OLED stoped");
}