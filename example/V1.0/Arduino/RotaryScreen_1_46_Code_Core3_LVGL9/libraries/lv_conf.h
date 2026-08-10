/**
 * LVGL 9.1 configuration for the 360 x 360 ESP32-S3 rotary display.
 * Keep this file next to the installed "lvgl" library directory.
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Display output and SquareLine image assets are RGB565. */
#define LV_COLOR_DEPTH 16

/* No LVGL operating-system abstraction is required by this sketch. */
#define LV_USE_OS LV_OS_NONE

/* Widgets used by the SquareLine project. */
#define LV_USE_ARC 1
#define LV_USE_BUTTON 1
#define LV_USE_IMAGE 1
#define LV_USE_IMAGEBUTTON 1
#define LV_USE_LABEL 1

/* Fonts referenced by the generated screens. */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Layout and theme support used by SquareLine-generated code. */
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/* SquareLine UI compatibility: LVGL 9.1 still uses the LV_SCR_* enum names. */
#define LV_SCREEN_LOAD_ANIM_FADE_ON LV_SCR_LOAD_ANIM_FADE_ON

/* Keep unused examples and demos out of the firmware. */
#define LV_BUILD_EXAMPLES 0
#define LV_USE_DEMO_WIDGETS 0
#define LV_USE_DEMO_KEYPAD_AND_ENCODER 0
#define LV_USE_DEMO_BENCHMARK 0
#define LV_USE_DEMO_RENDER 0
#define LV_USE_DEMO_STRESS 0
#define LV_USE_DEMO_MUSIC 0

/* Useful low-cost checks for allocation and null pointer failures. */
#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

#endif /* LV_CONF_H */
