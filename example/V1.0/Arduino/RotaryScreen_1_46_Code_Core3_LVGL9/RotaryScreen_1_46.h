#ifndef _ROTARYSCREEN_1_46_H
#define _ROTARYSCREEN_1_46_H

/*---------------------------------------------------------------
 * Project dependencies
 * Provide the display, touch, UI, LED, and ESP32 platform APIs.
 *--------------------------------------------------------------*/
#include <LovyanGFX.h>
#include <Arduino.h>
#include <esp_heap_caps.h>
#include <cst816t.h>
#include <Wire.h>
#include <lvgl.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#include "ui.h"

/*---------------------------------------------------------------
 * Auxiliary I2C bus pins
 * Define the second I2C connection reserved by the board design.
 *--------------------------------------------------------------*/
// GPIO used for data on the auxiliary I2C bus.
#define I2C_SDA_PIN     38
// GPIO used for the clock on the auxiliary I2C bus.
#define I2C_SCL_PIN     39

/*---------------------------------------------------------------
 * Power indicator and addressable LED ring
 * Keep these values aligned with the board schematic.
 *--------------------------------------------------------------*/
// Active-low power indicator output.
#define POWER_LIGHT_PIN 40                                           
// Data output shared by the eight addressable LEDs.
#define LED_PIN         48
// Number of NeoPixel devices connected in the ring.
#define LED_NUM         8  

/*---------------------------------------------------------------
 * Rotary encoder input pins
 * Read the quadrature phases and the integrated push button.
 *--------------------------------------------------------------*/
// Encoder phase A input.
#define ENCODER_A_PIN   45
// Encoder phase B input.
#define ENCODER_B_PIN   42
// Active-low encoder push-button input.
#define SWITCH_PIN      41

/*---------------------------------------------------------------
 * LED definitions retained by the original project
 * These repeated definitions intentionally preserve source logic.
 *--------------------------------------------------------------*/
#define POWER_LIGHT_PIN   40                                           
#define LED_PIN           48
#define LED_NUM           8  

/*---------------------------------------------------------------
 * PWM-controlled outputs
 * Drive the LCD backlight and the board's bulb demonstration LED.
 *--------------------------------------------------------------*/
// LCD backlight PWM output.
#define SCREEN_BACKLIGHT_PIN 46

// Bulb demonstration PWM output.
#define BULB_LED_PIN 43

// Logical display dimensions used by both LovyanGFX and LVGL.
const uint32_t screenWidth  = 360;
const uint32_t screenHeight = 360;

// Shared PWM timing: 5 kHz with an 8-bit duty cycle from 0 to 255.
const int pwmFreq = 5000;
const int pwmResolution = 8;

/*---------------------------------------------------------------
 * Encoder and push-button state
 * Share short-lived input state between the interrupt and RTOS task.
 *--------------------------------------------------------------*/
// Encoded direction awaiting processing: 1, 0, or -1 when idle.
volatile int8_t position_tmp = -1;  
// Reserved phase-A samples retained for compatibility with the sketch.
volatile int8_t currentA = 0;
volatile int8_t lastA = 0;

// Time of the most recent accepted button press.
volatile unsigned long lastPressTime = 0;   
// Records whether the button is currently considered pressed.
volatile bool pressFlag = false;      
// Number of accepted presses inside the double-click window.
volatile int clickCount = 0;  

// Button debounce interval, adjusted after single and double clicks.
unsigned long debounceTime = 30;   
// Maximum interval used to classify two presses as a double click.
const unsigned long doubleClickTime = 300; 

// Encoder counters used for direction logging and sanity checking.
int last_counter = 0;
int counter = 0;
// Current and previous logic levels sampled from encoder phase A.
int currentStateCLK;
int lastStateCLK;

// Human-readable direction included in serial diagnostic output.
String currentDir = "";
// Suppresses the first transition in one encoder direction at startup.
bool one_test = false;

/*---------------------------------------------------------------
 * NeoPixel animation state
 * Maintain the LED ring object and progress through each test pattern.
 *--------------------------------------------------------------*/
Adafruit_NeoPixel led = Adafruit_NeoPixel(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);  
// Allows the LED task to run or request a clean stop.
bool isLed = true;
// Counts completed repetitions within the current animation pattern.
uint8_t ledCount = 0;
// Stores the current global brightness during the breathing pattern.
int8_t ledBrightness = 0;

// Reserved touch coordinates retained by the original project.
uint16_t touchX, touchY;

/*---------------------------------------------------------------
 * LovyanGFX device configuration
 * Bind the ESP32-S3 SPI bus to the 360 x 360 ST77961 panel.
 *--------------------------------------------------------------*/
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST77961 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
public:
  /**
   * @brief Configure the display bus and ST77961 panel.
   *
   * Assigns the fixed board pins, SPI transfer rates, DMA behavior,
   * panel geometry, and RGB ordering before the display is started.
   *
   * @note Parameters: None.
   * @note Return value: None; constructors do not return a value.
   * @note Called while the global gfx object is constructed, before setup().
   */
  LGFX(void) {
    {
      // The write clock favors fast DMA updates, while the lower read
      // clock is retained even though this panel is configured write-only.
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
      // The memory and visible panel dimensions match the physical
      // 360 x 360 display, so no coordinate offset is required.
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

// Global display device used by setup() and the LVGL flush callback.
LGFX gfx; 

#endif
