# ESP32 Core 3.3.8 / LVGL 9.1.0 build

Open `RotaryScreen_1_46_Code_Core3_LVGL9.ino` in Arduino IDE 2.3.6.

Required board settings:

- Board: ESP32S3 Dev Module
- CPU Frequency: 240 MHz (WiFi)
- Flash Mode: QIO 80 MHz
- Flash Size: 16 MB (128 Mb)
- Partition Scheme: Huge APP (3 MB No OTA / 1 MB SPIFFS)
- PSRAM: OPI PSRAM
- USB CDC On Boot: Disabled
- Upload Mode: UART0 / Hardware CDC
- Upload Speed: 921600

Required software:

- ESP32 Arduino Core 3.3.8
- LVGL 9.1.0
- LovyanGFX 1.2.7
- Adafruit NeoPixel 1.15.1
- cst816t 1.5.1

Install the official LVGL 9.1.0 library. Place `lv_conf.h` beside the `lvgl`
library folder in the Arduino sketchbook `libraries` directory. Install or copy
the other supplied libraries without editing their source files.

The LVGL 9.1 UI in `libraries/UI` is based on the preserved SquareLine Studio
project and retains the original startup-to-main-screen animation transition.
