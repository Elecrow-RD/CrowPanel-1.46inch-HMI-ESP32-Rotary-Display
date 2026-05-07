#ifndef _ESP_DISPLAY_1_46_H
#define _ESP_DISPLAY_1_46_H

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
unsigned long debounceTime = 40;   
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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIGHT, &Wire, OLED_RESET);
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
      cfg.freq_write = 80000000;    
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
      cfg.pin_rst = 14;       
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
#endif