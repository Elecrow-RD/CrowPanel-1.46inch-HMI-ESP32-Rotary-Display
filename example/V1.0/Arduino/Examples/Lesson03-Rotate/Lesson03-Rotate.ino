#include "Common_1_46.h"

/*-----------------------------------------------------------------
 * Rotary encoder brightness and LED control.
 * The two encoder channels are decoded as a quadrature sequence;
 * the push button independently toggles the indicator LED.
 *----------------------------------------------------------------*/

static constexpr int A = 45;
static constexpr int B = 42;
static constexpr int KEY = 41;
static constexpr int LED = 4;
static int pct = 50;
static bool led = false;
static uint8_t last = 0;
static int8_t q = 0;
static const int8_t tr[16] = {
  0, -1, 1, 0, 1, 0, 0, -1,
  -1, 0, 0, 1, 0, 1, -1, 0
};
static lv_obj_t* l1;
static lv_obj_t* l2;

/**
 * @brief Apply the enlarged teaching font to one LVGL label.
 * @param obj Label that receives the font style.
 * @return None.
 * @call Called during UI construction before the first refresh.
 */
static void applyLargeFont(lv_obj_t* obj) {
  lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN);
}

/**
 * @brief Refresh displayed state and apply the current backlight PWM.
 * @param None. Uses the global brightness and LED state.
 * @return None.
 * @call Called at startup and after encoder or button changes.
 */
static void labels() {
  char s[32];
  snprintf(s, sizeof(s), "Brightness: %d%%", pct);
  lv_label_set_text(l1, s);
  lv_label_set_text(l2, led ? "LED ON" : "LED OFF");
  ledcWrite(BACKLIGHT_GPIO, pct * 255 / 100);
}

/**
 * @brief Initialize the encoder, LED, display, and LVGL labels.
 * @param None.
 * @return None.
 * @call Arduino calls setup() once after reset.
 */
void setup() {
  Serial.begin(115200);
  examplePower();
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(KEY, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  last = (digitalRead(A) << 1) | digitalRead(B);

  auto* d = exampleDisplay();
  auto* t = lv_label_create(lv_screen_active());
  lv_label_set_text(t, "Encoder Control");
  applyLargeFont(t);
  lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 55);
  l1 = lv_label_create(lv_screen_active());
  applyLargeFont(l1);
  lv_obj_align(l1, LV_ALIGN_CENTER, 0, -10);
  l2 = lv_label_create(lv_screen_active());
  applyLargeFont(l2);
  lv_obj_align(l2, LV_ALIGN_CENTER, 0, 35);
  labels();
}

/**
 * @brief Decode encoder movement, handle the push button, and service LVGL.
 * @param None.
 * @return None.
 * @call Arduino calls loop() repeatedly while the board is running.
 */
void loop() {
  uint8_t now = (digitalRead(A) << 1) | digitalRead(B);
  q += tr[(last << 2) | now];
  last = now;

  if (q >= 4) {
    pct = constrain(pct + 5, 0, 100);
    q = 0;
    labels();
  }
  if (q <= -4) {
    pct = constrain(pct - 5, 0, 100);
    q = 0;
    labels();
  }

  static bool old = HIGH;
  bool k = digitalRead(KEY);
  if (old && !k) {
    led = !led;
    digitalWrite(LED, led);
    labels();
  }
  old = k;
  lv_timer_handler();
  delay(2);
}
