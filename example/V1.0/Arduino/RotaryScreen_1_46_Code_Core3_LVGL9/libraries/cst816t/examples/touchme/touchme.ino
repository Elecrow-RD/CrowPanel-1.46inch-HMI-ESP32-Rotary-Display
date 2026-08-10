/* demo for the CST816T capacitive touch ic */

#include <Wire.h>
#include "cst816t.h"

#define TP_SDA 6
#define TP_SCL 7
#define TP_RST 13
#define TP_IRQ 5

TwoWire Wire2(TP_SDA, TP_SCL);
cst816t touchpad(Wire2, TP_RST, TP_IRQ);

void setup() {
  // decode everything: single click, double click, long press, swipe up, swipe down, swipe left, swipe right
  touchpad.begin(mode_motion);
  Serial.println(touchpad.version());
}

void loop() {
  if (touchpad.available())
    Serial.println(touchpad.state());
}
