#include "Common_1_46.h"

/*-----------------------------------------------------------------
 * DHT20 temperature and humidity monitor.
 * A measurement is requested over the secondary I2C bus and the
 * sensor's packed 20-bit values are converted for display.
 *----------------------------------------------------------------*/

static lv_obj_t* out;

/**
 * @brief Trigger a DHT20 measurement and decode its result.
 * @param t Output temperature in degrees Celsius.
 * @param h Output relative humidity in percent.
 * @return true when a complete valid response is received.
 * @call Called periodically by loop().
 */
static bool readDHT(float& t, float& h) {
  uint8_t cmd[3] = {0xAC, 0x33, 0};
  Wire1.beginTransmission(0x38);
  Wire1.write(cmd, 3);
  if (Wire1.endTransmission() != 0) return false;
  delay(80);
  if (Wire1.requestFrom(0x38, 6) != 6) return false;

  uint8_t d[6];
  for (int i = 0; i < 6; i++) d[i] = Wire1.read();
  if (d[0] & 0x80) return false;

  uint32_t rh = (d[1] << 12) | (d[2] << 4) | (d[3] >> 4);
  uint32_t rt = ((d[3] & 15) << 16) | (d[4] << 8) | d[5];
  h = rh * 100.0 / 1048576.0;
  t = rt * 200.0 / 1048576.0 - 50;
  return true;
}

/**
 * @brief Initialize the display and the DHT20 I2C bus.
 * @param None.
 * @return None.
 * @call Arduino calls setup() once after reset.
 */
void setup() {
  Serial.begin(115200);
  examplePower();
  Wire1.begin(38, 39);

  auto* d = exampleDisplay();
  out = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(out, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_center(out);
  lv_label_set_text(out, "Reading DHT20...");
}

/**
 * @brief Read the sensor every two seconds and refresh the display/log.
 * @param None.
 * @return None.
 * @call Arduino calls loop() repeatedly.
 */
void loop() {
  static uint32_t last = 0;
  if (millis() - last > 2000) {
    last = millis();
    float t, h;
    char s[80];
    if (readDHT(t, h)) {
      snprintf(s, sizeof(s), "Temperature: %.1f C\nHumidity: %.1f %%", t, h);
    } else {
      strcpy(s, "DHT20 read error");
    }
    lv_label_set_text(out, s);
    Serial.println(s);
  }
  lv_timer_handler();
  delay(5);
}
