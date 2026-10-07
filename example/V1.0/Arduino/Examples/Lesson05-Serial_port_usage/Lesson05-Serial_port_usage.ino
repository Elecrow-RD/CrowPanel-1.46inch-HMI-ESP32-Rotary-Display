#include "Common_1_46.h"

/*-----------------------------------------------------------------
 * UART AT-command Wi-Fi example.
 * The display shows each connection stage while the primary serial
 * port receives the module responses for troubleshooting.
 *----------------------------------------------------------------*/

HardwareSerial wifi(1);
static const char SSID[] = "yanfa1";
static const char PASSWORD[] = "1223334444yanfa";
static lv_obj_t* st;
static String resp;

/**
 * @brief Replace the current status text on the LVGL screen.
 * @param s Null-terminated status message to display.
 * @return None.
 * @call Called before and between AT commands.
 */
static void status(const char* s) {
  lv_label_set_text(st, s);
  lv_timer_handler();
}

/**
 * @brief Send one AT command and collect its response until timeout.
 * @param c Command text without the line terminator.
 * @param ms Maximum wait time in milliseconds.
 * @return true when the response contains "OK".
 * @call Called by setup() for module detection and Wi-Fi setup.
 */
static bool at(const char* c, uint32_t ms) {
  while (wifi.available()) wifi.read();
  wifi.print(c);
  wifi.print("\r\n");

  uint32_t t = millis();
  resp = "";
  while (millis() - t < ms) {
    while (wifi.available()) resp += (char)wifi.read();
    lv_timer_handler();
    delay(5);
  }
  Serial.println(resp);
  return resp.indexOf("OK") >= 0;
}

/**
 * @brief Initialize the display and execute the Wi-Fi setup sequence.
 * @param None.
 * @return None.
 * @call Arduino calls setup() once after reset.
 */
void setup() {
  Serial.begin(115200);
  examplePower();

  auto* d = exampleDisplay();
  st = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font(st, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_center(st);
  wifi.begin(115200, SERIAL_8N1, 44, 43);

  status("UART1 TX43/RX44\nChecking module...");
  if (!at("AT", 1000)) {
    status("Module not found");
    return;
  }
  status("Configuring station mode...");
  at("AT+CWMODE=1", 1200);

  char join[160];
  snprintf(join, sizeof(join), "AT+CWJAP=\"%s\",\"%s\"", SSID, PASSWORD);
  status("Connecting to Wi-Fi...");
  if (!at(join, 15000)) {
    status("Connect failed");
    return;
  }
  status("Wi-Fi connected\nQuerying IP...");
  at("AT+CIFSR", 1500);
}

/**
 * @brief Keep LVGL responsive after the blocking setup sequence.
 * @param None.
 * @return None.
 * @call Arduino calls loop() repeatedly.
 */
void loop() {
  lv_timer_handler();
  delay(5);
}
