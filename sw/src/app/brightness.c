#include "app/brightness.h"

#include "bsp/light_control.h"

static brightness_mode_t brightness_mode;

void BRIGHTNESS_Init(void) {
    brightness_mode = brightness_mode_emergency;
}

void BRIGHTNESS_Update10ms(void) {
    if (brightness_mode == brightness_mode_off) {
        // turn off
    }
    else if (brightness_mode == brightness_mode_standard) {
        // calculate brightness
    }
    else if (brightness_mode == brightness_mode_adaptive) {
        // calculate brightness
    }
    else if (brightness_mode == brightness_mode_emergency) {
        // Set emergency brightness
    }
    else if (brightness_mode == brightness_mode_safety) {
        // Set safety brightness
    }
    else {
        // ?
    }
}

void BRIGHTNESS_SetBlinking(bool blink) {

}

void BRIGHTNESS_Strobe(bool strobe) {

}