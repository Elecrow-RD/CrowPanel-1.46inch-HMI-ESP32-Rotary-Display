#include "Common_1_46.h"

/*-----------------------------------------------------------------
 * Touch-controlled LED example.
 * CST816T input is translated into an LVGL button event, and the
 * event changes both the physical LED and its on-screen label.
 *----------------------------------------------------------------*/

static constexpr int LED_GPIO = 4;
static cst816t touch(Wire, TOUCH_RST, TOUCH_IRQ);
static lv_obj_t* label;
static bool on = false;

/**
 * @brief Convert the current CST816T reading to LVGL pointer data.
 * @param d Destination structure filled with state and coordinates.
 * @return None.
 * @call LVGL invokes this callback while processing input devices.
 */
static void readTouch(lv_indev_t*, lv_indev_data_t* d) {
  if (touch.available()) {
    d->state = LV_INDEV_STATE_PRESSED;
    d->point.x = touch.x;
    d->point.y = touch.y;
  } else {
    d->state = LV_INDEV_STATE_RELEASED;
  }
}

/**
 * @brief Toggle the LED and update the button label.
 * @param None. The event object is not needed by this handler.
 * @return None.
 * @call LVGL invokes this callback after a button click.
 */
static void click(lv_event_t*) {
  on = !on;
  digitalWrite(LED_GPIO, on);
  lv_label_set_text(label, on ? "LED ON" : "LED OFF");
}

/**
 * @brief Initialize hardware, LVGL display, touch input, and button UI.
 * @param None.
 * @return None.
 * @call Arduino calls setup() once after reset.
 */
void setup() {
  Serial.begin(115200);
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LOW);
  examplePower();
  touch.begin();

  auto* d = exampleDisplay();
  auto* i = lv_indev_create();
  lv_indev_set_type(i, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(i, readTouch);
  lv_indev_set_display(i, d);

  auto* b = lv_button_create(lv_screen_active());
  lv_obj_set_size(b, 180, 90);
  lv_obj_center(b);
  lv_obj_add_event_cb(b, click, LV_EVENT_CLICKED, nullptr);
  label = lv_label_create(b);
  lv_label_set_text(label, "LED OFF");
  lv_obj_center(label);
}

/**
 * @brief Run LVGL timers and keep the interface responsive.
 * @param None.
 * @return None.
 * @call Arduino calls loop() repeatedly.
 */
void loop() {
  lv_timer_handler();
  delay(5);
}
