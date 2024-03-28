#if !defined(APP_BRIGHTNESS_H_)
#define APP_BRIGHTNESS_H_

#include <stdbool.h>

typedef enum {
    brightness_mode_off,
    brightness_mode_standard,
    brightness_mode_adaptive,
    brightness_mode_emergency,
    brightness_mode_safety
} brightness_mode_t;

void BRIGHTNESS_Init(void);

void BRIGHTNESS_SetMode(brightness_mode_t mode);

void BRIGHTNESS_SetBraking(bool brake);

void BRIGHTNESS_SetBlinking(bool blink);

void BRIGHTNESS_Strobe(bool strobe);

void BRIGHTNESS_Update10ms(void);

#endif // APP_BRIGHTNESS_H_
