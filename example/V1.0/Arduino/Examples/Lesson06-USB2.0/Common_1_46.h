#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include <Wire.h>
#include <cst816t.h>

/*-----------------------------------------------------------------
 * Shared 1.46 display and power configuration.
 * This header keeps the board pin map and LVGL display bridge
 * identical in each independent lesson folder.
 *----------------------------------------------------------------*/

static constexpr int SCREEN_WIDTH = 360;
static constexpr int SCREEN_HEIGHT = 360;
static constexpr int POWER_RAIL_1 = 1;
static constexpr int POWER_RAIL_2 = 2;
static constexpr int POWER_LIGHT_GPIO = 40;
static constexpr int BACKLIGHT_GPIO = 46;

// CST816T reset GPIO13, interrupt GPIO5 (per 1.46 schematic TP_RST/TP_INT).
static constexpr int TOUCH_SDA = 6;
static constexpr int TOUCH_SCL = 7;
static constexpr int TOUCH_RST = 13;
static constexpr int TOUCH_IRQ = 5;
static constexpr int DRAW_BUFFER_LINES = 40;

class ExampleLGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST77961 panel;
  lgfx::Bus_SPI bus;

 public:
  ExampleLGFX() {
    auto b = bus.config();
    b.spi_host = SPI2_HOST;
    b.freq_write = 80000000;
    b.freq_read = 20000000;
    b.spi_3wire = true;
    b.pin_sclk = 10;
    b.pin_mosi = 11;
    b.pin_miso = -1;
    b.pin_dc = 3;
    bus.config(b);

    panel.setBus(&bus);
    auto p = panel.config();
    p.pin_cs = 9;
    p.pin_rst = 14;
    p.memory_width = 360;
    p.memory_height = 360;
    p.panel_width = 360;
    p.panel_height = 360;
    p.invert = false;
    p.rgb_order = true;
    p.readable = false;
    panel.config(p);
    setPanel(&panel);
  }
};

static ExampleLGFX gfx;
static uint8_t* exampleBuf1 = nullptr;
static uint8_t* exampleBuf2 = nullptr;

/**
 * @brief Copy an LVGL draw area to the physical LCD.
 * @param d LVGL display that requested the flush.
 * @param a Pixel area being updated.
 * @param p RGB565 pixel buffer.
 * @return None.
 * @call LVGL invokes this callback after rendering a display area.
 */
static void exampleFlush(lv_display_t* d, const lv_area_t* a, uint8_t* p) {
  gfx.pushImage(a->x1, a->y1, a->x2 - a->x1 + 1, a->y2 - a->y1 + 1,
                (lgfx::rgb565_t*)p);
  lv_display_flush_ready(d);
}

/**
 * @brief Initialize LVGL and allocate the partial-rendering buffers.
 * @param None.
 * @return The configured LVGL display object.
 * @call Called by each lesson after board power and LCD setup.
 */
static lv_display_t* exampleDisplay() {
  lv_init();
  lv_tick_set_cb(millis);

  size_t n = SCREEN_WIDTH * DRAW_BUFFER_LINES * 2;
  uint32_t c = psramFound() ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL;
  exampleBuf1 = (uint8_t*)heap_caps_malloc(n, c);
  exampleBuf2 = (uint8_t*)heap_caps_malloc(n, c);
  if (!exampleBuf1) exampleBuf1 = (uint8_t*)malloc(n);
  if (!exampleBuf2) exampleBuf2 = (uint8_t*)malloc(n);

  auto* d = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_color_format(d, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(d, exampleFlush);
  lv_display_set_buffers(d, exampleBuf1, exampleBuf2, n,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  return d;
}

/**
 * @brief Enable board rails, backlight, I2C, and the LCD controller.
 * @param None.
 * @return None.
 * @call Called once during each lesson's setup().
 */
static void examplePower() {
  pinMode(POWER_LIGHT_GPIO, OUTPUT);
  digitalWrite(POWER_LIGHT_GPIO, LOW);
  pinMode(POWER_RAIL_1, OUTPUT);
  pinMode(POWER_RAIL_2, OUTPUT);
  digitalWrite(POWER_RAIL_1, HIGH);
  digitalWrite(POWER_RAIL_2, HIGH);
  ledcAttach(BACKLIGHT_GPIO, 5000, 8);
  ledcWrite(BACKLIGHT_GPIO, 128);
  Wire.setPins(TOUCH_SDA, TOUCH_SCL);
  Wire.begin();
  gfx.init();
  gfx.fillScreen(TFT_BLACK);
}

