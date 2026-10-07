#include <USB.h>
#include <USBHIDMouse.h>
#include <cst816t.h>
#include "Common_1_46.h"

/*-----------------------------------------------------------------
 * Touch-to-USB-HID mouse example.
 * Touch coordinates are converted to relative mouse reports; the
 * left button remains pressed while contact is maintained.
 *----------------------------------------------------------------*/

USBHIDMouse mouse;
static cst816t touch(Wire, TOUCH_RST, TOUCH_IRQ);
static uint16_t lx, ly;
static bool tracking = false;
static uint32_t lastTouchMs = 0;
static uint32_t lastLogMs = 0;
static int pendingDx = 0;
static int pendingDy = 0;

/**
 * @brief Initialize touch, USB HID mouse, and serial diagnostics.
 * @param None.
 * @return None.
 * @call Arduino calls setup() once after reset.
 */
void setup() {
  Serial.begin(115200);
  examplePower();
  touch.begin();
  mouse.begin();
  USB.begin();
  delay(1000);
  Serial.println("USB mouse ready (select USB-OTG/TinyUSB)");
  Serial.println("Touch logging started");
}

/**
 * @brief Convert touch motion into HID reports and release the button.
 * @param None.
 * @return None.
 * @call Arduino calls loop() repeatedly while the board is running.
 */
void loop() {
  if (touch.available()) {
    lastTouchMs = millis();
    if (!tracking) {
      lx = touch.x;
      ly = touch.y;
      tracking = true;
      pendingDx = 0;
      pendingDy = 0;
      Serial.printf("Touch start: x=%u y=%u\n", touch.x, touch.y);
    }
    int dx = (int)touch.x - lx;
    int dy = (int)touch.y - ly;
    pendingDx += dx * 2;
    pendingDy += dy * 2;
    int mouseDx = constrain(pendingDx, -127, 127);
    int mouseDy = constrain(pendingDy, -127, 127);
    if (!mouse.isPressed(MOUSE_LEFT)) mouse.press(MOUSE_LEFT);
    if (mouseDx || mouseDy) {
      mouse.move(mouseDx, mouseDy, 0);
      pendingDx -= mouseDx;
      pendingDy -= mouseDy;
      if (millis() - lastLogMs >= 100) {
        Serial.printf("Touch: x=%u y=%u, raw=%d,%d, move=%d,%d\n",
                      touch.x, touch.y, dx, dy, mouseDx, mouseDy);
        lastLogMs = millis();
      }
    }
    lx = touch.x;
    ly = touch.y;
  } else if (tracking && millis() - lastTouchMs > 40) {
    if (mouse.isPressed(MOUSE_LEFT)) {
      mouse.release(MOUSE_LEFT);
      Serial.println("Touch release");
    }
    pendingDx = 0;
    pendingDy = 0;
    tracking = false;
  }
  delay(8);
}
